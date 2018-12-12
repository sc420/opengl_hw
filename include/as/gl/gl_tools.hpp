#pragma once

/*******************************************************************************
 * Include GL Managers
 ******************************************************************************/

#include "as/gl/buffer_manager.hpp"
#include "as/gl/framebuffer_manager.hpp"
#include "as/gl/program_manager.hpp"
#include "as/gl/shader_manager.hpp"
#include "as/gl/texture_manager.hpp"
#include "as/gl/uniform_manager.hpp"
#include "as/gl/vertex_spec_manager.hpp"

/*******************************************************************************
 * Declarations
 ******************************************************************************/

namespace as {
void InitGLEW();

void PrintGLContextInfo(const bool print_entensions = false,
                        const bool print_supported_glsl_versions = false);

void EnableCatchingGLError(const bool stop_when_error = false);

};  // namespace as
