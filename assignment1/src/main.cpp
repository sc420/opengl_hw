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
bool pressed_keys[KEYBOARD_KEY_SIZE] = { false };

// Mouse states
bool camera_rotating = false;
glm::vec2 last_mouse_pos;

/*******************************************************************************
 * Object Transformations (Object states)
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

// Camera transformations
as::CameraTrans camera_trans(glm::vec3(0.0f, 0.0f, 15.0f), glm::vec3(0.0f));

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
 * GL Transformations (Feed to GL)
 ******************************************************************************/

 // Global MVP declaration
struct GlobalMvp {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

// Object transformation declaration
struct ObjTrans {
  glm::mat4 trans;
  glm::vec3 color;
};

// Global MVP
GlobalMvp global_mvp;

// Object transformations
ObjTrans obj_trans;

/*******************************************************************************
 * Objects
 ******************************************************************************/

 // Cube
std::vector<glm::vec3> cube_vertices;
std::vector<glm::vec3> cube_colors;
size_t cube_vertices_mem_sz;
// Cylinder
std::vector<glm::vec3> cylinder_vertices;
std::vector<glm::vec3> cylinder_colors;
size_t cylinder_vertices_mem_sz;
// Sphere
std::vector<glm::vec3> sphere_vertices;
std::vector<glm::vec3> sphere_colors;
size_t sphere_vertices_mem_sz;

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

void InitObjectTransformation() {
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

void LoadObjects() {
  // Cube
  as::LoadObjByTinyobj("assets/models/cube.obj", cube_vertices);
  cube_colors.assign(cube_vertices.size(), glm::vec3(1.0f, 0.0f, 0.0f));
  cube_vertices_mem_sz = cube_vertices.size() * sizeof(glm::vec3);
  // Cylinder
  as::LoadObjByTinyobj("assets/models/cylinder.obj", cylinder_vertices);
  cylinder_colors.assign(cylinder_vertices.size(), glm::vec3(0.0f, 1.0f, 0.0f));
  cylinder_vertices_mem_sz = cylinder_vertices.size() * sizeof(glm::vec3);
  // Sphere
  as::LoadObjByTinyobj("assets/models/sphere.obj", sphere_vertices);
  sphere_colors.assign(sphere_vertices.size(), glm::vec3(0.0f, 0.0f, 1.0f));
  sphere_vertices_mem_sz = sphere_vertices.size() * sizeof(glm::vec3);
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
  // Object transformation
  buffer_manager.GenBuffer("obj_trans_buffer");
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

  /* Bind buffer targets to be repeatedly used later */
  // Global MVP
  buffer_manager.BindBuffer("global_mvp_buffer", GL_UNIFORM_BUFFER);
  // Object transformation
  buffer_manager.BindBuffer("obj_trans_buffer", GL_UNIFORM_BUFFER);
  // Cube
  buffer_manager.BindBuffer("cube_buffer", GL_ARRAY_BUFFER);
  // Cylinder
  buffer_manager.BindBuffer("cylinder_buffer", GL_ARRAY_BUFFER);
  // Sphere
  buffer_manager.BindBuffer("sphere_buffer", GL_ARRAY_BUFFER);

  /* Initialize buffers */
  // Global MVP
  buffer_manager.InitBuffer("global_mvp_buffer", GL_UNIFORM_BUFFER,
    sizeof(GlobalMvp), NULL, GL_STATIC_DRAW);
  // Object transformation
  buffer_manager.InitBuffer("obj_trans_buffer", GL_UNIFORM_BUFFER,
    sizeof(ObjTrans), NULL, GL_STATIC_DRAW);
  // Cube
  buffer_manager.InitBuffer("cube_buffer", GL_ARRAY_BUFFER,
    2 * cube_vertices_mem_sz, NULL, GL_STATIC_DRAW);
  // Cylinder
  buffer_manager.InitBuffer("cylinder_buffer", GL_ARRAY_BUFFER,
    2 * cylinder_vertices_mem_sz, NULL, GL_STATIC_DRAW);
  // Sphere
  buffer_manager.InitBuffer("sphere_buffer", GL_ARRAY_BUFFER,
    2 * sphere_vertices_mem_sz, NULL, GL_STATIC_DRAW);

  /* Update buffers */
  // Global MVP
  buffer_manager.UpdateBuffer("global_mvp_buffer", GL_UNIFORM_BUFFER, 0,
    sizeof(GlobalMvp), &global_mvp);
  // Object transformation
  buffer_manager.UpdateBuffer("obj_trans_buffer", GL_UNIFORM_BUFFER, 0,
    sizeof(ObjTrans), &obj_trans);
  // Cube
  buffer_manager.UpdateBuffer("cube_buffer", GL_ARRAY_BUFFER,
    0 * cube_vertices_mem_sz, cube_vertices_mem_sz,
    cube_vertices.data());
  buffer_manager.UpdateBuffer("cube_buffer", GL_ARRAY_BUFFER,
    1 * cube_vertices_mem_sz, cube_vertices_mem_sz,
    cube_colors.data());
  // Cylinder
  buffer_manager.UpdateBuffer(
    "cylinder_buffer", GL_ARRAY_BUFFER, 0 * cylinder_vertices_mem_sz,
    cylinder_vertices_mem_sz, cylinder_vertices.data());
  buffer_manager.UpdateBuffer("cylinder_buffer", GL_ARRAY_BUFFER,
    1 * cylinder_vertices_mem_sz,
    cylinder_vertices_mem_sz, cylinder_colors.data());
  // Sphere
  buffer_manager.UpdateBuffer("sphere_buffer", GL_ARRAY_BUFFER,
    0 * sphere_vertices_mem_sz,
    sphere_vertices_mem_sz, sphere_vertices.data());
  buffer_manager.UpdateBuffer("sphere_buffer", GL_ARRAY_BUFFER,
    1 * sphere_vertices_mem_sz,
    sphere_vertices_mem_sz, sphere_colors.data());

  /* Bind uniform blocks to buffers */
  // Global MVP
  uniform_manager.AssignUniformBlockToBindingPoint("program", "global_mvp", 0);
  uniform_manager.BindBufferBaseToBindingPoint("global_mvp_buffer", 0);
  // Object transformation
  uniform_manager.AssignUniformBlockToBindingPoint("program", "obj_trans", 1);
  uniform_manager.BindBufferBaseToBindingPoint("obj_trans_buffer", 1);

  /* Bind vertex arrays to buffers */
  // Cube
  vertex_spec_manager.SpecifyVertexArrayOrg("cube_va", 0, 3, GL_FLOAT, GL_FALSE,
    0);
  vertex_spec_manager.SpecifyVertexArrayOrg("cube_va", 1, 3, GL_FLOAT, GL_FALSE,
    0);
  vertex_spec_manager.AssocVertexAttribToBindingPoint("cube_va", 0, 0);
  vertex_spec_manager.AssocVertexAttribToBindingPoint("cube_va", 1, 1);
  vertex_spec_manager.BindBufferToBindingPoint("cube_va", "cube_buffer", 0, 0,
    sizeof(glm::vec3));
  vertex_spec_manager.BindBufferToBindingPoint(
    "cube_va", "cube_buffer", 1, cube_vertices_mem_sz, sizeof(glm::vec3));
  // Cylinder
  vertex_spec_manager.SpecifyVertexArrayOrg("cylinder_va", 0, 3, GL_FLOAT,
    GL_FALSE, 0);
  vertex_spec_manager.SpecifyVertexArrayOrg("cylinder_va", 1, 3, GL_FLOAT,
    GL_FALSE, 0);
  vertex_spec_manager.AssocVertexAttribToBindingPoint("cylinder_va", 0, 0);
  vertex_spec_manager.AssocVertexAttribToBindingPoint("cylinder_va", 1, 1);
  vertex_spec_manager.BindBufferToBindingPoint("cylinder_va", "cylinder_buffer",
    0, 0, sizeof(glm::vec3));
  vertex_spec_manager.BindBufferToBindingPoint("cylinder_va", "cylinder_buffer",
    1, cylinder_vertices_mem_sz,
    sizeof(glm::vec3));
  // Sphere
  vertex_spec_manager.SpecifyVertexArrayOrg("sphere_va", 0, 3, GL_FLOAT,
    GL_FALSE, 0);
  vertex_spec_manager.SpecifyVertexArrayOrg("sphere_va", 1, 3, GL_FLOAT,
    GL_FALSE, 0);
  vertex_spec_manager.AssocVertexAttribToBindingPoint("sphere_va", 0, 0);
  vertex_spec_manager.AssocVertexAttribToBindingPoint("sphere_va", 1, 1);
  vertex_spec_manager.BindBufferToBindingPoint("sphere_va", "sphere_buffer", 0,
    0, sizeof(glm::vec3));
  vertex_spec_manager.BindBufferToBindingPoint("sphere_va", "sphere_buffer", 1,
    sphere_vertices_mem_sz,
    sizeof(glm::vec3));
}

/*******************************************************************************
 * GL Transformation Handling Methods
 ******************************************************************************/

void UpdateGlobalMvp() {
  const glm::mat4 identity(1.0f);
  global_mvp.proj =
    glm::perspective(glm::radians(45.0f), window_aspect_ratio, 0.1f, 100.0f);
  global_mvp.view = camera_trans.GetTrans();
  global_mvp.model = identity;
}

/*******************************************************************************
 * GLUT Callbacks
 ******************************************************************************/

void GLUTDisplayCallback() {
  /* Clear frame buffers */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* Use the program */
  program_manager.UseProgram("program");

  /* Update buffers */
  // Global MVP
  UpdateGlobalMvp();
  buffer_manager.UpdateBuffer("global_mvp_buffer");

  /* Draw vertex arrays */
  // Torso
  torso_trans.rotate_angle =
    static_cast<float>(0.1f * sin(ROBOT_MOVEMENT_STEP * timer_cnt));
  obj_trans.trans = torso_trans.GetTrans();
  obj_trans.color = torso_trans.GetColor();
  buffer_manager.UpdateBuffer("obj_trans_buffer");
  vertex_spec_manager.BindVertexArray("cube_va");
  glDrawArrays(GL_TRIANGLES, 0, cube_vertices.size());
  // Head
  head_trans.translate.x =
    static_cast<float>(-0.1f * sin(ROBOT_MOVEMENT_STEP * timer_cnt));
  obj_trans.trans = torso_trans.GetTransWithoutScale() * head_trans.GetTrans();
  obj_trans.color = head_trans.GetColor();
  buffer_manager.UpdateBuffer("obj_trans_buffer");
  vertex_spec_manager.BindVertexArray("sphere_va");
  glDrawArrays(GL_TRIANGLES, 0, sphere_vertices.size());
  // L1 arm
  l1_arm_trans.rotate_angle =
    static_cast<float>(0.3f * sin(ROBOT_MOVEMENT_STEP * timer_cnt));
  obj_trans.trans =
    torso_trans.GetTransWithoutScale() * l1_arm_trans.GetTrans();
  obj_trans.color = l1_arm_trans.GetColor();
  buffer_manager.UpdateBuffer("obj_trans_buffer");
  vertex_spec_manager.BindVertexArray("cylinder_va");
  glDrawArrays(GL_TRIANGLES, 0, cylinder_vertices.size());
  // L2 arm
  l2_arm_trans.rotate_angle =
    static_cast<float>(0.5f * sin(ROBOT_MOVEMENT_STEP * timer_cnt));
  obj_trans.trans = torso_trans.GetTransWithoutScale() *
    l1_arm_trans.GetTransWithoutScale() *
    l2_arm_trans.GetTrans();
  obj_trans.color = l2_arm_trans.GetColor();
  buffer_manager.UpdateBuffer("obj_trans_buffer");
  vertex_spec_manager.BindVertexArray("cylinder_va");
  glDrawArrays(GL_TRIANGLES, 0, cylinder_vertices.size());
  // R1 arm
  r1_arm_trans.rotate_angle =
    static_cast<float>(0.3f * sin(ROBOT_MOVEMENT_STEP * timer_cnt));
  obj_trans.trans =
    torso_trans.GetTransWithoutScale() * r1_arm_trans.GetTrans();
  obj_trans.color = r1_arm_trans.GetColor();
  buffer_manager.UpdateBuffer("obj_trans_buffer");
  vertex_spec_manager.BindVertexArray("cylinder_va");
  glDrawArrays(GL_TRIANGLES, 0, cylinder_vertices.size());
  // R2 arm
  r2_arm_trans.rotate_angle =
    static_cast<float>(0.5f * sin(ROBOT_MOVEMENT_STEP * timer_cnt));
  obj_trans.trans = torso_trans.GetTransWithoutScale() *
    r1_arm_trans.GetTransWithoutScale() *
    r2_arm_trans.GetTrans();
  obj_trans.color = r2_arm_trans.GetColor();
  buffer_manager.UpdateBuffer("obj_trans_buffer");
  vertex_spec_manager.BindVertexArray("cylinder_va");
  glDrawArrays(GL_TRIANGLES, 0, cylinder_vertices.size());
  // L1 leg
  l1_leg_trans.rotate_angle =
    static_cast<float>(-0.3f * sin(ROBOT_MOVEMENT_STEP * timer_cnt));
  obj_trans.trans =
    torso_trans.GetTransWithoutScale() * l1_leg_trans.GetTrans();
  obj_trans.color = l1_leg_trans.GetColor();
  buffer_manager.UpdateBuffer("obj_trans_buffer");
  vertex_spec_manager.BindVertexArray("cylinder_va");
  glDrawArrays(GL_TRIANGLES, 0, cylinder_vertices.size());
  // L2 leg
  l2_leg_trans.rotate_angle =
    static_cast<float>(-0.5f * sin(ROBOT_MOVEMENT_STEP * timer_cnt));
  obj_trans.trans = torso_trans.GetTransWithoutScale() *
    l1_leg_trans.GetTransWithoutScale() *
    l2_leg_trans.GetTrans();
  obj_trans.color = l2_leg_trans.GetColor();
  buffer_manager.UpdateBuffer("obj_trans_buffer");
  vertex_spec_manager.BindVertexArray("cylinder_va");
  glDrawArrays(GL_TRIANGLES, 0, cylinder_vertices.size());
  // R1 leg
  r1_leg_trans.rotate_angle =
    static_cast<float>(-0.3f * sin(ROBOT_MOVEMENT_STEP * timer_cnt));
  obj_trans.trans =
    torso_trans.GetTransWithoutScale() * r1_leg_trans.GetTrans();
  obj_trans.color = r1_leg_trans.GetColor();
  buffer_manager.UpdateBuffer("obj_trans_buffer");
  vertex_spec_manager.BindVertexArray("cylinder_va");
  glDrawArrays(GL_TRIANGLES, 0, cylinder_vertices.size());
  // R2 leg
  r2_leg_trans.rotate_angle =
    static_cast<float>(-0.5f * sin(ROBOT_MOVEMENT_STEP * timer_cnt));
  obj_trans.trans = torso_trans.GetTransWithoutScale() *
    r1_leg_trans.GetTransWithoutScale() *
    r2_leg_trans.GetTrans();
  obj_trans.color = r2_leg_trans.GetColor();
  buffer_manager.UpdateBuffer("obj_trans_buffer");
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
  }
  else if (state == GLUT_UP) {
    camera_rotating = false;
  }
}

void GLUTMouseWheelCallback(int button, int dir, int x, int y) {
  if (dir > 0) {
    camera_trans.AddEye(CAMERA_ZOOMING_STEP * glm::vec3(0.0f, 0.0f, -1.0f));
  }
  else {
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
    InitObjectTransformation();
    LoadObjects();
    InitGLUT(argc, argv);
    InitGLEW();
    ConfigGL();
    RegisterGLUTCallbacks();
    CreateGLUTMenus();
    EnterGLUTLoop();
  }
  catch (const std::exception &ex) {
    std::cerr << "Exception: " << ex.what() << std::endl;
    return 1;
  }
  return 0;
}
