#include "as/common.hpp"
#include "as/gl/gl_managers.hpp"
#include "as/model/model_tools.hpp"
#include "as/trans/camera.hpp"

namespace fs = std::experimental::filesystem;

/*******************************************************************************
 * Constants
 ******************************************************************************/

constexpr auto INIT_WINDOW_WIDTH = 600;
constexpr auto INIT_WINDOW_HEIGHT = 600;
constexpr auto TIMER_INTERVAL = 10;
constexpr auto KEYBOARD_KEY_SIZE = 256;
constexpr auto CAMERA_MOVING_STEP = 0.2f;
constexpr auto CAMERA_ROTATION_SENSITIVITY = 0.005f;
constexpr auto CAMERA_ZOOMING_STEP = 5.0f;
constexpr auto SCENE_SIZE = 1;
constexpr auto SKYBOX_SIZE = 1;
constexpr auto NUM_MIPMAP_LEVEL = 5;

/*******************************************************************************
 * Models
 ******************************************************************************/

// Screen quad
as::Model screen_quad_model;
// Scenes
as::Model scene_model[SCENE_SIZE];
// Skyboxes
as::Model skybox_model[SKYBOX_SIZE];

/*******************************************************************************
 * Model States
 ******************************************************************************/

// Effects
enum Effects {
  kEffectImgAbs,
  kEffectLaplacian,
  kEffectSharpness,
  kEffectPixelation,
  kEffectBloomEffect,
  kEffectMagnifier
};

// Modes
enum class Modes { comparison, navigation };

// Scene
size_t cur_scene_idx = 0;
size_t cur_skybox_idx = 0;

// Modes
Modes cur_mode = Modes::comparison;

// Effects
int cur_effect_idx = Effects::kEffectMagnifier;
int cur_pass_idx = 0;

/*******************************************************************************
 * Camera States
 ******************************************************************************/

// Camera transformations
as::CameraTrans camera_trans(glm::vec3(-12.0f, 2.0f, 0.0f),
                             glm::vec3(-0.15f * glm::half_pi<float>(),
                                       glm::half_pi<float>(), 0.0f));

/*******************************************************************************
 * Textures
 ******************************************************************************/

// Texture unit indexes
std::map<std::string, GLuint> texture_unit_idxs;

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

/*******************************************************************************
 * GL States (Feed to GL)
 ******************************************************************************/

// Global MVP declaration
struct GlobalMvp {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

// Model transformation declaration
struct ModelTrans {
  glm::mat4 trans;
};

// Comparison bar declaration
struct ComparisonBar {
  // Use int[2] instead of bool to avoid alignment mismatch problem (OpenGL
  // will pad the memory for alignment, but C++ sizeof() won't)
  int enabled[2];
  glm::vec2 mouse_pos;
};

// Post-processing inputs
struct PostprocInputs {
  int effect_idx[2];
  glm::vec2 window_size;
  int pass_idx[2];
};

// Global MVP
GlobalMvp global_mvp;

// Model transformations
ModelTrans model_trans;

// Comparison bar
ComparisonBar comparison_bar;

// Post-processing inputs
PostprocInputs postproc_inputs;

/*******************************************************************************
 * User Interface States
 ******************************************************************************/

// Window states
glm::vec2 window_size;
bool window_closed = false;
float window_aspect_ratio;

// Keyboard states
bool pressed_keys[KEYBOARD_KEY_SIZE] = {false};

// Mouse states
bool mouse_left_down = false;
glm::vec2 mouse_left_down_init_pos;
glm::vec2 mouse_pos;

/*******************************************************************************
 * Timers
 ******************************************************************************/

unsigned int timer_cnt = 0;
bool timer_enabled = true;

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
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT);
  glutCreateWindow("Assignment 3");
}

/*******************************************************************************
 * Model Handlers
 ******************************************************************************/

