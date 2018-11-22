#include "as/common.hpp"
#include "as/gl/common.hpp"
#include "as/model/loader.hpp"
#include "as/trans/camera.hpp"

/*******************************************************************************
 * Constants
 ******************************************************************************/

constexpr auto TIMER_INTERVAL = 10;
constexpr auto KEYBOARD_KEY_SIZE = 256;
constexpr auto CAMERA_MOVING_STEP = 0.2f;
constexpr auto CAMERA_ROTATION_SENSITIVITY = 0.01f;
constexpr auto CAMERA_ZOOMING_STEP = 5.0f;
constexpr auto ROBOT_MOVEMENT_STEP = 0.05f;

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
as::CameraTrans camera_trans(glm::vec3(0.0f, 0.0f, 15.0f), glm::vec3(0.0f));

/*******************************************************************************
 * Model States
 ******************************************************************************/

// Body part transformation declaration
class BodyPartTrans {
 public:
  BodyPartTrans()
      : pre_translate(glm::vec3(0.0f)),
        scale(glm::vec3(1.0f)),
        rotate_angle(0.0f),
        rotate_axis(glm::vec3(1.0f, 0.0f, 0.0f)),
        translate(glm::vec3(0.0f)) {}

  glm::mat4 GetTrans() const {
    const glm::mat4 identity(1.0f);
    return glm::translate(identity, translate) *
           glm::rotate(identity, rotate_angle, rotate_axis) *
           glm::scale(identity, scale) *
           glm::translate(identity, pre_translate);
  }

  glm::mat4 GetTransWithoutScale() const {
    const glm::mat4 identity(1.0f);
    return glm::translate(identity, translate) *
           glm::rotate(identity, rotate_angle, rotate_axis) *
           glm::translate(identity, pre_translate);
  }

  glm::vec3 GetColor() const { return color; }

  glm::vec3 pre_translate;

  glm::vec3 scale;
  float rotate_angle;
  glm::vec3 rotate_axis;
  glm::vec3 translate;

  glm::vec3 color;
};

// Body part transformations
BodyPartTrans torso_trans;
BodyPartTrans head_trans;
BodyPartTrans l1_arm_trans;
BodyPartTrans l2_arm_trans;
BodyPartTrans r1_arm_trans;
BodyPartTrans r2_arm_trans;
BodyPartTrans l1_leg_trans;
BodyPartTrans l2_leg_trans;
BodyPartTrans r1_leg_trans;
BodyPartTrans r2_leg_trans;

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
  glm::vec3 color;
};

// Texture declaration
struct Textures {
  GLint tex_hdlr;
};

// Global MVP
GlobalMvp global_mvp;

// Model transformations
ModelTrans model_trans;

// Textures
Textures textures;

/*******************************************************************************
 * Models
 ******************************************************************************/

// Vertex declaration
struct Vertex {
  glm::vec3 pos;
  glm::vec2 tex_coord;
};

// Cube
std::vector<Vertex> cube_vertices;
size_t cube_vertices_mem_sz;
// Cylinder
std::vector<Vertex> cylinder_vertices;
size_t cylinder_vertices_mem_sz;
// Sphere
std::vector<Vertex> sphere_vertices;
size_t sphere_vertices_mem_sz;

/*******************************************************************************
 * Textures
 ******************************************************************************/

// Metal
std::vector<GLubyte> metal_texels;
GLsizei metal_width;
GLsizei metal_height;

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

