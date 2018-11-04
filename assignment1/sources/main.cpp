#pragma warning(push, 0)
#include "assignment/common.hpp"
#pragma warning(pop)
#include "assignment/shader.hpp"
#include "assignment/uniform.hpp"

#define MENU_TIMER_START 1
#define MENU_TIMER_STOP 2
#define MENU_EXIT 3

#define MENU_SHADER_SIN 4
#define MENU_SHADER_BRICK 5

GLubyte timer_cnt = 0;
bool timer_enabled = true;
unsigned int timer_speed = 16;

glm::mat4 mvp;
GLuint um4mvp;

/*******************************************************************************
 * Constants
 ******************************************************************************/

constexpr auto MOUSE_SENSITIVITY = 1.0f;
constexpr auto MOUSE_DIV_FACTOR = 300.0f;

/*******************************************************************************
 * Programs
 ******************************************************************************/

GLuint program;

/*******************************************************************************
 * Buffers
 ******************************************************************************/

GLuint buffer;
GLuint mvp_buffer_hdlr;

/*******************************************************************************
 * Transformation
 ******************************************************************************/

 // MVP
glm::mat4 model;
glm::mat4 view;
glm::mat4 proj;

/*******************************************************************************
 * Managers
 ******************************************************************************/

ShaderManager shader_manager;
UniformManager uniform_manager;

void My_Init() {
  EnableCatchingError();

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  // Create the shaders
  shader_manager.CreateShader(GL_VERTEX_SHADER, "vertex.vs.glsl", "vertex_shader");
  shader_manager.CreateShader(GL_FRAGMENT_SHADER, "fragment.fs.glsl", "fragment_shader");
  GLuint vertexShader = shader_manager.GetShaderHdlr("vertex_shader");
  GLuint fragmentShader = shader_manager.GetShaderHdlr("fragment_shader");

  program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);
  glUseProgram(program);
  um4mvp = glGetUniformLocation(program, "um4mvp");

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glGenBuffers(1, &buffer);
  glBindBuffer(GL_ARRAY_BUFFER, buffer);

  // Bind data

  glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), NULL, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0,
    reinterpret_cast<GLvoid*>(sizeof(float) * 9));

  // TODO: Add program manager and buffer manager
  // TODO: delete buffer

  glGenBuffers(1, &mvp_buffer_hdlr);
  glBindBuffer(GL_UNIFORM_BUFFER, mvp_buffer_hdlr);

  glBufferData(GL_UNIFORM_BUFFER, 3 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);

  uniform_manager.RegisterProgram(program, "program");
  uniform_manager.RegisterBuffer(mvp_buffer_hdlr, "mvp_buffer");
  uniform_manager.AssignBindingPoint("program", "mvp", 0);
  uniform_manager.BindBufferToBindingPoint(0, "mvp_buffer");
}

// GLUT callback. Called to draw the scene.
void My_Display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(program);

  float f_timer_cnt = ((5 * timer_cnt) % 255) / 255.0f;
  // TODO: Generate sphere by octahedron

  float data[18] = { -0.5f,       -0.4f, 0.0f, 0.5f,        -0.4f, 0.0f,
                    0.0f,        0.6f,  0.0f, f_timer_cnt, 0.0f,  0.0f,
                    f_timer_cnt, 0.0f,  0.0f, f_timer_cnt, 0.0f,  0.0f };

  glBindBuffer(GL_ARRAY_BUFFER, buffer);

  glBufferSubData(GL_ARRAY_BUFFER, 0, 18 * sizeof(float), data);

  glUniformMatrix4fv(um4mvp, 1, GL_FALSE, glm::value_ptr(mvp));

  glBindBuffer(GL_UNIFORM_BUFFER, mvp_buffer_hdlr);
  glBufferSubData(GL_UNIFORM_BUFFER, 0 * sizeof(glm::mat4), sizeof(glm::mat4),
    glm::value_ptr(model));
  glBufferSubData(GL_UNIFORM_BUFFER, 1 * sizeof(glm::mat4), sizeof(glm::mat4),
    glm::value_ptr(view));
  glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::mat4),
    glm::value_ptr(proj));

  glDrawArrays(GL_TRIANGLES, 0, 3);

  glutSwapBuffers();
}

