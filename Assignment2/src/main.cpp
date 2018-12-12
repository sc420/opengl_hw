#include "as/common.hpp"
#include "as/gl/gl_tools.hpp"
#include "as/model/model_tools.hpp"
#include "as/trans/camera.hpp"

namespace fs = std::experimental::filesystem;

/*******************************************************************************
 * Constants
 ******************************************************************************/

constexpr auto TIMER_INTERVAL = 10;
constexpr auto KEYBOARD_KEY_SIZE = 256;
constexpr auto CAMERA_MOVING_STEP = 0.2f;
constexpr auto CAMERA_ROTATION_SENSITIVITY = 0.005f;
constexpr auto CAMERA_ZOOMING_STEP = 5.0f;
constexpr auto SCENE_SIZE = 2;
constexpr auto SKYBOX_SIZE = 2;
constexpr auto NUM_MIPMAP_LEVEL = 5;

/*******************************************************************************
 * Timers
 ******************************************************************************/

unsigned int timer_cnt = 0;
bool timer_enabled = true;

/*******************************************************************************
 * User Interface States
 ******************************************************************************/

// Window states
float window_aspect_ratio;

// Keyboard states
bool pressed_keys[KEYBOARD_KEY_SIZE] = {false};

// Mouse states
bool mouse_left_down = false;
glm::vec2 mouse_left_down_init_pos;

/*******************************************************************************
 * Camera States
 ******************************************************************************/

// Camera transformations
as::CameraTrans camera_trans(glm::vec3(-12.0f, 2.0f, 0.0f),
                             glm::vec3(-0.15f * glm::half_pi<float>(),
                                       glm::half_pi<float>(), 0.0f));

/*******************************************************************************
 * Models
 ******************************************************************************/

// Scenes
as::Model scene_model[SCENE_SIZE];
// Skyboxes
as::Model skybox_model[SKYBOX_SIZE];

/*******************************************************************************
 * Textures
 ******************************************************************************/

// Texture unit indexes
std::map<std::string, GLuint> texture_unit_idxs;

/*******************************************************************************
 * Model States
 ******************************************************************************/

size_t cur_scene_idx = 0;
size_t cur_skybox_idx = 0;

/*******************************************************************************
 * GL Managers
 ******************************************************************************/

as::BufferManager buffer_manager;
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

// Global MVP
GlobalMvp global_mvp;

// Model transformations
ModelTrans model_trans;

/*******************************************************************************
 * GL Initialization Methods
 ******************************************************************************/

void InitGLUT(int argc, char *argv[]) {
  glutInit(&argc, argv);
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
  glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(600, 600);
  glutCreateWindow("Assignment 2");
}

/*******************************************************************************
 * Model Handlers
 ******************************************************************************/

