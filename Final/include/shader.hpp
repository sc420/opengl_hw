#pragma once

#include "as/gl/gl_tools.hpp"
#include "as/model/converter.hpp"
#include "as/model/model.hpp"

namespace shader {
enum class ShaderTypes { kVertex, kFragment };

class Shader {
 public:
  Shader();

  /* GL Initializations */

  void RegisterGLManagers(as::GLManagers &gl_managers);

  /* GL Drawing Methods */

  virtual void UseProgram() const;

  void UseDefaultFramebuffer() const;

  /* Name Management */

  virtual std::string GetId() const = 0;

  virtual std::string GetProgramName() const;

 protected:
  as::GLManagers *gl_managers_;

  /* GL Initializations */

  void CreateShaders();

  void CreatePrograms();

  template <class T>
  void LinkDataToUniformBlock(const std::string &buffer_name,
                              const std::string &uniform_block_name,
                              const T &buffer_data);

  template <class T>
  void InitUniformBuffer(const std::string &buffer_name, const T &buffer_data);

  void InitVertexArray(const std::string &group_name, const as::Model &model);

  /* GL Drawing Methods */

  virtual void UseMesh(const std::string &group_name,
                       const size_t mesh_idx) const;

  /* Name Management */

  std::string GetMeshVertexArrayName(const std::string &group_name,
                                     const size_t mesh_idx) const;

  std::string GetMeshVertexArrayBufferName(const std::string &group_name,
                                           const size_t mesh_idx) const;

  std::string GetMeshVertexArrayIdxsBufferName(const std::string &group_name,
                                               const size_t mesh_idx) const;

 private:
  /* Path Management */

  std::string GetShaderPath(const ShaderTypes &shader_type) const;
};

template <class T>
inline void Shader::LinkDataToUniformBlock(
    const std::string &buffer_name, const std::string &uniform_block_name,
    const T &buffer_data) {
  // Get managers
  as::UniformManager &uniform_manager = gl_managers_->GetUniformManager();
  // Get names
  const std::string program_name = GetProgramName();
  // Initialize uniform buffer
  InitUniformBuffer(buffer_name, buffer_data);
  // Bind the uniform block to the binding point
  uniform_manager.AssignUniformBlockToBindingPoint(
      program_name, uniform_block_name, buffer_name);
  // Bind the buffer to the binding point
  uniform_manager.BindBufferBaseToBindingPoint(buffer_name, buffer_name);
}

template <class T>
inline void Shader::InitUniformBuffer(const std::string &buffer_name,
                                      const T &buffer_data) {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  buffer_manager.GenBuffer(buffer_name);
  buffer_manager.InitBuffer(buffer_name, GL_UNIFORM_BUFFER, sizeof(T), NULL,
                            GL_STATIC_DRAW);
  buffer_manager.UpdateBuffer(buffer_name, GL_UNIFORM_BUFFER, 0, sizeof(T),
                              &buffer_data);
}

}  // namespace shader
