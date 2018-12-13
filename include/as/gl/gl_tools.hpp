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

void ClearDepthBuffer();

/* GL Manager Container */

class GLManagers {
 public:
  void RegisterManagers(BufferManager &buffer_manager,
                        FramebufferManager &framebuffer_manager,
                        ProgramManager &program_manager,
                        ShaderManager &shader_manager,
                        TextureManager &texture_manager,
                        UniformManager &uniform_manager,
                        VertexSpecManager &vertex_spec_manager);

  void Init();

  BufferManager &GetBufferManager() const;

  FramebufferManager &GetFramebufferManager() const;

  ProgramManager &GetProgramManager() const;

  ShaderManager &GetShaderManager() const;

  TextureManager &GetTextureManager() const;

  UniformManager &GetUniformManager() const;

  VertexSpecManager &GetVertexSpecManager() const;

 private:
  BufferManager *buffer_manager_;
  FramebufferManager *framebuffer_manager_;
  ProgramManager *program_manager_;
  ShaderManager *shader_manager_;
  TextureManager *texture_manager_;
  UniformManager *uniform_manager_;
  VertexSpecManager *vertex_spec_manager_;
};

};  // namespace as
