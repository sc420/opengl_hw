#pragma warning(push, 0)
#include "assignment/common.hpp"
#pragma warning(pop)
#include "assignment/buffer.hpp"
#include "assignment/program.hpp"
#include "assignment/shader.hpp"
#include "assignment/uniform.hpp"
#include "assignment/vertex_spec.hpp"
#include "assignment/object.hpp"

/*******************************************************************************
 * Constants
 ******************************************************************************/

constexpr auto TIMER_INTERVAL = 10;
constexpr auto ROTATION_STEP = 0.1f;

/*******************************************************************************
 * Managers
 ******************************************************************************/

BufferManager buffer_manager;
ProgramManager program_manager;
ShaderManager shader_manager;
UniformManager uniform_manager;
VertexSpecManager vertex_spec_manager;

/*******************************************************************************
 * Timer
 ******************************************************************************/

unsigned int timer_cnt = 0;
bool timer_enabled = true;

/*******************************************************************************
 * Transformation
 ******************************************************************************/

struct Mvp {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

Mvp mvp;

glm::vec3 rot_deg;

/*******************************************************************************
 * Objects
 ******************************************************************************/

std::vector<glm::vec3> cube_vertices;
std::vector<glm::vec3> cube_colors;
size_t cube_vertices_mem_sz;

void InitGLUT(int argc, char* argv[]) {
  glutInit(&argc, argv);
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(600, 600);
  glutCreateWindow("Assignment 1");
}

void InitGLEW() {
  GLenum err = glewInit();
  if (err != GLEW_OK) {
    std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
    throw std::runtime_error("Could not initialize GLEW");
  }
  // DumpGLInfo();
}

void InitTransformation() {
  rot_deg = glm::vec3(0.0f, 0.0f, 0.0f);
}

void LoadObjects() {
  TinyobjLoadObj("cube.obj", cube_vertices);
  // All red colors
  cube_colors.assign(cube_vertices.size(), glm::vec3(1.0f, 0.0f, 0.0f));
  cube_vertices_mem_sz = cube_vertices.size() * sizeof(glm::vec3);
}

void ConfigGL() {
  EnableCatchingError();

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
                              "vertex.vs.glsl");
  shader_manager.CreateShader("fragment_shader", GL_FRAGMENT_SHADER,
                              "fragment.fs.glsl");

  /* Create programs */
  program_manager.CreateProgram("program");
  program_manager.AttachShader("program", "vertex_shader");
  program_manager.AttachShader("program", "fragment_shader");
  program_manager.LinkProgram("program");
  program_manager.UseProgram("program");

  /* Create buffers */
  // va
  //buffer_manager.GenBuffer("va_buffer");
  // MVP
  buffer_manager.GenBuffer("mvp_buffer");
  // Head
  buffer_manager.GenBuffer("head_buffer");

  /* Create vertex arrays */
  // va
  //vertex_spec_manager.GenVertexArray("va");
  // Head
  vertex_spec_manager.GenVertexArray("head_va");

  /* Bind buffer targets to be repeatedly used later */
  // VA
  //buffer_manager.BindBuffer("va_buffer", GL_ARRAY_BUFFER);
  // MVP
  buffer_manager.BindBuffer("mvp_buffer", GL_UNIFORM_BUFFER);
  // Head
  buffer_manager.BindBuffer("head_buffer", GL_ARRAY_BUFFER);