void InitModelTransformation() {
  // Torso
  torso_trans.rotate_axis = glm::vec3(0.0f, 0.0f, 1.0f);
  torso_trans.scale = glm::vec3(2.0f, 2.5f, 1.0f);
  torso_trans.color = glm::vec3(0.5f, 0.0f, 0.0f);
  // Head
  head_trans.scale = glm::vec3(1.5f);
  head_trans.translate = glm::vec3(0.0f, 2.2f, 0.0f);
  head_trans.color = glm::vec3(0.0f, 0.5f, 0.0f);
  // L1 arm
  l1_arm_trans.pre_translate = glm::vec3(0.0f, -1.0f, 0.0f);
  l1_arm_trans.scale = glm::vec3(0.8f, 0.5f, 0.8f);
  l1_arm_trans.rotate_axis = glm::vec3(1.0f, 0.0f, 0.5f);
  l1_arm_trans.translate = glm::vec3(-1.6f, 1.0f, 0.0f);
  l1_arm_trans.color = glm::vec3(0.0f, 0.0f, 0.5f);
  // L2 arm
  l2_arm_trans.pre_translate = glm::vec3(0.0f, -1.0f, 0.0f);
  l2_arm_trans.scale = glm::vec3(0.8f, 0.5f, 0.8f);
  l2_arm_trans.rotate_axis = glm::vec3(1.0f, 0.0f, 0.5f);
  l2_arm_trans.translate = glm::vec3(-0.1f, -0.2f, 0.0f);
  l2_arm_trans.color = glm::vec3(0.0f, 0.5f, 0.5f);
  // R1 arm
  r1_arm_trans.pre_translate = glm::vec3(0.0f, -1.0f, 0.0f);
  r1_arm_trans.scale = glm::vec3(0.8f, 0.5f, 0.8f);
  r1_arm_trans.rotate_axis = glm::vec3(-1.0f, 0.0f, 0.5f);
  r1_arm_trans.translate = glm::vec3(1.6f, 1.0f, 0.0f);
  r1_arm_trans.color = glm::vec3(0.0f, 0.0f, 0.5f);
  // R2 arm
  r2_arm_trans.pre_translate = glm::vec3(0.0f, -1.0f, 0.0f);
  r2_arm_trans.scale = glm::vec3(0.8f, 0.5f, 0.8f);
  r2_arm_trans.rotate_axis = glm::vec3(-1.0f, 0.0f, 0.5f);
  r2_arm_trans.translate = glm::vec3(0.1f, -0.2f, 0.0f);
  r2_arm_trans.color = glm::vec3(0.0f, 0.5f, 0.5f);
  // L1 leg
  l1_leg_trans.pre_translate = glm::vec3(0.0f, -1.0f, 0.0f);
  l1_leg_trans.scale = glm::vec3(0.8f, 0.5f, 0.8f);
  l1_leg_trans.rotate_axis = glm::vec3(1.0f, 0.0f, 0.5f);
  l1_leg_trans.translate = glm::vec3(-0.5f, -1.5f, 0.0f);
  l1_leg_trans.color = glm::vec3(0.0f, 0.0f, 0.5f);
  // L2 leg
  l2_leg_trans.pre_translate = glm::vec3(0.0f, -1.0f, 0.0f);
  l2_leg_trans.scale = glm::vec3(0.8f, 0.7f, 0.8f);
  l2_leg_trans.rotate_axis = glm::vec3(1.0f, 0.0f, 0.5f);
  l2_leg_trans.translate = glm::vec3(-0.1f, -0.2f, 0.0f);
  l2_leg_trans.color = glm::vec3(0.0f, 0.5f, 0.5f);
  // R1 leg
  r1_leg_trans.pre_translate = glm::vec3(0.0f, -1.0f, 0.0f);
  r1_leg_trans.scale = glm::vec3(0.8f, 0.5f, 0.8f);
  r1_leg_trans.rotate_axis = glm::vec3(-1.0f, 0.0f, 0.5f);
  r1_leg_trans.translate = glm::vec3(0.5f, -1.5f, 0.0f);
  r1_leg_trans.color = glm::vec3(0.0f, 0.0f, 0.5f);
  // R2 leg
  r2_leg_trans.pre_translate = glm::vec3(0.0f, -1.0f, 0.0f);
  r2_leg_trans.scale = glm::vec3(0.8f, 0.7f, 0.8f);
  r2_leg_trans.rotate_axis = glm::vec3(-1.0f, 0.0f, 0.5f);
  r2_leg_trans.translate = glm::vec3(0.1f, -0.2f, 0.0f);
  r2_leg_trans.color = glm::vec3(0.0f, 0.5f, 0.5f);
}

