#pragma once

/*******************************************************************************
 * Include GL Managers
 ******************************************************************************/

#include "as/gl/buffer_manager.hpp"
#include "as/gl/framebuffer_manager.hpp"
#include "as/gl/program_manager.hpp"
#include "as/gl/shader_manager.hpp"
#include "as/gl/texture_manager.hpp"
#include "as/gl/ui_manager.hpp"
#include "as/gl/uniform_manager.hpp"
#include "as/gl/vertex_spec_manager.hpp"

/*******************************************************************************
 * Declarations
 ******************************************************************************/

namespace as {
/* Tools */

void InitGLEW();

void PrintGLContextInfo(const bool print_entensions = false,
                        const bool print_supported_glsl_versions = false);

void EnableCatchingGLError(const bool stop_when_error = false);

/* Window Position and Size */

void SetGLWindowInitCenterPos(const glm::ivec2 &window_pos,
                              const glm::ivec2 &window_size);

void SetGLWindowInitRelativeCenterPos(const glm::vec2 &relative_window_pos,
                                      const glm::ivec2 &window_size);

void SetGLWindowInitSize(const glm::ivec2 &window_size);

bool LimitGLWindowSize(const glm::ivec2 &window_size,
                       const glm::ivec2 &min_window_size = glm::ivec2(0),
                       const glm::ivec2 &max_window_size = glm::ivec2(INT_MAX));

/* Drawing Methods */

void ClearColorBuffer();

void ClearStencilBuffer();

void ClearDepthBuffer();

/* GL Manager Container */

class GLManagers {
 public:
  GLManagers();

  BufferManager &GetBufferManager();

  FramebufferManager &GetFramebufferManager();

  ProgramManager &GetProgramManager();

  ShaderManager &GetShaderManager();

  TextureManager &GetTextureManager();

  UiManager &GetUiManager();

  UniformManager &GetUniformManager();

  VertexSpecManager &GetVertexSpecManager();

 private:
  BufferManager buffer_manager_;
  FramebufferManager framebuffer_manager_;
  ProgramManager program_manager_;
  ShaderManager shader_manager_;
  TextureManager texture_manager_;
  UiManager ui_manager_;
  UniformManager uniform_manager_;
  VertexSpecManager vertex_spec_manager_;
};

};  // namespace as