void My_Reshape(int width, int height) {
  float viewportAspect = (float)width / (float)height;

  glViewport(0, 0, width, height);

  mvp = glm::ortho(-1 * viewportAspect, 1 * viewportAspect, -1.0f, 1.0f);
  mvp = mvp * glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));
  // TODO: Use explicit model, view and projection and store in buffer-backed
  // buffer
  proj = glm::ortho(-1 * viewportAspect, 1 * viewportAspect, -1.0f, 1.0f);
  model = glm::mat4();
}

void My_Timer(int val) {
  timer_cnt++;
  glutPostRedisplay();
  if (timer_enabled) {
    glutTimerFunc(timer_speed, My_Timer, val);
  }
}

void My_Mouse(int button, int state, int x, int y) {
  if (state == GLUT_DOWN) {
    printf("Mouse %d is pressed at (%d, %d)\n", button, x, y);
  }
  else if (state == GLUT_UP) {
    printf("Mouse %d is released at (%d, %d)\n", button, x, y);
  }
}

void My_Keyboard(unsigned char key, int x, int y) {
    printf("Key %c is pressed at (%d, %d)\n", key, x, y);
}

void My_SpecialKeys(int key, int x, int y) {
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

void My_Menu(int id) {
  switch (id) {
  case 999:
    if (!timer_enabled) {
      timer_enabled = true;
      glutTimerFunc(timer_speed, My_Timer, 0);
    }
    break;
    // TODO:
    // New 2 cases for switch shader
  case 1:
    glUseProgram(program);
    break;

  case 2:
    glutChangeToMenuEntry(2, "new label", 2);
    break;

  case 99:
    timer_enabled = false;
    break;

  case 3:
    exit(0);
    break;

  default:
    break;
  }
}

int main(int argc, char* argv[]) {
#ifdef __APPLE__
  // Change working directory to source code path
  chdir(__FILEPATH__("/../Assets/"));
#endif
  // Initialize GLUT and GLEW, then create a window.
  ////////////////////
  glutInit(&argc, argv);
#ifdef _MSC_VER
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#else
  glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE |
    GLUT_DEPTH);
#endif
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(600, 600);
  glutCreateWindow("Assignment 1");  // You cannot use OpenGL functions before
                                     // this line; The OpenGL context must be
                                     // created first by glutCreateWindow()!
#ifdef _MSC_VER
  glewInit();
#endif
  // dumpInfo();
  My_Init();

  // Create a menu and bind it to mouse right button.
  ////////////////////////////
  int menu_main = glutCreateMenu(My_Menu);
  //int menu_timer = glutCreateMenu(My_Menu);

  glutSetMenu(menu_main);
  //glutAddSubMenu("Timer", menu_timer);

  // TODO:
  // New the menu selection to select which shader you want
  glutAddMenuEntry("Sin shader", 1);
  glutAddMenuEntry("Brick shader", 2);
  glutAddMenuEntry("Exit", 3);

  //glutSetMenu(menu_timer);
  //glutAddMenuEntry("Start", MENU_TIMER_START);
  //glutAddMenuEntry("Stop", MENU_TIMER_STOP);

  //glutSetMenu(menu_main);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
  ////////////////////////////

  // Register GLUT callback functions.
  ///////////////////////////////
  glutDisplayFunc(My_Display);
  glutReshapeFunc(My_Reshape);
  glutMouseFunc(My_Mouse);
  glutKeyboardFunc(My_Keyboard);
  glutSpecialFunc(My_SpecialKeys);
  glutTimerFunc(timer_speed, My_Timer, 0);
  ///////////////////////////////

  // Enter main event loop.
  //////////////
  glutMainLoop();
  //////////////
  return 0;
}
