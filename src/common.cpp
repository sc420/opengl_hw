#include "as/common.hpp"

/*******************************************************************************
 * Include Header-only Libraries
 ******************************************************************************/

#pragma warning(push)
#pragma warning(disable : ALL_CODE_ANALYSIS_WARNINGS)

// Load tinyobjloader
#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

// Load stb
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#pragma warning(pop)