void LoadModels() {
  std::vector<glm::vec3> positions;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec2> tex_coords;
  // Cube
  as::LoadModelByTinyobj("assets/models/cube.obj", positions, normals,
                         tex_coords);
  cube_vertices.clear();
  for (size_t i = 0; i < positions.size(); i++) {
    Vertex vertex = {positions.at(i), tex_coords.at(i)};
    cube_vertices.push_back(vertex);
  }
  cube_vertices_mem_sz = cube_vertices.size() * sizeof(Vertex);
  // Cylinder
  as::LoadModelByTinyobj("assets/models/cylinder.obj", positions, normals,
                         tex_coords);
  cylinder_vertices.clear();
  for (size_t i = 0; i < positions.size(); i++) {
    Vertex vertex = {positions.at(i), tex_coords.at(i)};
    cylinder_vertices.push_back(vertex);
  }
  cylinder_vertices_mem_sz = cylinder_vertices.size() * sizeof(Vertex);
  // Sphere
  as::LoadModelByTinyobj("assets/models/sphere.obj", positions, normals,
                         tex_coords);
  sphere_vertices.clear();
  for (size_t i = 0; i < positions.size(); i++) {
    Vertex vertex = {positions.at(i), tex_coords.at(i)};
    sphere_vertices.push_back(vertex);
  }
  sphere_vertices_mem_sz = sphere_vertices.size() * sizeof(Vertex);
}

void LoadTextures() {
  int comp;
  // Metal
  as::LoadTextureByStb("assets/textures/metal.png", 4, metal_width,
                       metal_height, comp, metal_texels);
}