void LoadModels() {
  const unsigned int flags =
      aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_Triangulate;
  // First scene
  scene_model[0].LoadFile("assets/models/crytek-sponza/sponza.obj", flags);
  // Second scene
  scene_model[1].LoadFile("assets/models/dabrovic-sponza/sponza.obj", flags);
  // First skybox
  skybox_model[0].LoadFile("assets/models/sea/skybox.obj", flags);
  // Second skybox
  skybox_model[1].LoadFile("assets/models/ame_shadow/skybox.obj", flags);
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
 * GL Context Configuration
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

    /* Bind buffers */
    // VA
    buffer_manager.BindBuffer(scene_buffer_name, GL_ARRAY_BUFFER);
    // VA indexes
    buffer_manager.BindBuffer(scene_idxs_buffer_name, GL_ELEMENT_ARRAY_BUFFER);

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

void ConfigGL() {
  as::EnableCatchingGLError();

  /* Configure GL */
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  /* Initialize managers */
  texture_manager.Init();

  /* Register managers */
  program_manager.RegisterShaderManager(shader_manager);
  uniform_manager.RegisterProgramManager(program_manager);
  uniform_manager.RegisterBufferManager(buffer_manager);
  vertex_spec_manager.RegisterBufferManager(buffer_manager);

  /* Create shaders */
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

  /* Create programs */
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

  /* Create buffers */
  // Global MVP
  buffer_manager.GenBuffer("global_mvp_buffer");
  // Model transformation
  buffer_manager.GenBuffer("model_trans_buffer");

  /* Bind buffer targets to be repeatedly used later */
  // Global MVP
  buffer_manager.BindBuffer("global_mvp_buffer", GL_UNIFORM_BUFFER);
  // Model transformation
  buffer_manager.BindBuffer("model_trans_buffer", GL_UNIFORM_BUFFER);

  /* Initialize buffers */
  // Global MVP
  buffer_manager.InitBuffer("global_mvp_buffer", GL_UNIFORM_BUFFER,
                            sizeof(GlobalMvp), NULL, GL_STATIC_DRAW);
  // Model transformation
  buffer_manager.InitBuffer("model_trans_buffer", GL_UNIFORM_BUFFER,
                            sizeof(ModelTrans), NULL, GL_STATIC_DRAW);

  /* Update buffers */
  // Global MVP
  buffer_manager.UpdateBuffer("global_mvp_buffer", GL_UNIFORM_BUFFER, 0,
                              sizeof(GlobalMvp), &global_mvp);
  // Model transformation
  buffer_manager.UpdateBuffer("model_trans_buffer", GL_UNIFORM_BUFFER, 0,
                              sizeof(ModelTrans), &model_trans);

  /* Bind uniform blocks to buffers */
  // Global MVP
  uniform_manager.AssignUniformBlockToBindingPoint("scene", "GlobalMvp", 0);
  uniform_manager.BindBufferBaseToBindingPoint("global_mvp_buffer", 0);
  // Model transformation
  uniform_manager.AssignUniformBlockToBindingPoint("scene", "ModelTrans", 1);
  uniform_manager.BindBufferBaseToBindingPoint("model_trans_buffer", 1);

  /* Configure models */
  // Scenes
  ConfigSceneBuffers();
  ConfigSceneTextures();
  // Skyboxes
  ConfigSkyboxBuffers();
  ConfigSkyboxTextures();
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

/*******************************************************************************
 * GLUT Callbacks
 ******************************************************************************/

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

void GLUTDisplayCallback() {
  /* Clear frame buffers */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* Update GL states */
  // Global MVP
  UpdateGlobalMvp();
  buffer_manager.UpdateBuffer("global_mvp_buffer");

  /* Update model transformations */
  const float scale_factors[SCENE_SIZE] = {0.01f, 1.0f};
  model_trans.trans =
      glm::scale(glm::mat4(1.0f), glm::vec3(scale_factors[cur_scene_idx]));
  buffer_manager.UpdateBuffer("model_trans_buffer");

  /* Draw the scenes */
  program_manager.UseProgram("scene");
  const std::string scene_group_name = GetSceneGroupName(cur_scene_idx);
  const std::vector<as::Mesh> scene_meshes =
      scene_model[cur_scene_idx].GetMeshes();
  DrawMeshes("scene", scene_group_name, scene_meshes);

  /* Draw the skyboxes */
  program_manager.UseProgram("skybox");
  const std::string skybox_group_name = GetSkyboxGroupName(cur_skybox_idx);
  const std::vector<as::Mesh> skybox_meshes =
      skybox_model[cur_skybox_idx].GetMeshes();
  DrawMeshes("skybox", skybox_group_name, skybox_meshes);

  /* Swap frame buffers in double buffer mode */
  glutSwapBuffers();
}

void GLUTReshapeCallback(const int width, const int height) {
  // Update window aspect ratio
  window_aspect_ratio = static_cast<float>(width) / static_cast<float>(height);

  // Set the viewport
  glViewport(0, 0, width, height);
}

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
  if (state == GLUT_DOWN) {
    if (button == GLUT_LEFT_BUTTON) {
      mouse_left_down_init_pos = glm::vec2(x, y);
      mouse_left_down = true;
    }
  } else if (state == GLUT_UP) {
    mouse_left_down = false;
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
  if (mouse_left_down) {
    const glm::vec2 mouse_pos = glm::vec2(x, y);
    const glm::vec2 diff = mouse_pos - mouse_left_down_init_pos;
    camera_trans.AddAngle(CAMERA_ROTATION_SENSITIVITY *
                          glm::vec3(diff.y, diff.x, 0.0f));
    mouse_left_down_init_pos = mouse_pos;
  }
}

void GLUTTimerCallback(const int val) {
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

void GLUTMainMenuCallback(const int id) {
  switch (id) {
    case 2: {
      glutChangeToMenuEntry(2, "New Label", 2);
    } break;
    case 3: {
      glutLeaveMainLoop();
    } break;
    default: {
      throw std::runtime_error("Unrecognized menu ID '" + std::to_string(id) +
                               "'");
    }
  }
}

void GLUTTimerMenuCallback(const int id) {
  switch (id) {
    case 1: {
      if (!timer_enabled) {
        timer_enabled = true;
        glutTimerFunc(TIMER_INTERVAL, GLUTTimerCallback, 0);
      }
    } break;
    case 2: {
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
  glutDisplayFunc(GLUTDisplayCallback);
  glutReshapeFunc(GLUTReshapeCallback);
  glutKeyboardFunc(GLUTKeyboardCallback);
  glutKeyboardUpFunc(GLUTKeyboardUpCallback);
  glutSpecialFunc(GLUTSpecialCallback);
  glutMouseFunc(GLUTMouseCallback);
  glutMouseWheelFunc(GLUTMouseWheelCallback);
  glutMotionFunc(GLUTMotionCallback);
  glutTimerFunc(TIMER_INTERVAL, GLUTTimerCallback, 0);
}

void CreateGLUTMenus() {
  const int main_menu_hdlr = glutCreateMenu(GLUTMainMenuCallback);
  const int timer_menu_hdlr = glutCreateMenu(GLUTTimerMenuCallback);

  glutSetMenu(main_menu_hdlr);

  glutAddSubMenu("Timer", timer_menu_hdlr);
  glutAddMenuEntry("Change Label", 2);
  glutAddMenuEntry("Exit", 3);

  glutSetMenu(timer_menu_hdlr);

  glutAddMenuEntry("Start", 1);
  glutAddMenuEntry("Stop", 2);

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
    LoadModels();
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
