#pragma once

#ifdef _MSC_VER
#include <GLEW/glew.h>

#include <FreeGLUT/freeglut.h>
#include <direct.h>
#else
#include <GLUT/glut.h>
#include <OpenGL/gl3.h>
#include <unistd.h>
#endif

#ifdef _MSC_VER
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freeglut.lib")
#endif

#define GLM_SWIZZLE
#include <GLM/glm/glm.hpp>
#include <GLM/glm/gtc/matrix_transform.hpp>
#include <GLM/glm/gtc/quaternion.hpp>
#include <GLM/glm/gtc/type_ptr.hpp>
#include <GLM/glm/gtx/quaternion.hpp>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _MSC_VER
#define __FILENAME__ \
  (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define __FILEPATH__(x)                                                       \
  ((std::string(__FILE__).substr(0, std::string(__FILE__).rfind('\\')) + (x)) \
       .c_str())
#else
#define __FILENAME__ \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define __FILEPATH__(x)                                                      \
  ((std::string(__FILE__).substr(0, std::string(__FILE__).rfind('/')) + (x)) \
       .c_str())
#endif

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