/*******************************************************************************
 * GL Context Configuration
 ******************************************************************************/

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
  // Cube
  buffer_manager.GenBuffer("cube_buffer");
  // Cylinder
  buffer_manager.GenBuffer("cylinder_buffer");
  // Sphere
  buffer_manager.GenBuffer("sphere_buffer");

  /* Create vertex arrays */
  // Cube
  vertex_spec_manager.GenVertexArray("cube_va");
  // Cylinder
  vertex_spec_manager.GenVertexArray("cylinder_va");
  // Sphere
  vertex_spec_manager.GenVertexArray("sphere_va");

  /* Create textures */
  // Metal
  texture_manager.GenTexture("metal_tex");

  /* Bind buffer targets to be repeatedly used later */
  // Global MVP
  buffer_manager.BindBuffer("global_mvp_buffer", GL_UNIFORM_BUFFER);
  // Model transformation
  buffer_manager.BindBuffer("model_trans_buffer", GL_UNIFORM_BUFFER);
  // Cube
  buffer_manager.BindBuffer("cube_buffer", GL_ARRAY_BUFFER);
  // Cylinder
  buffer_manager.BindBuffer("cylinder_buffer", GL_ARRAY_BUFFER);
  // Sphere
  buffer_manager.BindBuffer("sphere_buffer", GL_ARRAY_BUFFER);

  /* Bind textures to be repeatedly used later */
  // Metal
  texture_manager.BindTexture("metal_tex", GL_TEXTURE_2D, 0);

  /* Initialize buffers */
  // Global MVP
  buffer_manager.InitBuffer("global_mvp_buffer", GL_UNIFORM_BUFFER,
                            sizeof(GlobalMvp), NULL, GL_STATIC_DRAW);
  // Model transformation
  buffer_manager.InitBuffer("model_trans_buffer", GL_UNIFORM_BUFFER,
                            sizeof(ModelTrans), NULL, GL_STATIC_DRAW);
  // Cube
  buffer_manager.InitBuffer("cube_buffer", GL_ARRAY_BUFFER,
                            cube_vertices_mem_sz, NULL, GL_STATIC_DRAW);
  // Cylinder
  buffer_manager.InitBuffer("cylinder_buffer", GL_ARRAY_BUFFER,
                            cylinder_vertices_mem_sz, NULL, GL_STATIC_DRAW);
  // Sphere
  buffer_manager.InitBuffer("sphere_buffer", GL_ARRAY_BUFFER,
                            sphere_vertices_mem_sz, NULL, GL_STATIC_DRAW);

  /* Initialize textures */
  // Metal
  texture_manager.InitTexture2D("metal_tex", GL_TEXTURE_2D, 1, GL_RGBA8,
                                metal_width, metal_height);

  /* Update buffers */
  // Global MVP
  buffer_manager.UpdateBuffer("global_mvp_buffer", GL_UNIFORM_BUFFER, 0,
                              sizeof(GlobalMvp), &global_mvp);
  // Model transformation
  buffer_manager.UpdateBuffer("model_trans_buffer", GL_UNIFORM_BUFFER, 0,
                              sizeof(ModelTrans), &model_trans);
  // Cube
  buffer_manager.UpdateBuffer("cube_buffer", GL_ARRAY_BUFFER, 0,
                              cube_vertices_mem_sz, cube_vertices.data());
  // Cylinder
  buffer_manager.UpdateBuffer("cylinder_buffer", GL_ARRAY_BUFFER, 0,
                              cylinder_vertices_mem_sz,
                              cylinder_vertices.data());
  // Sphere
  buffer_manager.UpdateBuffer("sphere_buffer", GL_ARRAY_BUFFER, 0,
                              sphere_vertices_mem_sz, sphere_vertices.data());

  /* Update textures */
  // Metal
  texture_manager.UpdateTexture2D("metal_tex", GL_TEXTURE_2D, 0, 0, 0,
                                  metal_width, metal_height, GL_RGBA,
                                  GL_UNSIGNED_BYTE, metal_texels.data());
  texture_manager.GenMipmap("metal_tex", GL_TEXTURE_2D);

  /* Bind uniform blocks to buffers */
  // Global MVP
  uniform_manager.AssignUniformBlockToBindingPoint("program", "GlobalMvp", 0);
  uniform_manager.BindBufferBaseToBindingPoint("global_mvp_buffer", 0);
  // Model transformation
  uniform_manager.AssignUniformBlockToBindingPoint("program", "ModelTrans", 1);
  uniform_manager.BindBufferBaseToBindingPoint("model_trans_buffer", 1);

  /* Bind vertex arrays to buffers */
  // Cube
  vertex_spec_manager.SpecifyVertexArrayOrg("cube_va", 0, 3, GL_FLOAT, GL_FALSE,
                                            0);
  vertex_spec_manager.SpecifyVertexArrayOrg("cube_va", 1, 3, GL_FLOAT, GL_FALSE,
                                            0);
  vertex_spec_manager.AssocVertexAttribToBindingPoint("cube_va", 0, 0);
  vertex_spec_manager.AssocVertexAttribToBindingPoint("cube_va", 1, 1);
  vertex_spec_manager.BindBufferToBindingPoint(
      "cube_va", "cube_buffer", 0, offsetof(Vertex, pos), sizeof(Vertex));
  vertex_spec_manager.BindBufferToBindingPoint(
      "cube_va", "cube_buffer", 1, offsetof(Vertex, tex_coord), sizeof(Vertex));
  // Cylinder
  vertex_spec_manager.SpecifyVertexArrayOrg("cylinder_va", 0, 3, GL_FLOAT,
                                            GL_FALSE, 0);
  vertex_spec_manager.SpecifyVertexArrayOrg("cylinder_va", 1, 3, GL_FLOAT,
                                            GL_FALSE, 0);
  vertex_spec_manager.AssocVertexAttribToBindingPoint("cylinder_va", 0, 0);
  vertex_spec_manager.AssocVertexAttribToBindingPoint("cylinder_va", 1, 1);
  vertex_spec_manager.BindBufferToBindingPoint("cylinder_va", "cylinder_buffer",
                                               0, offsetof(Vertex, pos),
                                               sizeof(Vertex));
  vertex_spec_manager.BindBufferToBindingPoint("cylinder_va", "cylinder_buffer",
                                               1, offsetof(Vertex, tex_coord),
                                               sizeof(Vertex));
  // Sphere
  vertex_spec_manager.SpecifyVertexArrayOrg("sphere_va", 0, 3, GL_FLOAT,
                                            GL_FALSE, 0);
  vertex_spec_manager.SpecifyVertexArrayOrg("sphere_va", 1, 3, GL_FLOAT,
                                            GL_FALSE, 0);
  vertex_spec_manager.AssocVertexAttribToBindingPoint("sphere_va", 0, 0);
  vertex_spec_manager.AssocVertexAttribToBindingPoint("sphere_va", 1, 1);
  vertex_spec_manager.BindBufferToBindingPoint(
      "sphere_va", "sphere_buffer", 0, offsetof(Vertex, pos), sizeof(Vertex));
  vertex_spec_manager.BindBufferToBindingPoint("sphere_va", "sphere_buffer", 1,
                                               offsetof(Vertex, tex_coord),
                                               sizeof(Vertex));
}

