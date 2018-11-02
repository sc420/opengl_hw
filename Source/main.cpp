#pragma warning(push, 0)
#include "../Include/Common.h"
#pragma warning(pop)
#include "../Include/my_common.h"

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

GLuint program_sin;
GLuint program_brick;

void My_Init()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint fragmentBrickShader = glCreateShader(GL_FRAGMENT_SHADER);
	GLchar* vertexShaderSource = LoadShaderSource("vertex.vs.glsl");
	GLchar* fragmentShaderSource = LoadShaderSource("fragment_sin.fs.glsl");
	GLchar* fragmentBrickShaderSource = LoadShaderSource("fragment_brick.fs.glsl");
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glShaderSource(fragmentBrickShader, 1, &fragmentBrickShaderSource, NULL);
	FreeShaderSource(vertexShaderSource);
	FreeShaderSource(fragmentShaderSource);
	FreeShaderSource(fragmentBrickShaderSource);

	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);
	glCompileShader(fragmentBrickShader);
	shaderLog(vertexShader);
	shaderLog(fragmentShader);
	shaderLog(fragmentBrickShader);

	program_sin = glCreateProgram();
	glAttachShader(program_sin, vertexShader);
	glAttachShader(program_sin, fragmentShader);
	glLinkProgram(program_sin);
	glUseProgram(program_sin);
	um4mvp = glGetUniformLocation(program_sin, "um4mvp");

	program_brick = glCreateProgram();
	glAttachShader(program_brick, vertexShader);
	glAttachShader(program_brick, fragmentBrickShader);
	glLinkProgram(program_brick);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
}

// GLUT callback. Called to draw the scene.
void My_Display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float f_timer_cnt = timer_cnt / 255.0f;
	float data[18] = {
		-0.5f, -0.4f, 0.0f,
		0.5f, -0.4f, 0.0f,
		0.0f,  0.6f, 0.0f,
		f_timer_cnt, 0.0f, 1.0f - f_timer_cnt,
		1.0f, f_timer_cnt, 1.0f - f_timer_cnt,
		1.0f - f_timer_cnt, 0.0, f_timer_cnt
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float) * 9));
	glUniformMatrix4fv(um4mvp, 1, GL_FALSE, glm::value_ptr(mvp));
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glutSwapBuffers();
}

void My_Reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	float viewportAspect = (float)width / (float)height;
	mvp = glm::ortho(-1 * viewportAspect, 1 * viewportAspect, -1.0f, 1.0f);
	mvp = mvp * glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

void My_Timer(int val)
{
	timer_cnt++;
	glutPostRedisplay();
	if (timer_enabled)
	{
		glutTimerFunc(timer_speed, My_Timer, val);
	}
}

void My_Mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		printf("Mouse %d is pressed at (%d, %d)\n", button, x, y);
	}
	else if (state == GLUT_UP)
	{
		printf("Mouse %d is released at (%d, %d)\n", button, x, y);
	}
}

void My_Keyboard(unsigned char key, int x, int y)
{
	printf("Key %c is pressed at (%d, %d)\n", key, x, y);
}

void My_SpecialKeys(int key, int x, int y)
{
	switch (key)
	{
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

void My_Menu(int id)
{
	switch (id)
	{
	case MENU_TIMER_START:
		if (!timer_enabled)
		{
			timer_enabled = true;
			glutTimerFunc(timer_speed, My_Timer, 0);
		}
		break;
		//TODO:
		//New 2 cases for switch shader
	case MENU_SHADER_SIN:
		glUseProgram(program_sin);
		break;

	case MENU_SHADER_BRICK:
		glUseProgram(program_brick);
		break;

	case MENU_TIMER_STOP:
		timer_enabled = false;
		break;

	case MENU_EXIT:
		exit(0);
		break;

	default:
		break;
	}
}

int main(int argc, char *argv[])
{
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
	glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(600, 600);
	glutCreateWindow("Assignment 1"); // You cannot use OpenGL functions before this line; The OpenGL context must be created first by glutCreateWindow()!
#ifdef _MSC_VER
	glewInit();
#endif
	//dumpInfo();
	My_Init();

	// Create a menu and bind it to mouse right button.
	////////////////////////////
	int menu_main = glutCreateMenu(My_Menu);
	int menu_timer = glutCreateMenu(My_Menu);

	glutSetMenu(menu_main);
	glutAddSubMenu("Timer", menu_timer);

	//TODO:
	//New the menu selection to select which shader you want
	glutAddMenuEntry("Sin shader", MENU_SHADER_SIN);
	glutAddMenuEntry("Brick shader", MENU_SHADER_BRICK);

	glutAddMenuEntry("Exit", MENU_EXIT);

	glutSetMenu(menu_timer);
	glutAddMenuEntry("Start", MENU_TIMER_START);
	glutAddMenuEntry("Stop", MENU_TIMER_STOP);

	glutSetMenu(menu_main);
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