  /* Initialize buffers */
  // va
  //buffer_manager.InitBuffer("va_buffer", GL_ARRAY_BUFFER, 18 * sizeof(float),
  //                          NULL, GL_STATIC_DRAW);
  // MVP
  buffer_manager.InitBuffer("mvp_buffer", GL_UNIFORM_BUFFER,
                            3 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
  // Head
  buffer_manager.InitBuffer("head_buffer", GL_ARRAY_BUFFER, 2 * cube_vertices_mem_sz, NULL, GL_STATIC_DRAW);

  /* Update buffers */
  // MVP
  buffer_manager.UpdateBuffer("mvp_buffer", GL_UNIFORM_BUFFER, 0, sizeof(Mvp),
                              &mvp);
  // Head
  buffer_manager.UpdateBuffer("head_buffer", GL_ARRAY_BUFFER, 0 * cube_vertices_mem_sz, cube_vertices_mem_sz, cube_vertices.data());
  buffer_manager.UpdateBuffer("head_buffer", GL_ARRAY_BUFFER, 1 * cube_vertices_mem_sz, cube_vertices_mem_sz, cube_colors.data());

  /* Bind uniform blocks to buffers */
  uniform_manager.AssignUniformBlockToBindingPoint("program", "mvp", 0);
  uniform_manager.BindBufferBaseToBindingPoint("mvp_buffer", 0);

  /* Bind vertex arrays to buffers */
  // va
  //vertex_spec_manager.SpecifyVertexArrayOrg("va", 0, 3, GL_FLOAT, GL_FALSE, 0);
  //vertex_spec_manager.SpecifyVertexArrayOrg("va", 1, 3, GL_FLOAT, GL_FALSE,
  //                                          3 * 3 * sizeof(float));
  //vertex_spec_manager.AssocVertexAttribToBindingPoint("va", 0, 0);
  //vertex_spec_manager.AssocVertexAttribToBindingPoint("va", 1, 1);
  //vertex_spec_manager.BindBufferToBindingPoint("va", "va_buffer", 0, 0,
  //                                             3 * sizeof(float));
  //vertex_spec_manager.BindBufferToBindingPoint("va", "va_buffer", 1, 0,
  //                                             3 * sizeof(float));
  // Head
  vertex_spec_manager.SpecifyVertexArrayOrg("head_va", 0, 3, GL_FLOAT, GL_FALSE, 0);
  vertex_spec_manager.SpecifyVertexArrayOrg("head_va", 1, 3, GL_FLOAT, GL_FALSE, 0);
  vertex_spec_manager.AssocVertexAttribToBindingPoint("head_va", 0, 0);
  vertex_spec_manager.AssocVertexAttribToBindingPoint("head_va", 1, 1);
  vertex_spec_manager.BindBufferToBindingPoint("head_va", "head_buffer", 0, 0,
    sizeof(glm::vec3));
  vertex_spec_manager.BindBufferToBindingPoint("head_va", "head_buffer", 1, cube_vertices_mem_sz,
    sizeof(glm::vec3));
}

void GLUTDisplayCallback() {
  /* Clear frame buffers */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* Use the program */
  program_manager.UseProgram("program");
  
  /* Update buffers */
  // va
  //float f_timer_cnt = ((5 * timer_cnt) % 255) / 255.0f;
  //float data[18] = {-0.5f,       -0.4f, 0.0f, 0.5f,        -0.4f, 0.0f,
  //                  0.0f,        0.6f,  0.0f, f_timer_cnt, 0.0f,  0.0f,
  //                  f_timer_cnt, 0.0f,  0.0f, f_timer_cnt, 0.0f,  0.0f};
  //buffer_manager.BindBuffer("va_buffer");
  //buffer_manager.UpdateBuffer("va_buffer", GL_ARRAY_BUFFER, 0,
  //                            18 * sizeof(float), data);

  // MVP
  buffer_manager.BindBuffer("mvp_buffer");
  buffer_manager.UpdateBuffer("mvp_buffer");

  // Head
  buffer_manager.BindBuffer("head_buffer");
  buffer_manager.UpdateBuffer("head_buffer");

  /* Draw vertex arrays */
  // va
  //vertex_spec_manager.BindVertexArray("va");
  //glDrawArrays(GL_TRIANGLES, 0, 3);
  // Head
  vertex_spec_manager.BindVertexArray("head_va");
  glDrawArrays(GL_TRIANGLES, 0, cube_vertices.size());

  /* Swap frame buffers in double buffer mode */
  glutSwapBuffers();
}

void GLUTReshapeCallback(int width, int height) {
  const float ratio = static_cast<float>(width) / static_cast<float>(height);

  glViewport(0, 0, width, height);

  //constexpr float WIDTH = 10.0f;
  //constexpr float HEIGHT = 10.0f;
  //mvp.proj = glm::ortho(-1.0f * WIDTH * ratio, WIDTH * ratio, -1.0f * HEIGHT, HEIGHT);
  mvp.proj = glm::perspective(glm::radians(45.0f), ratio, 0.1f, 100.0f);
  mvp.view =
      glm::lookAt(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, -1.0f),
                  glm::vec3(0.0f, 1.0f, 0.0f));
  mvp.model = glm::toMat4(glm::quat(rot_deg));
}

