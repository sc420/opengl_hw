#pragma once

/*******************************************************************************
 * Include MSVC Libraries
 ******************************************************************************/

// For disabling code analysis warnings for third-party libraries
#include <codeanalysis\warnings.h>

/*******************************************************************************
 * Include Standard Libraries
 ******************************************************************************/

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

/*******************************************************************************
 * Include GL Libraries
 ******************************************************************************/

#pragma warning(push)
#pragma warning(disable : ALL_CODE_ANALYSIS_WARNINGS)

// Cross-platform 3D model import library
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

// Loading library
#include <glew/glew.h>

// Cross-platform C++ mathematics library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

// Cross-platform windowing and keyboard/mouse handler
#include <freeglut/freeglut.h>

#pragma warning(pop)

/*******************************************************************************
 * Link .lib Files
 ******************************************************************************/

#pragma comment(lib, "assimp.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freeglut.lib")

/*******************************************************************************
 * Declarations
 ******************************************************************************/

namespace as {
void InitGLEW();

void PrintGLContextInfo(const bool print_entensions = false,
                        const bool print_supported_glsl_versions = false);

void EnableCatchingGLError(const bool stop_when_error = false);
}  // namespace as
