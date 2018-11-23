#include "as/common.hpp"
#include "as/gl/common.hpp"
#include "as/model/loader.hpp"
#include "as/model/model.hpp"
#include "as/trans/camera.hpp"

/*******************************************************************************
 * Constants
 ******************************************************************************/

constexpr auto TIMER_INTERVAL = 10;
constexpr auto KEYBOARD_KEY_SIZE = 256;
constexpr auto CAMERA_MOVING_STEP = 0.2f;
constexpr auto CAMERA_ROTATION_SENSITIVITY = 0.005f;
constexpr auto CAMERA_ZOOMING_STEP = 5.0f;
constexpr auto SCENE_SIZE = 2;

/*******************************************************************************
 * Managers
 ******************************************************************************/

as::BufferManager buffer_manager;
as::ProgramManager program_manager;
as::ShaderManager shader_manager;
as::TextureManager texture_manager;
as::UniformManager uniform_manager;
as::VertexSpecManager vertex_spec_manager;

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
bool camera_rotating = false;
glm::vec2 last_mouse_pos;

/*******************************************************************************
 * Camera States
 ******************************************************************************/

// Camera transformations
as::CameraTrans camera_trans(glm::vec3(0.0f, 30.0f, 50.0f),
                             glm::vec3(0.5f, 0.0f, 0.0f));

/*******************************************************************************
 * Models
 ******************************************************************************/

// Scenes
as::Model scene_model[SCENE_SIZE];

/*******************************************************************************
 * Textures
 ******************************************************************************/

// Texture unit indexes
std::map<std::string, GLuint> texture_unit_idxs;

/*******************************************************************************
 * Model States
 ******************************************************************************/

size_t cur_scene_idx = 0;

/*******************************************************************************
 * GL States (Feed to GL)
 ******************************************************************************/

// Global MVP declaration
struct GlobalMvp {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

// Texture declaration
struct Textures {
  GLint tex_hdlr;
};

// Global MVP
GlobalMvp global_mvp;

// Textures
Textures textures;

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
  glutCreateWindow("Assignment 1");
}

void InitGLEW() {
  const GLenum err = glewInit();
  if (err != GLEW_OK) {
    std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
    throw std::runtime_error("Could not initialize GLEW");
  }
  // as::PrintGLContextInfo();
}

/*******************************************************************************
 * Model Handlers
 ******************************************************************************/

void LoadModels() {
  // First scene
  scene_model[0].LoadFile("assets/models/dabrovic-sponza/sponza.obj");
  // Second scene
  scene_model[1].LoadFile("assets/models/crytek-sponza/sponza.obj");
}

/*******************************************************************************
 * GL Context Configuration
 ******************************************************************************/

std::vector<GLubyte> ConvertChannels3To4(const std::vector<GLubyte> data) {
  const size_t size = data.size();
  const size_t num_pixel = size / 3;
  const size_t new_size = num_pixel * 4;
  std::vector<GLubyte> output(new_size, 0);
  for (size_t i = 0; i < num_pixel; i++) {
    for (size_t c = 0; c < 3; c++) {
      output[4 * i + c] = data[3 * i + c];
    }
  }
  return output;
}