/*******************************************************************************
 * GL States Handling Methods
 ******************************************************************************/

void UpdateGlobalMvp() {
  const glm::mat4 identity(1.0f);
  global_mvp.proj =
      glm::perspective(glm::radians(45.0f), window_aspect_ratio, 0.1f, 100.0f);
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

  /* Update textures */
  // Textures
  UpdateTextures();
  uniform_manager.SetUniform1Int("program", "tex_hdlr", textures.tex_hdlr);

  /* Draw vertex arrays */
  // Torso
  torso_trans.rotate_angle =
      static_cast<float>(0.1f * sin(ROBOT_MOVEMENT_STEP * timer_cnt));
  model_trans.trans = torso_trans.GetTrans();
  model_trans.color = torso_trans.GetColor();
  buffer_manager.UpdateBuffer("model_trans_buffer");
  vertex_spec_manager.BindVertexArray("cube_va");
  glDrawArrays(GL_TRIANGLES, 0, cube_vertices.size());
  // Head
  head_trans.translate.x =
      static_cast<float>(-0.1f * sin(ROBOT_MOVEMENT_STEP * timer_cnt));
  model_trans.trans =
      torso_trans.GetTransWithoutScale() * head_trans.GetTrans();
  model_trans.color = head_trans.GetColor();
  buffer_manager.UpdateBuffer("model_trans_buffer");
  vertex_spec_manager.BindVertexArray("sphere_va");
  glDrawArrays(GL_TRIANGLES, 0, sphere_vertices.size());
  // L1 arm
  l1_arm_trans.rotate_angle =
      static_cast<float>(0.3f * sin(ROBOT_MOVEMENT_STEP * timer_cnt));
  model_trans.trans =
      torso_trans.GetTransWithoutScale() * l1_arm_trans.GetTrans();
  model_trans.color = l1_arm_trans.GetColor();
  buffer_manager.UpdateBuffer("model_trans_buffer");
  vertex_spec_manager.BindVertexArray("cylinder_va");
  glDrawArrays(GL_TRIANGLES, 0, cylinder_vertices.size());
  // L2 arm
  l2_arm_trans.rotate_angle =
      static_cast<float>(0.5f * sin(ROBOT_MOVEMENT_STEP * timer_cnt));
  model_trans.trans = torso_trans.GetTransWithoutScale() *
                      l1_arm_trans.GetTransWithoutScale() *
                      l2_arm_trans.GetTrans();
  model_trans.color = l2_arm_trans.GetColor();
  buffer_manager.UpdateBuffer("model_trans_buffer");
  vertex_spec_manager.BindVertexArray("cylinder_va");
  glDrawArrays(GL_TRIANGLES, 0, cylinder_vertices.size());
  // R1 arm
  r1_arm_trans.rotate_angle =
      static_cast<float>(0.3f * sin(ROBOT_MOVEMENT_STEP * timer_cnt));
  model_trans.trans =
      torso_trans.GetTransWithoutScale() * r1_arm_trans.GetTrans();
  model_trans.color = r1_arm_trans.GetColor();
  buffer_manager.UpdateBuffer("model_trans_buffer");
  vertex_spec_manager.BindVertexArray("cylinder_va");
  glDrawArrays(GL_TRIANGLES, 0, cylinder_vertices.size());
  // R2 arm
  r2_arm_trans.rotate_angle =
      static_cast<float>(0.5f * sin(ROBOT_MOVEMENT_STEP * timer_cnt));
  model_trans.trans = torso_trans.GetTransWithoutScale() *
                      r1_arm_trans.GetTransWithoutScale() *
                      r2_arm_trans.GetTrans();
  model_trans.color = r2_arm_trans.GetColor();
  buffer_manager.UpdateBuffer("model_trans_buffer");
  vertex_spec_manager.BindVertexArray("cylinder_va");
  glDrawArrays(GL_TRIANGLES, 0, cylinder_vertices.size());
  // L1 leg
  l1_leg_trans.rotate_angle =
      static_cast<float>(-0.3f * sin(ROBOT_MOVEMENT_STEP * timer_cnt));
  model_trans.trans =
      torso_trans.GetTransWithoutScale() * l1_leg_trans.GetTrans();
  model_trans.color = l1_leg_trans.GetColor();
  buffer_manager.UpdateBuffer("model_trans_buffer");
  vertex_spec_manager.BindVertexArray("cylinder_va");
  glDrawArrays(GL_TRIANGLES, 0, cylinder_vertices.size());
  // L2 leg
  l2_leg_trans.rotate_angle =
      static_cast<float>(-0.5f * sin(ROBOT_MOVEMENT_STEP * timer_cnt));
  model_trans.trans = torso_trans.GetTransWithoutScale() *
                      l1_leg_trans.GetTransWithoutScale() *
                      l2_leg_trans.GetTrans();
  model_trans.color = l2_leg_trans.GetColor();
  buffer_manager.UpdateBuffer("model_trans_buffer");
  vertex_spec_manager.BindVertexArray("cylinder_va");
  glDrawArrays(GL_TRIANGLES, 0, cylinder_vertices.size());
  // R1 leg
  r1_leg_trans.rotate_angle =
      static_cast<float>(-0.3f * sin(ROBOT_MOVEMENT_STEP * timer_cnt));
  model_trans.trans =
      torso_trans.GetTransWithoutScale() * r1_leg_trans.GetTrans();
  model_trans.color = r1_leg_trans.GetColor();
  buffer_manager.UpdateBuffer("model_trans_buffer");
  vertex_spec_manager.BindVertexArray("cylinder_va");
  glDrawArrays(GL_TRIANGLES, 0, cylinder_vertices.size());
  // R2 leg
  r2_leg_trans.rotate_angle =
      static_cast<float>(-0.5f * sin(ROBOT_MOVEMENT_STEP * timer_cnt));
  model_trans.trans = torso_trans.GetTransWithoutScale() *
                      r1_leg_trans.GetTransWithoutScale() *
                      r2_leg_trans.GetTrans();
  model_trans.color = r2_leg_trans.GetColor();
  buffer_manager.UpdateBuffer("model_trans_buffer");
  vertex_spec_manager.BindVertexArray("cylinder_va");
  glDrawArrays(GL_TRIANGLES, 0, cylinder_vertices.size());

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
      printf("Left arrow is pressed at (%d, %d)\n", x, y);
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
    InitModelTransformation();
    LoadModels();
    LoadTextures();
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