void LoadModels() {
  const unsigned int screen_quad_flags = 0;
  const unsigned int scene_flags =
      aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_Triangulate;
  // Screen quad
  screen_quad_model.LoadFile("assets/models/quad/quad.obj", screen_quad_flags);
  // First scene
  scene_model[0].LoadFile("assets/models/crytek-sponza/sponza.obj",
                          scene_flags);
  // First skybox
  skybox_model[0].LoadFile("assets/models/sea/skybox.obj", scene_flags);
}

/*******************************************************************************
 * GL Manager Name Handlers
 ******************************************************************************/

std::string GetSceneGroupName(const size_t scene_idx) {
  return "scene" + std::to_string(scene_idx);
}

std::string GetSkyboxGroupName(const size_t scene_idx) {
  return "skybox" + std::to_string(scene_idx);
}

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
  // Initialize managers
  texture_manager.Init();

  // Register managers
  framebuffer_manager.RegisterTextureManager(texture_manager);
  program_manager.RegisterShaderManager(shader_manager);
  uniform_manager.RegisterProgramManager(program_manager);
  uniform_manager.RegisterBufferManager(buffer_manager);
  vertex_spec_manager.RegisterBufferManager(buffer_manager);
}

void CreateGLShaders() {
  // Post-processing
  shader_manager.CreateShader("vertex/postproc", GL_VERTEX_SHADER,
                              "assets/shaders/postproc.vert");
  shader_manager.CreateShader("fragment/postproc", GL_FRAGMENT_SHADER,
                              "assets/shaders/postproc.frag");
  // Scenes
  shader_manager.CreateShader("vertex/scene", GL_VERTEX_SHADER,
                              "assets/shaders/scene.vert");
  shader_manager.CreateShader("fragment/scene", GL_FRAGMENT_SHADER,
                              "assets/shaders/scene.frag");
  // Skyboxes
  shader_manager.CreateShader("vertex/skybox", GL_VERTEX_SHADER,
                              "assets/shaders/skybox.vert");
  shader_manager.CreateShader("fragment/skybox", GL_FRAGMENT_SHADER,
                              "assets/shaders/skybox.frag");
}

void CreateGLPrograms() {
  // Post-processing
  program_manager.CreateProgram("postproc");
  program_manager.AttachShader("postproc", "vertex/postproc");
  program_manager.AttachShader("postproc", "fragment/postproc");
  program_manager.LinkProgram("postproc");
  program_manager.UseProgram("postproc");
  // Scenes
  program_manager.CreateProgram("scene");
  program_manager.AttachShader("scene", "vertex/scene");
  program_manager.AttachShader("scene", "fragment/scene");
  program_manager.LinkProgram("scene");
  program_manager.UseProgram("scene");
  // Skyboxes
  program_manager.CreateProgram("skybox");
  program_manager.AttachShader("skybox", "vertex/skybox");
  program_manager.AttachShader("skybox", "fragment/skybox");
  program_manager.LinkProgram("skybox");
  program_manager.UseProgram("skybox");
}

/*******************************************************************************
 * GL Context Configuration / Screens
 ******************************************************************************/

