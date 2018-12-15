#pragma once

#include "as/gl/gl_tools.hpp"
#include "as/model/converter.hpp"
#include "as/model/model.hpp"

namespace shader {
enum class ShaderTypes { kVertex, kFragment };

class Shader {
 public:
  Shader();

  /* GL initialization methods */

  void RegisterGLManagers(as::GLManagers &gl_managers);

  virtual void Init();

  /* GL drawing methods */

  virtual void UseProgram() const;

  virtual void Draw() = 0;

  /* Name management */

  virtual std::string GetId() const = 0;

  virtual std::string GetProgramName() const;

 protected:
  as::GLManagers *gl_managers_;

  /* Model handlers */

  virtual as::Model &GetModel() = 0;

  /* GL initialization methods */

  template <class T>
  void InitUniformBuffer(const std::string &buffer_name, const T &buffer_data);

  void InitVertexArray(const as::Model &model);

  /* GL drawing methods */

  virtual void UseMesh(const size_t mesh_idx) const;

  /* Name management */

  std::string GetMeshVertexArrayName(const size_t mesh_idx) const;

  std::string GetMeshVertexArrayBufferName(const size_t mesh_idx) const;

  std::string GetMeshVertexArrayIdxsBufferName(const size_t mesh_idx) const;

 private:
  /* GL initialization methods */

  void CreateShaders();

  void CreatePrograms();

  /* Path management */

  std::string GetShaderPath(const ShaderTypes &shader_type) const;
};

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