void ConfigSceneBuffers() {
  for (size_t scene_idx = 0; scene_idx < SCENE_SIZE; scene_idx++) {
    const std::vector<as::Mesh> meshes = scene_model[scene_idx].GetMeshes();
    for (size_t mesh_idx = 0; mesh_idx < meshes.size(); mesh_idx++) {
      const as::Mesh &mesh = meshes.at(mesh_idx);
      // Decide the vertex spec name
      const std::string scene_va_name =
          std::to_string(scene_idx) + "va_" + std::to_string(mesh_idx);
      // Decide the buffer names
      const std::string scene_buffer_name =
          std::to_string(scene_idx) + "mesh_" + std::to_string(mesh_idx);
      const std::string scene_idxs_buffer_name = scene_buffer_name + "_idxs";
      // Get mesh data
      const std::vector<as::Vertex> vertices = mesh.GetVertices();
      const std::vector<size_t> idxs = mesh.GetIdxs();
      // Get memory size of mesh data
      const size_t vertices_mem_sz = mesh.GetVerticesMemSize();
      const size_t idxs_mem_sz = mesh.GetIdxsMemSize();

      /* Generate buffers */
      // Scene
      buffer_manager.GenBuffer(scene_buffer_name);
      // Scene array indexes
      buffer_manager.GenBuffer(scene_idxs_buffer_name);

      /* Bind buffers */
      // Scene
      buffer_manager.BindBuffer(scene_buffer_name, GL_ARRAY_BUFFER);
      // Scene array indexes
      buffer_manager.BindBuffer(scene_idxs_buffer_name,
                                GL_ELEMENT_ARRAY_BUFFER);

      /* Initialize buffers */
      // Scene
      buffer_manager.InitBuffer(scene_buffer_name, GL_ARRAY_BUFFER,
                                vertices_mem_sz, NULL, GL_STATIC_DRAW);
      // Scene array indexes
      buffer_manager.InitBuffer(scene_idxs_buffer_name, GL_ELEMENT_ARRAY_BUFFER,
                                idxs_mem_sz, NULL, GL_STATIC_DRAW);

      /* Update buffers */
      // Scene
      buffer_manager.UpdateBuffer(scene_buffer_name, GL_ARRAY_BUFFER, 0,
                                  vertices_mem_sz, vertices.data());
      // Scene array indexes
      buffer_manager.UpdateBuffer(scene_idxs_buffer_name,
                                  GL_ELEMENT_ARRAY_BUFFER, 0, idxs_mem_sz,
                                  idxs.data());

      /* Create vertex arrays */
      // Scene
      vertex_spec_manager.GenVertexArray(scene_va_name);

      /* Bind vertex arrays to buffers */
      // Scene
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
}

void ConfigSceneTextures() {
  for (size_t scene_idx = 0; scene_idx < SCENE_SIZE; scene_idx++) {
    const std::vector<as::Mesh> meshes = scene_model[scene_idx].GetMeshes();
    for (const as::Mesh mesh : meshes) {
      const std::set<as::Texture> textures = mesh.GetTextures();
      for (const as::Texture &texture : textures) {
        const std::string path = texture.GetPath();
        // Check if the texture has been loaded
        if (texture_unit_idxs.count(path) <= 0) {
          // Calculate the new unit index
          const GLuint unit_idx = texture_unit_idxs.size();
          // Load the texture
          GLsizei width, height;
          int comp;
          std::vector<GLubyte> texels;
          as::LoadTextureByStb(path, 0, width, height, comp, texels);
          // Convert the texels from 3 channels to 4 channels to avoid GL errors
          texels = ConvertChannels3To4(texels);
          // Generate the texture
          texture_manager.GenTexture(path);
          // Bind the texture
          texture_manager.BindTexture(path, GL_TEXTURE_2D, unit_idx);
          // Initialize the texture
          texture_manager.InitTexture2D(path, GL_TEXTURE_2D, 5, GL_RGBA8, width,
                                        height);
          // Update the texture
          texture_manager.UpdateTexture2D(path, GL_TEXTURE_2D, 0, 0, 0, width,
                                          height, GL_RGBA, GL_UNSIGNED_BYTE,
                                          texels.data());
          texture_manager.GenMipmap(path, GL_TEXTURE_2D);
          texture_manager.SetTextureParamInt(path, GL_TEXTURE_2D,
                                             GL_TEXTURE_MIN_FILTER,
                                             GL_LINEAR_MIPMAP_LINEAR);
          // Save the unit index
          texture_unit_idxs[path] = unit_idx;
        }
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
  shader_manager.CreateShader("vertex_shader", GL_VERTEX_SHADER,
                              "assets/shaders/vertex.vert");
  shader_manager.CreateShader("fragment_shader", GL_FRAGMENT_SHADER,
                              "assets/shaders/fragment.frag");

  /* Create programs */
  program_manager.CreateProgram("program");
  program_manager.AttachShader("program", "vertex_shader");
  program_manager.AttachShader("program", "fragment_shader");
  program_manager.LinkProgram("program");
  program_manager.UseProgram("program");

  /* Create buffers */
  // Global MVP
  buffer_manager.GenBuffer("global_mvp_buffer");
  // Model transformation
  buffer_manager.GenBuffer("model_trans_buffer");

  /* Bind buffer targets to be repeatedly used later */
  // Global MVP
  buffer_manager.BindBuffer("global_mvp_buffer", GL_UNIFORM_BUFFER);

  /* Initialize buffers */
  // Global MVP
  buffer_manager.InitBuffer("global_mvp_buffer", GL_UNIFORM_BUFFER,
                            sizeof(GlobalMvp), NULL, GL_STATIC_DRAW);

  /* Update buffers */
  // Global MVP
  buffer_manager.UpdateBuffer("global_mvp_buffer", GL_UNIFORM_BUFFER, 0,
                              sizeof(GlobalMvp), &global_mvp);

  /* Bind uniform blocks to buffers */
  // Global MVP
  uniform_manager.AssignUniformBlockToBindingPoint("program", "GlobalMvp", 0);
  uniform_manager.BindBufferBaseToBindingPoint("global_mvp_buffer", 0);

  /* Configure scenes */
  ConfigSceneBuffers();
  ConfigSceneTextures();
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

void UpdateTextures() { textures.tex_hdlr = 0; }

/*******************************************************************************
 * GLUT Callbacks
 ******************************************************************************/

void GLUTDisplayCallback() {
  /* Clear frame buffers */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* Use the program */
  program_manager.UseProgram("program");

  /* Update GL states */
  // Global MVP
  UpdateGlobalMvp();
  buffer_manager.UpdateBuffer("global_mvp_buffer");

  /* Draw the scenes */
  const std::vector<as::Mesh> meshes = scene_model[cur_scene_idx].GetMeshes();
  for (size_t mesh_idx = 0; mesh_idx < meshes.size(); mesh_idx++) {
    const as::Mesh &mesh = meshes.at(mesh_idx);
    // Decide the vertex spec name
    const std::string scene_va_name =
        std::to_string(cur_scene_idx) + "va_" + std::to_string(mesh_idx);
    // Decide the buffer names
    const std::string scene_buffer_name =
        std::to_string(cur_scene_idx) + "mesh_" + std::to_string(mesh_idx);
    const std::string scene_idxs_buffer_name = scene_buffer_name + "_idxs";
    // Get the array indexes
    const std::vector<size_t> idxs = mesh.GetIdxs();
    // Get the textures
    const std::set<as::Texture> textures = mesh.GetTextures();

    /* Update textures */
    // TODO: There is only diffuse texture
    for (const as::Texture texture : textures) {
      const std::string path = texture.GetPath();
      // Bind the texture
      texture_manager.BindTexture(path);
      // Get the unit index
      const GLuint unit_idx = texture_unit_idxs.at(path);
      // Set the texture handler to the unit index
      uniform_manager.SetUniform1Int("program", "tex_hdlr", unit_idx);
    }

    /* Draw vertex arrays */
    vertex_spec_manager.BindVertexArray(scene_va_name);
    buffer_manager.BindBuffer(scene_buffer_name);
    buffer_manager.BindBuffer(scene_idxs_buffer_name);
    glDrawElements(GL_TRIANGLES, idxs.size(), GL_UNSIGNED_INT, 0);
  }

  /* Swap frame buffers in double buffer mode */
  glutSwapBuffers();
}

void GLUTReshapeCallback(int width, int height) {
  window_aspect_ratio = static_cast<float>(width) / static_cast<float>(height);

  glViewport(0, 0, width, height);

  UpdateGlobalMvp();
}

void GLUTKeyboardCallback(unsigned char key, int x, int y) {
  pressed_keys[key] = true;
  switch (key) {
    case 'r':
      // Reset camera transformation
      camera_trans.ResetTrans();
      UpdateGlobalMvp();
      break;
    case 27:  // Escape
      glutLeaveMainLoop();
      break;
  }
}

void GLUTKeyboardUpCallback(unsigned char key, int x, int y) {
  pressed_keys[key] = false;
}

void GLUTSpecialCallback(int key, int x, int y) {
  switch (key) {
    case GLUT_KEY_F1:
      printf("F1 is pressed at (%d, %d)\n", x, y);
      break;
    case GLUT_KEY_PAGE_UP:
      printf("Page up is pressed at (%d, %d)\n", x, y);
      break;
    case GLUT_KEY_LEFT:
      cur_scene_idx = (cur_scene_idx + SCENE_SIZE - 1) % SCENE_SIZE;
      break;
    case GLUT_KEY_RIGHT:
      cur_scene_idx = (cur_scene_idx + SCENE_SIZE + 1) % SCENE_SIZE;
      break;
    default:
      printf("Other special key is pressed at (%d, %d)\n", x, y);
      break;
  }
}

void GLUTMouseCallback(int button, int state, int x, int y) {
  if (state == GLUT_DOWN) {
    if (button == GLUT_LEFT_BUTTON) {
      last_mouse_pos = glm::vec2(x, y);
      camera_rotating = true;
    }
  } else if (state == GLUT_UP) {
    camera_rotating = false;
  }
}

void GLUTMouseWheelCallback(int button, int dir, int x, int y) {
  if (dir > 0) {
    camera_trans.AddEye(CAMERA_ZOOMING_STEP * glm::vec3(0.0f, 0.0f, -1.0f));
  } else {
    camera_trans.AddEye(CAMERA_ZOOMING_STEP * glm::vec3(0.0f, 0.0f, 1.0f));
  }
}

void GLUTMotionCallback(int x, int y) {
  if (camera_rotating) {
    const glm::vec2 mouse_pos = glm::vec2(x, y);
    const glm::vec2 diff = mouse_pos - last_mouse_pos;
    camera_trans.AddAngle(CAMERA_ROTATION_SENSITIVITY *
                          glm::vec3(diff.y, diff.x, 0.0f));
    last_mouse_pos = mouse_pos;
  }
}

void GLUTTimerCallback(int val) {
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
    camera_trans.AddEye(CAMERA_MOVING_STEP * glm::vec3(0.0f, 1.0f, 0.0f));
    UpdateGlobalMvp();
  }
  if (pressed_keys['x']) {
    camera_trans.AddEye(CAMERA_MOVING_STEP * glm::vec3(0.0f, -1.0f, 0.0f));
    UpdateGlobalMvp();
  }

  glutPostRedisplay();
  if (timer_enabled) {
    glutTimerFunc(TIMER_INTERVAL, GLUTTimerCallback, val);
  }
}

void GLUTMainMenuCallback(int id) {
  switch (id) {
    case 2:
      glutChangeToMenuEntry(2, "New Label", 2);
      break;
    case 3:
      glutLeaveMainLoop();
      break;
    default:
      throw std::runtime_error("Unrecognized menu ID '" + std::to_string(id) +
                               "'");
  }
}

void GLUTTimerMenuCallback(int id) {
  switch (id) {
    case 1:
      if (!timer_enabled) {
        timer_enabled = true;
        glutTimerFunc(TIMER_INTERVAL, GLUTTimerCallback, 0);
      }
      break;
    case 2:
      timer_enabled = false;
      break;
    default:
      throw std::runtime_error("Unrecognized menu ID '" + std::to_string(id) +
                               "'");
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
    LoadModels();
    InitGLUT(argc, argv);
    InitGLEW();
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