void ConfigScreenVertexArrays() {
  const std::vector<as::Mesh> meshes = screen_quad_model.GetMeshes();
  for (size_t mesh_idx = 0; mesh_idx < meshes.size(); mesh_idx++) {
    // There should be only one mesh
    assert(mesh_idx <= 0);

    const as::Mesh &mesh = meshes.at(mesh_idx);
    // Decide the names
    const std::string va_name = "screen_va";
    const std::string buffer_name = "screen_buffer";
    const std::string idxs_buffer_name = "screen_idxs_buffer";
    // Get mesh data
    const std::vector<as::Vertex> vertices = mesh.GetVertices();
    const std::vector<size_t> idxs = mesh.GetIdxs();
    // Get memory size of mesh data
    const size_t vertices_mem_sz = mesh.GetVerticesMemSize();
    const size_t idxs_mem_sz = mesh.GetIdxsMemSize();

    /* Generate buffers */
    // VA
    buffer_manager.GenBuffer(buffer_name);
    // VA indexes
    buffer_manager.GenBuffer(idxs_buffer_name);

    /* Initialize buffers */
    // VA
    buffer_manager.InitBuffer(buffer_name, GL_ARRAY_BUFFER, vertices_mem_sz,
                              NULL, GL_STATIC_DRAW);
    // VA indexes
    buffer_manager.InitBuffer(idxs_buffer_name, GL_ELEMENT_ARRAY_BUFFER,
                              idxs_mem_sz, NULL, GL_STATIC_DRAW);

    /* Update buffers */
    // VA
    buffer_manager.UpdateBuffer(buffer_name, GL_ARRAY_BUFFER, 0,
                                vertices_mem_sz, vertices.data());
    // VA indexes
    buffer_manager.UpdateBuffer(idxs_buffer_name, GL_ELEMENT_ARRAY_BUFFER, 0,
                                idxs_mem_sz, idxs.data());

    /* Create vertex arrays */
    // VA
    vertex_spec_manager.GenVertexArray(va_name);

    /* Bind vertex arrays to buffers */
    // VA
    vertex_spec_manager.SpecifyVertexArrayOrg(va_name, 0, 3, GL_FLOAT, GL_FALSE,
                                              0);
    vertex_spec_manager.SpecifyVertexArrayOrg(va_name, 1, 3, GL_FLOAT, GL_FALSE,
                                              0);
    vertex_spec_manager.AssocVertexAttribToBindingPoint(va_name, 0, 0);
    vertex_spec_manager.AssocVertexAttribToBindingPoint(va_name, 1, 1);
    vertex_spec_manager.BindBufferToBindingPoint(
        va_name, buffer_name, 0, offsetof(as::Vertex, pos), sizeof(as::Vertex));
    vertex_spec_manager.BindBufferToBindingPoint(
        va_name, buffer_name, 1, offsetof(as::Vertex, tex_coords),
        sizeof(as::Vertex));
  }
}

void ConfigScreenFramebuffers() {
  // Create framebuffers
  // 1st framebuffer is the original screen
  // 2nd and 3rd framebuffers are ping pong screen for multi-pass filtering
  for (int i = 0; i < 3; i++) {
    framebuffer_manager.GenFramebuffer("screen_framebuffer[" +
                                       std::to_string(i) + "]");
  }
}

