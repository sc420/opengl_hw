#pragma warning(push, 0)
#include "assignment/common.hpp"
#pragma warning(pop)
#include "assignment/buffer.hpp"
#include "assignment/program.hpp"
#include "assignment/shader.hpp"
#include "assignment/uniform.hpp"
#include "assignment/vertex_spec.hpp"

GLubyte timer_cnt = 0;
bool timer_enabled = true;
unsigned int timer_speed = 16;

/*******************************************************************************
 * Constants
 ******************************************************************************/

constexpr auto MOUSE_SENSITIVITY = 1.0f;
constexpr auto MOUSE_DIV_FACTOR = 300.0f;

/*******************************************************************************
 * Transformation
 ******************************************************************************/

struct Mvp {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

Mvp mvp;

/*******************************************************************************
 * Managers
 ******************************************************************************/

BufferManager buffer_manager;
ProgramManager program_manager;
ShaderManager shader_manager;
UniformManager uniform_manager;
VertexSpecManager vertex_spec_manager;

void InitGLUT(int argc, char* argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(600, 600);
  glutCreateWindow("Assignment 1");
}

void InitGLEW() {
  glewInit();
  // DumpGLInfo();
}

void ConfigGL() {
  EnableCatchingError();

  // Configure GL
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  // Register managers
  program_manager.RegisterShaderManager(shader_manager);
  uniform_manager.RegisterProgramManager(program_manager);
  uniform_manager.RegisterBufferManager(buffer_manager);
  vertex_spec_manager.RegisterBufferManager(buffer_manager);

  // Create shaders
  shader_manager.CreateShader("vertex_shader", GL_VERTEX_SHADER,
                              "vertex.vs.glsl");
  shader_manager.CreateShader("fragment_shader", GL_FRAGMENT_SHADER,
                              "fragment.fs.glsl");

  // Create programs
  program_manager.CreateProgram("program");
  program_manager.AttachShader("program", "vertex_shader");
  program_manager.AttachShader("program", "fragment_shader");
  program_manager.LinkProgram("program");
  program_manager.UseProgram("program");

  // Create buffers
  buffer_manager.GenBuffer("va_buffer");
  buffer_manager.GenBuffer("mvp_buffer");

  // Create vertex arrays
  vertex_spec_manager.GenVertexArray("va");

  // Bind buffer targets to be repeatedly used later
  buffer_manager.BindBuffer("va_buffer", GL_ARRAY_BUFFER);
  buffer_manager.BindBuffer("mvp_buffer", GL_UNIFORM_BUFFER);

  // Initialize buffers
  buffer_manager.InitBuffer("va_buffer", GL_ARRAY_BUFFER, 18 * sizeof(float), NULL,
                            GL_STATIC_DRAW);
  buffer_manager.InitBuffer("mvp_buffer", GL_UNIFORM_BUFFER,
                            3 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);

  // Update buffers
  buffer_manager.UpdateBuffer("mvp_buffer", GL_UNIFORM_BUFFER, 0, sizeof(Mvp), &mvp);

  // Bind uniform blocks to buffers
  uniform_manager.AssignUniformBlockToBindingPoint("program", "mvp", 0);
  uniform_manager.BindBufferBaseToBindingPoint("mvp_buffer", 0);

  // Bind vertex arrays to buffers
  vertex_spec_manager.SpecifyVertexArrayOrg("va", 0, 3, GL_FLOAT, GL_FALSE, 0);
  vertex_spec_manager.SpecifyVertexArrayOrg("va", 1, 3, GL_FLOAT, GL_FALSE,
                                            3 * 3 * sizeof(float));
  vertex_spec_manager.AssocVertexAttribToBindingPoint("va", 0, 0);
  vertex_spec_manager.AssocVertexAttribToBindingPoint("va", 1, 1);
  vertex_spec_manager.BindBufferToBindingPoint("va", "va_buffer", 0, 0,
                                               3 * sizeof(float));
  vertex_spec_manager.BindBufferToBindingPoint("va", "va_buffer", 1, 0,
                                               3 * sizeof(float));
}

void My_Display() {
  /* Clear frame buffers */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* Use the program */
  program_manager.UseProgram("program");

  /* Update buffers */
  // TODO: Generate sphere by octahedron
  float f_timer_cnt = ((5 * timer_cnt) % 255) / 255.0f;
  float data[18] = {-0.5f,       -0.4f, 0.0f, 0.5f,        -0.4f, 0.0f,
                    0.0f,        0.6f,  0.0f, f_timer_cnt, 0.0f,  0.0f,
                    f_timer_cnt, 0.0f,  0.0f, f_timer_cnt, 0.0f,  0.0f};

  buffer_manager.BindBuffer("va_buffer");
  buffer_manager.UpdateBuffer("va_buffer", GL_ARRAY_BUFFER, 0, 18 * sizeof(float),
                              data);

  buffer_manager.BindBuffer("mvp_buffer");
  buffer_manager.UpdateBuffer("mvp_buffer");

  /* Draw vertex arrays */
  vertex_spec_manager.BindVertexArray("va");
  glDrawArrays(GL_TRIANGLES, 0, 3);

  /* Swap frame buffers in double buffer mode */
  glutSwapBuffers();
}

void My_Reshape(int width, int height) {
  float ratio = static_cast<float>(width) / static_cast<float>(height);

  glViewport(0, 0, width, height);

  mvp.proj = glm::ortho(-1.0f * ratio, 1.0f * ratio, -1.0f, 1.0f);
  mvp.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));
  mvp.model = glm::mat4();
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
  } else if (state == GLUT_UP) {
    printf("Mouse %d is released at (%d, %d)\n", button, x, y);
  }
}

void My_Keyboard(unsigned char key, int x, int y) {
  printf("Key %c is pressed at (%d, %d)\n", key, x, y);
  switch (key) {
    case 27:  // Escape
      exit(0);
      break;
    default:
      break;
  }
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
    case 1:
      break;

    case 2:
      glutChangeToMenuEntry(2, "new label", 2);
      break;

    case 3:
      exit(0);
      break;

    case 99:
      timer_enabled = false;
      break;

    case 999:
      if (!timer_enabled) {
        timer_enabled = true;
        glutTimerFunc(timer_speed, My_Timer, 0);
      }
      break;

    default:
      break;
  }
}

void RegisterGLUTCallbacks() {
  glutDisplayFunc(My_Display);
  glutReshapeFunc(My_Reshape);
  glutMouseFunc(My_Mouse);
  glutKeyboardFunc(My_Keyboard);
  glutSpecialFunc(My_SpecialKeys);
  glutTimerFunc(timer_speed, My_Timer, 0);
}

void CreateGLUTMenus() {
  int menu_main = glutCreateMenu(My_Menu);

  glutSetMenu(menu_main);

  glutAddMenuEntry("Sin shader", 1);
  glutAddMenuEntry("Brick shader", 2);
  glutAddMenuEntry("Exit", 3);

  glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void EnterGLUTLoop() { glutMainLoop(); }

int main(int argc, char* argv[]) {
  InitGLUT(argc, argv);
  InitGLEW();
  ConfigGL();
  RegisterGLUTCallbacks();
  CreateGLUTMenus();
  EnterGLUTLoop();
  return 0;
}
