#include "as/common.hpp"
#include "as/gl/gl_tools.hpp"
#include "as/model/model_tools.hpp"
#include "as/trans/camera.hpp"

#include "postproc_shader.hpp"
#include "scene_shader.hpp"
#include "skybox_shader.hpp"

namespace fs = std::experimental::filesystem;

/*******************************************************************************
 * Constants
 ******************************************************************************/

/* Textures */
static const auto kNumMipmapLevel = 5;
/* User interfaces */
static const auto kInitWindowRelativeCenterPos = glm::vec2(0.5f, 0.5f);
static const auto kInitWindowSize = glm::ivec2(600, 600);
static const auto kMinWindowSize = glm::ivec2(300, 300);
static const auto kNumKeyboardKeys = 256;
static const auto kCameraMovingStep = 0.2f;
static const auto kCameraRotationSensitivity = 0.005f;
static const auto kCameraZoomingStep = 5.0f;
/* Timers */
static const auto kTimerInterval = 10;

/*******************************************************************************
 * Models
 ******************************************************************************/

// Scenes
as::Model scene_model;
// Skyboxes
as::Model skybox_model;

/*******************************************************************************
 * Model States
 ******************************************************************************/

// Modes
enum class Modes { comparison, navigation };

// Current mode
Modes cur_mode = Modes::comparison;

/*******************************************************************************
 * Camera States
 ******************************************************************************/

// Camera transformations
as::CameraTrans camera_trans(glm::vec3(-12.0f, 2.0f, 0.0f),
                             glm::vec3(-0.15f * glm::half_pi<float>(),
                                       glm::half_pi<float>(), 0.0f));

/*******************************************************************************
 * GL Managers
 ******************************************************************************/

as::BufferManager buffer_manager;
as::FramebufferManager framebuffer_manager;
as::ProgramManager program_manager;
as::ShaderManager shader_manager;
as::TextureManager texture_manager;
as::UniformManager uniform_manager;
as::VertexSpecManager vertex_spec_manager;

as::GLManagers gl_managers;

/*******************************************************************************
 * Shaders
 ******************************************************************************/

shader::PostprocShader postproc_shader;
shader::SceneShader scene_shader;
shader::SkyboxShader skybox_shader;

/*******************************************************************************
 * User Interface States
 ******************************************************************************/

/* Window states */
bool window_closed = false;
float window_aspect_ratio;

/* Keyboard states */
bool pressed_keys[kNumKeyboardKeys] = {false};

/* Mouse states */
bool mouse_left_down = false;
glm::vec2 mouse_left_down_init_pos;

/*******************************************************************************
 * Timers
 ******************************************************************************/

unsigned int timer_cnt = 0;
bool timer_enabled = true;

/*******************************************************************************
 * GL States (Feed to GL)
 ******************************************************************************/

// Post-processing inputs
shader::PostprocShader::PostprocInputs postproc_inputs;

/*******************************************************************************
 * Menus
 ******************************************************************************/

enum MainMenuItems {
  kMainMidLevelSep,
  kMainImgAbs,
  kMainLaplacian,
  kMainSharpness,
  kMainPixelation,
  kMainAdvancedSep,
  kMainBloomEffect,
  kMainMagnifier,
  kMainCool,
  kMainSpecial,
  kMainExit
};
enum ModeMenuItems { kModeComparison, kModeNavigation };
enum TimerMenuItems { kTimerStart, kTimerStop };

/*******************************************************************************
 * GL Initialization Methods
 ******************************************************************************/

void InitGLUT(int argc, char *argv[]) {
  glutInit(&argc, argv);
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
  glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  as::SetGLWindowInitRelativeCenterPos(kInitWindowRelativeCenterPos,
                                       kInitWindowSize);
  as::SetGLWindowInitSize(kInitWindowSize);
  glutCreateWindow("Assignment 3");
}

/*******************************************************************************
 * Model Handlers
 ******************************************************************************/

void LoadModels() {
  const unsigned int scene_flags =
      aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_Triangulate;
  // Scene
  scene_model.LoadFile("assets/models/crytek-sponza/sponza.obj", scene_flags);
  // Skybox
  skybox_model.LoadFile("assets/models/sea/skybox.obj", scene_flags);
}