void UpdateScreenTextures(const GLsizei width, const GLsizei height) {
  for (int i = 0; i < 3; i++) {
    // Decide the framebuffer name
    const std::string framebuffer_name =
        "screen_framebuffer[" + std::to_string(i) + "]";
    // Decide the texture name
    const std::string tex_name = "screen_tex[" + std::to_string(i) + "]";
    // Check whether to delete old texture
    GLuint unit_idx;
    if (texture_manager.HasTexture(tex_name)) {
      texture_manager.DeleteTexture(tex_name);
      unit_idx = texture_unit_idxs[tex_name];
    } else {
      // Calculate the new unit index
      unit_idx = texture_unit_idxs.size();
      // Save the unit index
      texture_unit_idxs[tex_name] = unit_idx;
    }
    // Generate texture
    texture_manager.GenTexture(tex_name);
    // Update texture
    texture_manager.BindTexture(tex_name, GL_TEXTURE_2D, unit_idx);
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
    const std::string framebuffer_name =
        "screen_framebuffer[" + std::to_string(i) + "]";
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
  ConfigScreenVertexArrays();
  ConfigScreenFramebuffers();
  UpdateScreenTextures(INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT);
  UpdateScreenRenderbuffers(INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT);
}

/*******************************************************************************
 * GL Context Configuration / Scenes
 ******************************************************************************/

void CreateGLBuffers() {
  // Global MVP
  buffer_manager.GenBuffer("global_mvp_buffer");
  // Model transformation
  buffer_manager.GenBuffer("model_trans_buffer");

  // Comparison bar
  buffer_manager.GenBuffer("comparison_bar_buffer");
  // Post-processing inputs
  buffer_manager.GenBuffer("postproc_inputs_buffer");
}

void InitGLBuffers() {
  // Global MVP
  buffer_manager.InitBuffer("global_mvp_buffer", GL_UNIFORM_BUFFER,
                            sizeof(GlobalMvp), NULL, GL_STATIC_DRAW);
  // Model transformation
  buffer_manager.InitBuffer("model_trans_buffer", GL_UNIFORM_BUFFER,
                            sizeof(ModelTrans), NULL, GL_STATIC_DRAW);

  // Comparison bar
  buffer_manager.InitBuffer("comparison_bar_buffer", GL_UNIFORM_BUFFER,
                            sizeof(ComparisonBar), NULL, GL_STATIC_DRAW);
  // Post-processing inputs
  buffer_manager.InitBuffer("postproc_inputs_buffer", GL_UNIFORM_BUFFER,
                            sizeof(PostprocInputs), NULL, GL_STATIC_DRAW);
}

void UpdateGLBuffers() {
  // Global MVP
  buffer_manager.UpdateBuffer("global_mvp_buffer", GL_UNIFORM_BUFFER, 0,
                              sizeof(GlobalMvp), &global_mvp);
  // Model transformation
  buffer_manager.UpdateBuffer("model_trans_buffer", GL_UNIFORM_BUFFER, 0,
                              sizeof(ModelTrans), &model_trans);

  // Comparison bar
  buffer_manager.UpdateBuffer("comparison_bar_buffer", GL_UNIFORM_BUFFER, 0,
                              sizeof(ComparisonBar), &comparison_bar);
  // Post-processing inputs
  buffer_manager.UpdateBuffer("postproc_inputs_buffer", GL_UNIFORM_BUFFER, 0,
                              sizeof(PostprocInputs), &postproc_inputs);
}

void BindGLUniformBlocksToBuffers() {
  // Global MVP
  uniform_manager.AssignUniformBlockToBindingPoint("scene", "GlobalMvp", 0);
  uniform_manager.BindBufferBaseToBindingPoint("global_mvp_buffer", 0);
  // Model transformation
  uniform_manager.AssignUniformBlockToBindingPoint("scene", "ModelTrans", 1);
  uniform_manager.BindBufferBaseToBindingPoint("model_trans_buffer", 1);

  // Comparison bar
  uniform_manager.AssignUniformBlockToBindingPoint("postproc", "ComparisonBar",
                                                   2);
  uniform_manager.BindBufferBaseToBindingPoint("comparison_bar_buffer", 2);
  // Post-processing inputs
  uniform_manager.AssignUniformBlockToBindingPoint("postproc", "PostprocInputs",
                                                   3);
  uniform_manager.BindBufferBaseToBindingPoint("postproc_inputs_buffer", 3);
}

void ConfigGLScenes() {
  CreateGLBuffers();
  InitGLBuffers();
  UpdateGLBuffers();
  BindGLUniformBlocksToBuffers();
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
    vertex_spec_manager.AssocVertexAttribToBindingPoint(scene_va_name, 0, 0);
    vertex_spec_manager.AssocVertexAttribToBindingPoint(scene_va_name, 1, 1);
    vertex_spec_manager.BindBufferToBindingPoint(
        scene_va_name, scene_buffer_name, 0, offsetof(as::Vertex, pos),
        sizeof(as::Vertex));
    vertex_spec_manager.BindBufferToBindingPoint(
        scene_va_name, scene_buffer_name, 1, offsetof(as::Vertex, tex_coords),
        sizeof(as::Vertex));
  }
}

void ConfigSceneBuffers() {
  for (size_t scene_idx = 0; scene_idx < SCENE_SIZE; scene_idx++) {
    const std::string group_name = GetSceneGroupName(scene_idx);
    ConfigModelBuffers(scene_model[scene_idx], group_name);
  }
}

