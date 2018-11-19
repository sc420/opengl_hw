#pragma once

/*******************************************************************************
 * Link .lib Files
 ******************************************************************************/

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freeglut.lib")

/*******************************************************************************
 * Include Standard Libraries
 ******************************************************************************/

#include <algorithm>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

/*******************************************************************************
* Include GL Libraries
******************************************************************************/

// OpenGL loading library
#include <glew/glew.h>

// Crossplatform windowing and keyboard/mouse handler
#include <freeglut/freeglut.h>

// Cross-platform C++ mathematics library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

/*******************************************************************************
* Declarations
******************************************************************************/

namespace as {

typedef struct _texture_data {
  _texture_data() : width(0), height(0), data(0) {}
  int width;
  int height;
  unsigned char* data;
} texture_data;

texture_data load_png(const char* path);

void DumpGLInfo(void);

void EnableCatchingGLError();
}  // namespace as