/*******************************************************************************
 * GL Manager Name Handlers
 ******************************************************************************/

std::string GetSceneGroupName() { return "scene"; }

std::string GetSkyboxGroupName() { return "skybox"; }

std::string GetMeshVAName(const std::string &group_name,
                          const size_t mesh_idx) {
  return "va/" + group_name + "/" + std::to_string(mesh_idx);
}

std::string GetMeshVABufferName(const std::string &group_name,
                                const size_t mesh_idx) {
  return "va_buffer/" + group_name + "/" + std::to_string(mesh_idx);
}

std::string GetMeshVAIdxsBufferName(const std::string &group_name,
                                    const size_t mesh_idx) {
  return "va_idxs/" + group_name + "/" + std::to_string(mesh_idx);
}

/*******************************************************************************
 * GL Context Configuration / Global
 ******************************************************************************/

void ConfigGLSettings() {
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_DEPTH_TEST);
}

void InitGLManagers() {
  gl_managers.RegisterManagers(buffer_manager, framebuffer_manager,
                               program_manager, shader_manager, texture_manager,
                               uniform_manager, vertex_spec_manager);
  gl_managers.Init();
}

void InitShaders() {
  // Post-processing
  postproc_shader.RegisterGLManagers(gl_managers);
  postproc_shader.Init();
  // Scene
  scene_shader.RegisterGLManagers(gl_managers);
  scene_shader.Init();
  // Skybox
  skybox_shader.RegisterGLManagers(gl_managers);
  skybox_shader.Init();
}

/*******************************************************************************
 * GL Context Configuration / Screens
 ******************************************************************************/