void ConfigSceneTextures() {
  for (size_t scene_idx = 0; scene_idx < SCENE_SIZE; scene_idx++) {
    const std::vector<as::Mesh> &meshes = scene_model[scene_idx].GetMeshes();
    for (const as::Mesh &mesh : meshes) {
      const std::set<as::Texture> &textures = mesh.GetTextures();
      for (const as::Texture &texture : textures) {
        const std::string &path = texture.GetPath();
        // Check if the texture has been loaded
        if (texture_unit_idxs.count(path) > 0) {
          continue;
        }
        // Calculate the new unit index
        const GLuint unit_idx = texture_unit_idxs.size();
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
        texture_manager.BindTexture(path, GL_TEXTURE_2D, unit_idx);
        // Initialize the texture
        texture_manager.InitTexture2D(path, GL_TEXTURE_2D, NUM_MIPMAP_LEVEL,
                                      GL_RGBA8, width, height);
        // Update the texture
        texture_manager.UpdateTexture2D(path, GL_TEXTURE_2D, 0, 0, 0, width,
                                        height, GL_RGBA, GL_UNSIGNED_BYTE,
                                        texels.data());
        texture_manager.GenMipmap(path, GL_TEXTURE_2D);
        texture_manager.SetTextureParamInt(path, GL_TEXTURE_2D,
                                           GL_TEXTURE_MIN_FILTER,
                                           GL_LINEAR_MIPMAP_LINEAR);
        texture_manager.SetTextureParamInt(path, GL_TEXTURE_2D,
                                           GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Save the unit index
        texture_unit_idxs[path] = unit_idx;
      }
    }
  }
}

void ConfigSkyboxBuffers() {
  for (size_t skybox_idx = 0; skybox_idx < SKYBOX_SIZE; skybox_idx++) {
    const std::string group_name = GetSkyboxGroupName(skybox_idx);
    ConfigModelBuffers(skybox_model[skybox_idx], group_name);
  }
}

void ConfigSkyboxTextures() {
  static const std::map<std::string, size_t> path_to_target_idx = {
      {"right.jpg", 0},  {"left.jpg", 1},  {"top.jpg", 2},
      {"bottom.jpg", 3}, {"front.jpg", 4}, {"back.jpg", 5}};
  for (size_t scene_idx = 0; scene_idx < SKYBOX_SIZE; scene_idx++) {
    const std::vector<as::Mesh> &meshes = skybox_model[scene_idx].GetMeshes();
    for (const as::Mesh &mesh : meshes) {
      const std::set<as::Texture> &textures = mesh.GetTextures();
      for (const as::Texture &texture : textures) {
        const std::string &path = texture.GetPath();
        // Check if the texture has been loaded
        if (texture_unit_idxs.count(path) > 0) {
          continue;
        }
        // Get the file name
        const fs::path fs_path(path);
        const std::string file_name = fs_path.filename().string();
        // Calculate the target
        const size_t target_idx = path_to_target_idx.at(file_name);
        const GLenum target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + target_idx;
        // Calculate the new unit index
        const GLuint unit_idx = texture_unit_idxs.size();
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
        texture_manager.BindTexture(path, GL_TEXTURE_CUBE_MAP, unit_idx);
        // Initialize the texture
        texture_manager.InitTexture2D(path, GL_TEXTURE_CUBE_MAP,
                                      NUM_MIPMAP_LEVEL, GL_RGBA8, width,
                                      height);
        // Update the texture
        texture_manager.UpdateCubeMapTexture2D(path, target, 0, 0, 0, width,
                                               height, GL_RGBA,
                                               GL_UNSIGNED_BYTE, texels.data());
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
        // Save the unit index
        texture_unit_idxs[path] = unit_idx;
      }
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
  /* Configure program-wise contexts */
  LoadModels();
  CreateGLShaders();
  CreateGLPrograms();
  /* Configure screen-wise contexts */
  ConfigGLScreens();
  /* Configure scene-wise contexts */
  ConfigGLScenes();
  /* Configure model-wise contexts */
  ConfigGLModels();
}

/*******************************************************************************
 * GL States Handling Methods
 ******************************************************************************/

void UpdateGlobalMvp() {
  const glm::mat4 identity(1.0f);
  global_mvp.proj =
      glm::perspective(glm::radians(45.0f), window_aspect_ratio, 0.1f, 1000.0f);
  global_mvp.view = camera_trans.GetTrans();
  global_mvp.model = identity;
}

void UpdateComparisonBar() {
  comparison_bar.enabled[0] =
      (cur_mode == Modes::comparison && mouse_left_down);
  comparison_bar.mouse_pos = mouse_pos;
}

void UpdatePostprocInputs() {
  postproc_inputs.effect_idx[0] = cur_effect_idx;
  postproc_inputs.window_size = window_size;
  postproc_inputs.pass_idx[0] = cur_pass_idx;
}

/*******************************************************************************
 * Drawing Methods
 ******************************************************************************/

void ClearColorBuffer() { glClear(GL_COLOR_BUFFER_BIT); }

void ClearDepthBuffer() { glClear(GL_DEPTH_BUFFER_BIT); }

void UpdateGlobalMvpBuffer() {
  UpdateGlobalMvp();
  buffer_manager.UpdateBuffer("global_mvp_buffer");
}

void UpdateModelTransBuffer() {
  const float scale_factors[SCENE_SIZE] = {0.01f};
  model_trans.trans =
      glm::scale(glm::mat4(1.0f), glm::vec3(scale_factors[cur_scene_idx]));
  buffer_manager.UpdateBuffer("model_trans_buffer");
}

void UpdateComparisonBarBuffer() {
  UpdateComparisonBar();
  buffer_manager.UpdateBuffer("comparison_bar_buffer");
}

void UpdatePostprocInputsBuffer() {
  UpdatePostprocInputs();
  buffer_manager.UpdateBuffer("postproc_inputs_buffer");
}

void UpdateGLStateBuffers() {
  UpdateGlobalMvpBuffer();
  UpdateModelTransBuffer();

  UpdateComparisonBarBuffer();
  UpdatePostprocInputsBuffer();
}

void UseScreenFramebuffer(const int framebuffer_idx) {
  framebuffer_manager.BindFramebuffer("screen_framebuffer[" +
                                      std::to_string(framebuffer_idx) + "]");
}

void UseDefaultFramebuffer() {
  framebuffer_manager.BindDefaultFramebuffer(GL_FRAMEBUFFER);
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
      const GLuint unit_idx = texture_unit_idxs.at(path);
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
  program_manager.UseProgram("scene");
  const std::string scene_group_name = GetSceneGroupName(cur_scene_idx);
  const std::vector<as::Mesh> scene_meshes =
      scene_model[cur_scene_idx].GetMeshes();
  DrawMeshes("scene", scene_group_name, scene_meshes);
}

void DrawSkyboxes() {
  program_manager.UseProgram("skybox");
  const std::string skybox_group_name = GetSkyboxGroupName(cur_skybox_idx);
  const std::vector<as::Mesh> skybox_meshes =
      skybox_model[cur_skybox_idx].GetMeshes();
  DrawMeshes("skybox", skybox_group_name, skybox_meshes);
}

void DrawScreenWithTexture(const int screen_tex_idx = 0) {
  program_manager.UseProgram("postproc");

  const std::vector<as::Mesh> meshes = screen_quad_model.GetMeshes();
  for (size_t mesh_idx = 0; mesh_idx < meshes.size(); mesh_idx++) {
    // There should be only one mesh
    assert(mesh_idx <= 0);

    // Decide the names
    const std::string va_name = "screen_va";
    const std::string tex_name =
        "screen_tex[" + std::to_string(screen_tex_idx) + "]";
    const std::string buffer_name = "screen_buffer";
    const std::string idxs_buffer_name = "screen_idxs_buffer";
    const as::Mesh &mesh = meshes.at(mesh_idx);
    // Get the array indexes
    const std::vector<size_t> &idxs = mesh.GetIdxs();

    // Get the unit indexes
    const GLuint orig_unit_idx = texture_unit_idxs.at("screen_tex[0]");
    const GLuint multipass_unit_idx1 = texture_unit_idxs.at("screen_tex[1]");
    const GLuint multipass_unit_idx2 = texture_unit_idxs.at("screen_tex[2]");
    // Set the texture handlers to the unit indexes
    uniform_manager.SetUniform1Int("postproc", "screen_tex", orig_unit_idx);
    uniform_manager.SetUniform1Int("postproc", "multipass_tex1",
                                   multipass_unit_idx1);
    uniform_manager.SetUniform1Int("postproc", "multipass_tex2",
                                   multipass_unit_idx2);

    vertex_spec_manager.BindVertexArray("screen_va");
    texture_manager.BindTexture(tex_name);
    buffer_manager.BindBuffer("screen_buffer");
    buffer_manager.BindBuffer("screen_idxs_buffer");
    glDrawElements(GL_TRIANGLES, idxs.size(), GL_UNSIGNED_INT, 0);
  }
}

void DrawScreen() {
  if (cur_effect_idx == Effects::kEffectBloomEffect) {
    const int kNumMultipass = 10;
    /* Draw to framebuffer 1 with texture 0 */
    // Update the current pass index
    cur_pass_idx = 0;
    UpdatePostprocInputsBuffer();
    // Draw to screen framebuffer
    UseScreenFramebuffer(1);
    DrawScreenWithTexture(0);

    /* Draw to framebuffer 2(1) with texture 1(2) */
    for (int i = 1; i < 1 + kNumMultipass * 2; i++) {
      // Update the current pass index
      cur_pass_idx = i;
      UpdatePostprocInputsBuffer();
      // Draw to screen framebuffer
      const int source_idx = 1 + (i % 2);
      const int target_idx = 1 + ((i + 1) % 2);
      UseScreenFramebuffer(target_idx);
      DrawScreenWithTexture(source_idx);
    }

    /* Draw to framebuffer 0 with texture 1 */
    // Update the current pass index
    cur_pass_idx = 1 + kNumMultipass * 2;
    UpdatePostprocInputsBuffer();
    // Draw to default framebuffer
    UseDefaultFramebuffer();
    DrawScreenWithTexture(1);
  } else {
    DrawScreenWithTexture();
  }
}

/*******************************************************************************
 * GLUT Callbacks / Display
 ******************************************************************************/

void GLUTDisplayCallback() {
  // Update scene-wise contexts
  UpdateGLStateBuffers();
  // Draw the scenes
  UseScreenFramebuffer(0);
  ClearColorBuffer();
  ClearDepthBuffer();
  DrawScenes();
  DrawSkyboxes();
  // Draw the post-processing effects
  UseDefaultFramebuffer();
  ClearDepthBuffer();
  DrawScreen();
  // Swap double buffers
  glutSwapBuffers();
}

void GLUTReshapeCallback(const int width, const int height) {
  // Save the window size
  window_size = glm::vec2(width, height);
  // Update window aspect ratio
  window_aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
  // Set the viewport
  glViewport(0, 0, width, height);
  // Update screen textures
  UpdateScreenTextures(width, height);
  // Update screen renderbuffers
  UpdateScreenRenderbuffers(width, height);
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

void GLUTSpecialCallback(const int key, const int x, const int y) {
  switch (key) {
    case GLUT_KEY_LEFT: {
      cur_scene_idx = (cur_scene_idx + SCENE_SIZE - 1) % SCENE_SIZE;
    } break;
    case GLUT_KEY_RIGHT: {
      cur_scene_idx = (cur_scene_idx + SCENE_SIZE + 1) % SCENE_SIZE;
    } break;
    case GLUT_KEY_UP: {
      cur_skybox_idx = (cur_skybox_idx + SKYBOX_SIZE - 1) % SKYBOX_SIZE;
    } break;
    case GLUT_KEY_DOWN: {
      cur_skybox_idx = (cur_skybox_idx + SKYBOX_SIZE + 1) % SKYBOX_SIZE;
    } break;
  }
}

void GLUTMouseCallback(const int button, const int state, const int x,
                       const int y) {
  // Save mouse position
  mouse_pos = glm::vec2(x, y);
  // Check which mouse button is clicked
  switch (state) {
    case GLUT_DOWN: {
      mouse_left_down_init_pos = glm::vec2(x, y);
      mouse_left_down = true;
    } break;
    case GLUT_UP: {
      mouse_left_down = false;
    } break;
  }
}

void GLUTMouseWheelCallback(const int button, const int dir, const int x,
                            const int y) {
  if (dir > 0) {
    camera_trans.AddEye(CAMERA_ZOOMING_STEP * glm::vec3(0.0f, 0.0f, -1.0f));
  } else {
    camera_trans.AddEye(CAMERA_ZOOMING_STEP * glm::vec3(0.0f, 0.0f, 1.0f));
  }
}

void GLUTMotionCallback(const int x, const int y) {
  // Save mouse position
  mouse_pos = glm::vec2(x, y);
  // Check whether the left mouse button is down
  if (mouse_left_down) {
    switch (cur_mode) {
      case Modes::comparison: {
      } break;
      case Modes::navigation: {
        const glm::vec2 diff = mouse_pos - mouse_left_down_init_pos;
        camera_trans.AddAngle(CAMERA_ROTATION_SENSITIVITY *
                              glm::vec3(diff.y, diff.x, 0.0f));
        mouse_left_down_init_pos = mouse_pos;
      } break;
      default: { throw new std::runtime_error("Unknown mode"); }
    }
  }
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
    camera_trans.AddEye(CAMERA_MOVING_STEP * glm::vec3(0.0f, 0.0f, -1.0f));
    UpdateGlobalMvp();
  }
  if (pressed_keys['s']) {
    camera_trans.AddEye(CAMERA_MOVING_STEP * glm::vec3(0.0f, 0.0f, 1.0f));
    UpdateGlobalMvp();
  }
  if (pressed_keys['a']) {
    camera_trans.AddEye(CAMERA_MOVING_STEP * glm::vec3(-1.0f, 0.0f, 0.0f));
    UpdateGlobalMvp();
  }
  if (pressed_keys['d']) {
    camera_trans.AddEye(CAMERA_MOVING_STEP * glm::vec3(1.0f, 0.0f, 0.0f));
    UpdateGlobalMvp();
  }
  if (pressed_keys['z']) {
    camera_trans.AddEyeWorldSpace(CAMERA_MOVING_STEP *
                                  glm::vec3(0.0f, 1.0f, 0.0f));
    UpdateGlobalMvp();
  }
  if (pressed_keys['x']) {
    camera_trans.AddEyeWorldSpace(CAMERA_MOVING_STEP *
                                  glm::vec3(0.0f, -1.0f, 0.0f));
    UpdateGlobalMvp();
  }

  // Mark the current window as needing to be redisplayed
  glutPostRedisplay();

  // Register the timer callback again
  if (timer_enabled) {
    glutTimerFunc(TIMER_INTERVAL, GLUTTimerCallback, val);
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
      cur_effect_idx = kEffectImgAbs;
    } break;
    case MainMenuItems::kMainLaplacian: {
      cur_effect_idx = kEffectLaplacian;
    } break;
    case MainMenuItems::kMainSharpness: {
      cur_effect_idx = kEffectSharpness;
    } break;
    case MainMenuItems::kMainPixelation: {
      cur_effect_idx = kEffectPixelation;
    } break;
    case MainMenuItems::kMainAdvancedSep: {
    } break;
    case MainMenuItems::kMainBloomEffect: {
      cur_effect_idx = kEffectBloomEffect;
    } break;
    case MainMenuItems::kMainMagnifier: {
      cur_effect_idx = kEffectMagnifier;
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
        glutTimerFunc(TIMER_INTERVAL, GLUTTimerCallback, 0);
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
  glutTimerFunc(TIMER_INTERVAL, GLUTTimerCallback, 0);
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