void GLUTMouseCallback(int button, int state, int x, int y) {
  if (state == GLUT_DOWN) {
    printf("Mouse %d is pressed at (%d, %d)\n", button, x, y);
  } else if (state == GLUT_UP) {
    printf("Mouse %d is released at (%d, %d)\n", button, x, y);
  }
}

void GLUTKeyboardCallback(unsigned char key, int x, int y) {
  printf("Key %c is pressed at (%d, %d)\n", key, x, y);
  switch (key) {
    case 'w':
      rot_deg += ROTATION_STEP * glm::vec3(-1.0, 0.0f, 0.0f);
      mvp.model = glm::toMat4(glm::quat(rot_deg));
      break;
    case 's':
      rot_deg += ROTATION_STEP * glm::vec3(1.0f, 0.0f, 0.0f);
      mvp.model = glm::toMat4(glm::quat(rot_deg));
      break;
    case 'a':
      rot_deg += ROTATION_STEP * glm::vec3(0.0f, -1.0f, 0.0f);
      mvp.model = glm::toMat4(glm::quat(rot_deg));
      break;
    case 'd':
      rot_deg += ROTATION_STEP * glm::vec3(0.0f, 1.0f, 0.0f);
      mvp.model = glm::toMat4(glm::quat(rot_deg));
      break;
    case 27:  // Escape
      glutLeaveMainLoop();
      break;
    default:
      break;
  }
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

void GLUTTimerCallback(int val) {
  timer_cnt++;
  glutPostRedisplay();
  if (timer_enabled) {
    glutTimerFunc(TIMER_INTERVAL, GLUTTimerCallback, val);
  }
}

void GLUTMainMenuCallback(int id) {
  switch (id) {
    case 2:
      glutChangeToMenuEntry(2, "New label", 2);
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

void RegisterGLUTCallbacks() {
  glutDisplayFunc(GLUTDisplayCallback);
  glutReshapeFunc(GLUTReshapeCallback);
  glutMouseFunc(GLUTMouseCallback);
  glutKeyboardFunc(GLUTKeyboardCallback);
  glutSpecialFunc(GLUTSpecialCallback);
  glutTimerFunc(TIMER_INTERVAL, GLUTTimerCallback, 0);
}

void CreateGLUTMenus() {
  int main_menu_hdlr = glutCreateMenu(GLUTMainMenuCallback);
  int timer_menu_hdlr = glutCreateMenu(GLUTTimerMenuCallback);

  glutSetMenu(main_menu_hdlr);

  glutAddSubMenu("Timer", timer_menu_hdlr);
  glutAddMenuEntry("Change label", 2);
  glutAddMenuEntry("Exit", 3);

  glutSetMenu(timer_menu_hdlr);

  glutAddMenuEntry("Start", 1);
  glutAddMenuEntry("Stop", 2);

  glutSetMenu(main_menu_hdlr);

  glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void EnterGLUTLoop() { glutMainLoop(); }

int main(int argc, char* argv[]) {
  try {
    InitGLUT(argc, argv);
    InitGLEW();
    InitTransformation();
    LoadObjects();
    ConfigGL();
    RegisterGLUTCallbacks();
    CreateGLUTMenus();
    EnterGLUTLoop();
  } catch (const std::exception& ex) {
    std::cerr << "Exception: " << ex.what() << std::endl;
    return 1;
  }
  return 0;
}