void UpdateScreenTextures(const GLsizei width, const GLsizei height) {
  for (int i = 0; i < 3; i++) {
    // Decide the framebuffer name
    const std::string framebuffer_name = "screen[" + std::to_string(i) + "]";
    // Decide the texture name
    const std::string tex_name = "screen_tex[" + std::to_string(i) + "]";
    // Decide the unit name
    const std::string tex_unit_name = tex_name;
    // Check whether to delete old texture
    if (texture_manager.HasTexture(tex_name)) {
      texture_manager.DeleteTexture(tex_name);
    }
    // Generate texture
    texture_manager.GenTexture(tex_name);
    // Update texture
    texture_manager.BindTexture(tex_name, GL_TEXTURE_2D, tex_unit_name);
    texture_manager.InitTexture2D(tex_name, GL_TEXTURE_2D, 1, GL_RGB8, width,
                                  height);
    texture_manager.SetTextureParamInt(tex_name, GL_TEXTURE_2D,
                                       GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    texture_manager.SetTextureParamInt(tex_name, GL_TEXTURE_2D,
                                       GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    texture_manager.SetTextureParamInt(tex_name, GL_TEXTURE_2D,
                                       GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    texture_manager.SetTextureParamInt(tex_name, GL_TEXTURE_2D,
                                       GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Attach textures to framebuffers
    framebuffer_manager.AttachTexture2DToFramebuffer(
        framebuffer_name, tex_name, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, 0);
  }
}

void UpdateScreenRenderbuffers(const GLsizei width, const GLsizei height) {
  for (int i = 0; i < 3; i++) {
    // Decide the framebuffer name
    const std::string framebuffer_name = "screen[" + std::to_string(i) + "]";
    // Decide the renderbuffer name
    const std::string renderbuffer_name =
        "screen_depth_renderbuffer[" + std::to_string(i) + "]";
    // Check whether to delete old renderbuffer
    if (framebuffer_manager.HasRenderbuffer(renderbuffer_name)) {
      framebuffer_manager.DeleteRenderbuffer(renderbuffer_name);
    }
    // Create renderbuffers
    framebuffer_manager.GenRenderbuffer(renderbuffer_name);
    // Initialize renderbuffers
    framebuffer_manager.InitRenderbuffer(renderbuffer_name, GL_RENDERBUFFER,
                                         GL_DEPTH_COMPONENT, width, height);
    // Attach renderbuffers to framebuffers
    framebuffer_manager.AttachRenderbufferToFramebuffer(
        framebuffer_name, renderbuffer_name, GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER);
  }
}

void ConfigGLScreens() {
  UpdateScreenTextures(kInitWindowSize.x, kInitWindowSize.y);
  UpdateScreenRenderbuffers(kInitWindowSize.x, kInitWindowSize.y);
}

/*******************************************************************************
 * GL Context Configuration / Models
 ******************************************************************************/

void ConfigModelBuffers(const as::Model &model, const std::string &group_name) {
  const std::vector<as::Mesh> meshes = model.GetMeshes();
  for (size_t mesh_idx = 0; mesh_idx < meshes.size(); mesh_idx++) {
    const as::Mesh &mesh = meshes.at(mesh_idx);
    // Decide the vertex spec name
    const std::string scene_va_name = GetMeshVAName(group_name, mesh_idx);
    // Decide the buffer names
    const std::string scene_buffer_name =
        GetMeshVABufferName(group_name, mesh_idx);
    const std::string scene_idxs_buffer_name =
        GetMeshVAIdxsBufferName(group_name, mesh_idx);
    // Get mesh data
    const std::vector<as::Vertex> vertices = mesh.GetVertices();
    const std::vector<size_t> idxs = mesh.GetIdxs();
    // Get memory size of mesh data
    const size_t vertices_mem_sz = mesh.GetVerticesMemSize();
    const size_t idxs_mem_sz = mesh.GetIdxsMemSize();

    /* Generate buffers */
    // VA
    buffer_manager.GenBuffer(scene_buffer_name);
    // VA indexes
    buffer_manager.GenBuffer(scene_idxs_buffer_name);

    /* Initialize buffers */
    // VA
    buffer_manager.InitBuffer(scene_buffer_name, GL_ARRAY_BUFFER,
                              vertices_mem_sz, NULL, GL_STATIC_DRAW);
    // VA indexes
    buffer_manager.InitBuffer(scene_idxs_buffer_name, GL_ELEMENT_ARRAY_BUFFER,
                              idxs_mem_sz, NULL, GL_STATIC_DRAW);

    /* Update buffers */
    // VA
    buffer_manager.UpdateBuffer(scene_buffer_name, GL_ARRAY_BUFFER, 0,
                                vertices_mem_sz, vertices.data());
    // VA indexes
    buffer_manager.UpdateBuffer(scene_idxs_buffer_name, GL_ELEMENT_ARRAY_BUFFER,
                                0, idxs_mem_sz, idxs.data());

    /* Create vertex arrays */
    // VA
    vertex_spec_manager.GenVertexArray(scene_va_name);

    /* Bind vertex arrays to buffers */
    // VA
    vertex_spec_manager.SpecifyVertexArrayOrg(scene_va_name, 0, 3, GL_FLOAT,
                                              GL_FALSE, 0);
    vertex_spec_manager.SpecifyVertexArrayOrg(scene_va_name, 1, 3, GL_FLOAT,
                                              GL_FALSE, 0);
    vertex_spec_manager.SpecifyVertexArrayOrg(scene_va_name, 2, 2, GL_FLOAT,
                                              GL_FALSE, 0);
    vertex_spec_manager.AssocVertexAttribToBindingPoint(scene_va_name, 0, 0);
    vertex_spec_manager.AssocVertexAttribToBindingPoint(scene_va_name, 1, 1);
    vertex_spec_manager.AssocVertexAttribToBindingPoint(scene_va_name, 2, 2);
    vertex_spec_manager.BindBufferToBindingPoint(
        scene_va_name, scene_buffer_name, 0, offsetof(as::Vertex, pos),
        sizeof(as::Vertex));
    vertex_spec_manager.BindBufferToBindingPoint(
        scene_va_name, scene_buffer_name, 1, offsetof(as::Vertex, normal),
        sizeof(as::Vertex));
    vertex_spec_manager.BindBufferToBindingPoint(
        scene_va_name, scene_buffer_name, 2, offsetof(as::Vertex, tex_coords),
        sizeof(as::Vertex));
  }
}

void ConfigSceneBuffers() {
  const std::string group_name = GetSceneGroupName();
  ConfigModelBuffers(scene_model, group_name);
}

void ConfigSceneTextures() {
  const std::vector<as::Mesh> &meshes = scene_model.GetMeshes();
  for (const as::Mesh &mesh : meshes) {
    const std::set<as::Texture> &textures = mesh.GetTextures();
    for (const as::Texture &texture : textures) {
      const std::string &path = texture.GetPath();
      // Check if the texture has been loaded
      if (texture_manager.HasTexture(path)) {
        continue;
      }
      // Decide the unit name
      const std::string unit_name = path;
      // Load the texture
      GLsizei width, height;
      int comp;
      std::vector<GLubyte> texels;
      as::LoadTextureByStb(path, 0, width, height, comp, texels);
      // Convert the texels from 3 channels to 4 channels to avoid GL errors
      texels = as::ConvertDataChannels3To4(texels);
      // Generate the texture
      texture_manager.GenTexture(path);
      // Bind the texture
      texture_manager.BindTexture(path, GL_TEXTURE_2D, unit_name);
      // Initialize the texture
      texture_manager.InitTexture2D(path, GL_TEXTURE_2D, kNumMipmapLevel,
                                    GL_RGBA8, width, height);
      // Update the texture
      texture_manager.UpdateTexture2D(path, GL_TEXTURE_2D, 0, 0, 0, width,
                                      height, GL_RGBA, GL_UNSIGNED_BYTE,
                                      texels.data());
      texture_manager.GenMipmap(path, GL_TEXTURE_2D);
      texture_manager.SetTextureParamInt(
          path, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      texture_manager.SetTextureParamInt(path, GL_TEXTURE_2D,
                                         GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
  }
}

void ConfigSkyboxBuffers() {
  const std::string group_name = GetSkyboxGroupName();
  ConfigModelBuffers(skybox_model, group_name);
}

void ConfigSkyboxTextures() {
  static const std::map<std::string, size_t> path_to_target_idx = {
      {"right.jpg", 0},  {"left.jpg", 1},  {"top.jpg", 2},
      {"bottom.jpg", 3}, {"front.jpg", 4}, {"back.jpg", 5}};
  const std::vector<as::Mesh> &meshes = skybox_model.GetMeshes();
  for (const as::Mesh &mesh : meshes) {
    const std::set<as::Texture> &textures = mesh.GetTextures();
    for (const as::Texture &texture : textures) {
      const std::string &path = texture.GetPath();
      // Check if the texture has been loaded
      if (texture_manager.HasTexture(path)) {
        continue;
      }
      // Decide the unit name
      const std::string unit_name = path;
      // Get the file name
      const fs::path fs_path(path);
      const std::string file_name = fs_path.filename().string();
      // Calculate the target
      const size_t target_idx = path_to_target_idx.at(file_name);
      const GLenum target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + target_idx;
      // Load the texture
      GLsizei width, height;
      int comp;
      std::vector<GLubyte> texels;
      as::LoadTextureByStb(path, 0, width, height, comp, texels);
      // Convert the texels from 3 channels to 4 channels to avoid GL errors
      texels = as::ConvertDataChannels3To4(texels);
      // Generate the texture
      texture_manager.GenTexture(path);
      // Bind the texture
      texture_manager.BindTexture(path, GL_TEXTURE_CUBE_MAP, unit_name);
      // Initialize the texture
      texture_manager.InitTexture2D(path, GL_TEXTURE_CUBE_MAP, kNumMipmapLevel,
                                    GL_RGBA8, width, height);
      // Update the texture
      texture_manager.UpdateCubeMapTexture2D(path, target, 0, 0, 0, width,
                                             height, GL_RGBA, GL_UNSIGNED_BYTE,
                                             texels.data());
      texture_manager.GenMipmap(path, GL_TEXTURE_CUBE_MAP);
      texture_manager.SetTextureParamInt(path, GL_TEXTURE_CUBE_MAP,
                                         GL_TEXTURE_MIN_FILTER,
                                         GL_LINEAR_MIPMAP_LINEAR);
      texture_manager.SetTextureParamInt(path, GL_TEXTURE_CUBE_MAP,
                                         GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      texture_manager.SetTextureParamInt(path, GL_TEXTURE_CUBE_MAP,
                                         GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      texture_manager.SetTextureParamInt(path, GL_TEXTURE_CUBE_MAP,
                                         GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      texture_manager.SetTextureParamInt(path, GL_TEXTURE_CUBE_MAP,
                                         GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }
  }
}

void ConfigGLModels() {
  // Scenes
  ConfigSceneBuffers();
  ConfigSceneTextures();
  // Skyboxes
  ConfigSkyboxBuffers();
  ConfigSkyboxTextures();
}

void ConfigGL() {
  /* Configure global contexts */
  as::EnableCatchingGLError();
  ConfigGLSettings();
  InitGLManagers();
  InitShaders();
  /* Configure program-wise contexts */
  LoadModels();
  /* Configure screen-wise contexts */
  ConfigGLScreens();
  /* Configure model-wise contexts */
  ConfigGLModels();
}

/*******************************************************************************
 * GL States Handling Methods
 ******************************************************************************/

void UpdateGlobalMvp() {
  shader::SceneShader::GlobalMvp global_mvp;
  const glm::mat4 identity(1.0f);
  global_mvp.proj =
      glm::perspective(glm::radians(45.0f), window_aspect_ratio, 0.1f, 1000.0f);
  global_mvp.view = camera_trans.GetTrans();
  global_mvp.model = identity;

  scene_shader.UpdateGlobalMvp(global_mvp);
}

void UpdatePostprocInputs() {
  postproc_shader.UpdateEnabled(cur_mode == Modes::comparison &&
                                mouse_left_down);
}

/*******************************************************************************
 * Drawing Methods
 ******************************************************************************/

void UpdateModelTrans() {
  shader::SceneShader::ModelTrans model_trans;
  const float scale_factors = 0.01f;
  model_trans.trans = glm::scale(glm::mat4(1.0f), glm::vec3(scale_factors));

  scene_shader.UpdateModelTrans(model_trans);
}

void UpdateGLStateBuffers() {
  UpdateGlobalMvp();
  UpdateModelTrans();
  UpdatePostprocInputs();
}

void UseScreenFramebuffer(const int framebuffer_idx) {
  framebuffer_manager.BindFramebuffer("screen[" +
                                      std::to_string(framebuffer_idx) + "]");
}

void DrawMeshes(const std::string &program_name, const std::string &group_name,
                const std::vector<as::Mesh> &meshes) {
  for (size_t mesh_idx = 0; mesh_idx < meshes.size(); mesh_idx++) {
    const as::Mesh &mesh = meshes.at(mesh_idx);
    // Decide the vertex spec name
    const std::string scene_va_name = GetMeshVAName(group_name, mesh_idx);
    // Decide the buffer names
    const std::string scene_buffer_name =
        GetMeshVABufferName(group_name, mesh_idx);
    const std::string scene_idxs_buffer_name =
        GetMeshVAIdxsBufferName(group_name, mesh_idx);
    // Get the array indexes
    const std::vector<size_t> &idxs = mesh.GetIdxs();
    // Get the textures
    const std::set<as::Texture> &textures = mesh.GetTextures();

    /* Update textures */
    // TODO: There is only diffuse texture
    for (const as::Texture &texture : textures) {
      const std::string &path = texture.GetPath();
      // Bind the texture
      texture_manager.BindTexture(path);
      // Get the unit index
      const GLuint unit_idx = texture_manager.GetUnitIdx(path);
      // Set the texture handler to the unit index
      uniform_manager.SetUniform1Int(program_name, "tex_hdlr", unit_idx);
    }

    /* Draw vertex arrays */
    vertex_spec_manager.BindVertexArray(scene_va_name);
    buffer_manager.BindBuffer(scene_buffer_name);
    buffer_manager.BindBuffer(scene_idxs_buffer_name);
    glDrawElements(GL_TRIANGLES, idxs.size(), GL_UNSIGNED_INT, 0);
  }
}

void DrawScenes() {
  scene_shader.Use();
  const std::string scene_group_name = GetSceneGroupName();
  const std::vector<as::Mesh> scene_meshes = scene_model.GetMeshes();
  DrawMeshes("scene", scene_group_name, scene_meshes);
}

void DrawSkyboxes() {
  skybox_shader.Use();
  const std::string skybox_group_name = GetSkyboxGroupName();
  const std::vector<as::Mesh> skybox_meshes = skybox_model.GetMeshes();
  DrawMeshes("skybox", skybox_group_name, skybox_meshes);
}

/*******************************************************************************
 * GLUT Callbacks / Display
 ******************************************************************************/

void GLUTDisplayCallback() {
  // Update scene-wise contexts
  UpdateGLStateBuffers();
  // Draw the scenes
  UseScreenFramebuffer(0);
  as::ClearColorBuffer();
  as::ClearDepthBuffer();
  DrawScenes();
  DrawSkyboxes();
  // Draw the post-processing effects
  postproc_shader.Draw();
  // Swap double buffers
  glutSwapBuffers();
}

void GLUTReshapeCallback(const int width, const int height) {
  const glm::ivec2 window_size = glm::vec2(width, height);
  // Limit the window size
  if (as::LimitGLWindowSize(window_size, kMinWindowSize)) {
    // If the window size has been limited, ignore the new size
    return;
  }
  // Update window aspect ratio
  window_aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
  // Set the viewport
  glViewport(0, 0, width, height);
  // Update screen textures
  UpdateScreenTextures(width, height);
  // Update screen renderbuffers
  UpdateScreenRenderbuffers(width, height);
  // Update window size
  postproc_shader.UpdateWindowSize(window_size);
}

/*******************************************************************************
 * GLUT Callbacks / User Interface
 ******************************************************************************/

void GLUTKeyboardCallback(const unsigned char key, const int x, const int y) {
  pressed_keys[key] = true;
  switch (key) {
    case 'r': {
      // Reset camera transformation
      camera_trans.ResetTrans();
      UpdateGlobalMvp();
    } break;
    case 27: {  // Escape
      glutLeaveMainLoop();
    } break;
  }
}

void GLUTKeyboardUpCallback(const unsigned char key, const int x, const int y) {
  pressed_keys[key] = false;
}

void GLUTSpecialCallback(const int key, const int x, const int y) {}

void GLUTMouseCallback(const int button, const int state, const int x,
                       const int y) {
  const glm::vec2 mouse_pos = glm::vec2(x, y);
  // Check which mouse button is clicked
  switch (state) {
    case GLUT_DOWN: {
      mouse_left_down_init_pos = mouse_pos;
      mouse_left_down = true;
    } break;
    case GLUT_UP: {
      mouse_left_down = false;
    } break;
  }
  // Update mouse position
  postproc_shader.UpdateMousePos(mouse_pos);
}

void GLUTMouseWheelCallback(const int button, const int dir, const int x,
                            const int y) {
  if (dir > 0) {
    camera_trans.AddEye(kCameraZoomingStep * glm::vec3(0.0f, 0.0f, -1.0f));
  } else {
    camera_trans.AddEye(kCameraZoomingStep * glm::vec3(0.0f, 0.0f, 1.0f));
  }
}

void GLUTMotionCallback(const int x, const int y) {
  const glm::vec2 mouse_pos = glm::vec2(x, y);
  // Check whether the left mouse button is down
  if (mouse_left_down) {
    switch (cur_mode) {
      case Modes::comparison: {
      } break;
      case Modes::navigation: {
        const glm::vec2 diff = mouse_pos - mouse_left_down_init_pos;
        camera_trans.AddAngle(kCameraRotationSensitivity *
                              glm::vec3(diff.y, diff.x, 0.0f));
        mouse_left_down_init_pos = mouse_pos;
      } break;
      default: { throw new std::runtime_error("Unknown mode"); }
    }
  }
  // Update mouse position
  postproc_shader.UpdateMousePos(mouse_pos);
}

void GLUTCloseCallback() { window_closed = true; }

/*******************************************************************************
 * GLUT Callbacks / Timer
 ******************************************************************************/

void GLUTTimerCallback(const int val) {
  // Check whether the window has closed
  if (window_closed) {
    return;
  }

  // Increment the counter
  timer_cnt++;

  // Update camera transformation
  if (pressed_keys['w']) {
    camera_trans.AddEye(kCameraMovingStep * glm::vec3(0.0f, 0.0f, -1.0f));
    UpdateGlobalMvp();
  }
  if (pressed_keys['s']) {
    camera_trans.AddEye(kCameraMovingStep * glm::vec3(0.0f, 0.0f, 1.0f));
    UpdateGlobalMvp();
  }
  if (pressed_keys['a']) {
    camera_trans.AddEye(kCameraMovingStep * glm::vec3(-1.0f, 0.0f, 0.0f));
    UpdateGlobalMvp();
  }
  if (pressed_keys['d']) {
    camera_trans.AddEye(kCameraMovingStep * glm::vec3(1.0f, 0.0f, 0.0f));
    UpdateGlobalMvp();
  }
  if (pressed_keys['z']) {
    camera_trans.AddEyeWorldSpace(kCameraMovingStep *
                                  glm::vec3(0.0f, 1.0f, 0.0f));
    UpdateGlobalMvp();
  }
  if (pressed_keys['x']) {
    camera_trans.AddEyeWorldSpace(kCameraMovingStep *
                                  glm::vec3(0.0f, -1.0f, 0.0f));
    UpdateGlobalMvp();
  }

  // Mark the current window as needing to be redisplayed
  glutPostRedisplay();

  // Update timer count
  postproc_shader.UpdateTime(timer_cnt);

  // Register the timer callback again
  if (timer_enabled) {
    glutTimerFunc(kTimerInterval, GLUTTimerCallback, val);
  }
}

/*******************************************************************************
 * GLUT Callbacks / Menus
 ******************************************************************************/

void GLUTMainMenuCallback(const int id) {
  switch (id) {
    case MainMenuItems::kMainMidLevelSep: {
    } break;
    case MainMenuItems::kMainImgAbs: {
      postproc_shader.UpdateEffectIdx(shader::PostprocShader::kEffectImgAbs);
    } break;
    case MainMenuItems::kMainLaplacian: {
      postproc_shader.UpdateEffectIdx(shader::PostprocShader::kEffectLaplacian);
    } break;
    case MainMenuItems::kMainSharpness: {
      postproc_shader.UpdateEffectIdx(shader::PostprocShader::kEffectSharpness);
    } break;
    case MainMenuItems::kMainPixelation: {
      postproc_shader.UpdateEffectIdx(
          shader::PostprocShader::kEffectPixelation);
    } break;
    case MainMenuItems::kMainAdvancedSep: {
    } break;
    case MainMenuItems::kMainBloomEffect: {
      postproc_shader.UpdateEffectIdx(
          shader::PostprocShader::kEffectBloomEffect);
    } break;
    case MainMenuItems::kMainMagnifier: {
      postproc_shader.UpdateEffectIdx(shader::PostprocShader::kEffectMagnifier);
    } break;
    case MainMenuItems::kMainCool: {
    } break;
    case MainMenuItems::kMainSpecial: {
      postproc_shader.UpdateEffectIdx(shader::PostprocShader::kEffectSpecial);
    } break;
    case MainMenuItems::kMainExit: {
      glutLeaveMainLoop();
    } break;
    default: {
      throw std::runtime_error("Unrecognized menu ID '" + std::to_string(id) +
                               "'");
    }
  }
}

void GLUTModeMenuCallback(const int id) {
  switch (id) {
    case ModeMenuItems::kModeComparison: {
      cur_mode = Modes::comparison;
    } break;
    case ModeMenuItems::kModeNavigation: {
      cur_mode = Modes::navigation;
    } break;
    default: {
      throw std::runtime_error("Unrecognized menu ID '" + std::to_string(id) +
                               "'");
    }
  }
}

void GLUTTimerMenuCallback(const int id) {
  switch (id) {
    case TimerMenuItems::kTimerStart: {
      if (!timer_enabled) {
        timer_enabled = true;
        glutTimerFunc(kTimerInterval, GLUTTimerCallback, 0);
      }
    } break;
    case TimerMenuItems::kTimerStop: {
      timer_enabled = false;
    } break;
    default: {
      throw std::runtime_error("Unrecognized menu ID '" + std::to_string(id) +
                               "'");
    }
  }
}

/*******************************************************************************
 * GLUT Handlers
 ******************************************************************************/

void RegisterGLUTCallbacks() {
  /* Display */
  glutDisplayFunc(GLUTDisplayCallback);
  glutReshapeFunc(GLUTReshapeCallback);
  /* User interface */
  glutKeyboardFunc(GLUTKeyboardCallback);
  glutKeyboardUpFunc(GLUTKeyboardUpCallback);
  glutSpecialFunc(GLUTSpecialCallback);
  glutMouseFunc(GLUTMouseCallback);
  glutMouseWheelFunc(GLUTMouseWheelCallback);
  glutMotionFunc(GLUTMotionCallback);
  glutCloseFunc(GLUTCloseCallback);
  /* Timer */
  glutTimerFunc(kTimerInterval, GLUTTimerCallback, 0);
}

void CreateGLUTMenus() {
  const int main_menu_hdlr = glutCreateMenu(GLUTMainMenuCallback);
  const int mode_submenu_hdlr = glutCreateMenu(GLUTModeMenuCallback);
  const int timer_submenu_hdlr = glutCreateMenu(GLUTTimerMenuCallback);

  /* Main menu */
  glutSetMenu(main_menu_hdlr);
  glutAddSubMenu("Mode", mode_submenu_hdlr);
  glutAddSubMenu("Timer", timer_submenu_hdlr);
  glutAddMenuEntry("(Mid-level)", MainMenuItems::kMainMidLevelSep);
  glutAddMenuEntry("1. Image Abstraction", MainMenuItems::kMainImgAbs);
  glutAddMenuEntry("2. Laplacian Filter", MainMenuItems::kMainLaplacian);
  glutAddMenuEntry("3. Sharpness Filter", MainMenuItems::kMainSharpness);
  glutAddMenuEntry("4. Pixelation", MainMenuItems::kMainPixelation);
  glutAddMenuEntry("(Advanced)", MainMenuItems::kMainAdvancedSep);
  glutAddMenuEntry("1. Blooem Effect", MainMenuItems::kMainBloomEffect);
  glutAddMenuEntry("2. Magnifier", MainMenuItems::kMainMagnifier);
  glutAddMenuEntry("(Cool)", MainMenuItems::kMainCool);
  glutAddMenuEntry("1. Special Effect", MainMenuItems::kMainSpecial);
  glutAddMenuEntry("Exit", MainMenuItems::kMainExit);

  /* Mode submenu */
  glutSetMenu(mode_submenu_hdlr);
  glutAddMenuEntry("Comparison", ModeMenuItems::kModeComparison);
  glutAddMenuEntry("Navigation", ModeMenuItems::kModeNavigation);

  /* Timer submenu */
  glutSetMenu(timer_submenu_hdlr);
  glutAddMenuEntry("Start", TimerMenuItems::kTimerStart);
  glutAddMenuEntry("Stop", TimerMenuItems::kTimerStop);

  glutSetMenu(main_menu_hdlr);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void EnterGLUTLoop() { glutMainLoop(); }

/*******************************************************************************
 * Entry Point
 ******************************************************************************/

int main(int argc, char *argv[]) {
  try {
    InitGLUT(argc, argv);
    as::InitGLEW();
    ConfigGL();
    RegisterGLUTCallbacks();
    CreateGLUTMenus();
    EnterGLUTLoop();
  } catch (const std::exception &ex) {
    std::cerr << "Exception: " << ex.what() << std::endl;
    return 1;
  }
  return 0;
}
