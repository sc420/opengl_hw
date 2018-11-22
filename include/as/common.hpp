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

// Cross-platform windowing and keyboard/mouse handler
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

void PrintGLContextInfo(const bool print_entensions = false,
                        const bool print_supported_glsl_versions = false);

void EnableCatchingGLError();
}  // namespace as
