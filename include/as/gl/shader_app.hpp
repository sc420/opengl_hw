#pragma once

#include "as/gl/gl_tools.hpp"

namespace app {
enum class ShaderTypes { kVertex, kFragment };

class ShaderApp {
 public:
  /* GL initialization methods */

  void RegisterGLManagers(as::GLManagers &gl_managers);

  void Init();

  virtual void InitUniformBlocks() = 0;

  template <class T>
  void InitUniformBuffer(const std::string &buffer_name, const T &buffer_data);

  /* GL drawing methods */

  void Use() const;

  /* Name management */

  virtual std::string GetId() const = 0;

  std::string GetProgramName() const;

 protected:
  as::GLManagers *gl_managers_;

 private:
  /* GL initialization methods */

  void CreateShaders();

  void CreatePrograms();

  /* Path management */

  std::string GetShaderPath(const ShaderTypes &shader_type) const;
};

template <class T>
inline void ShaderApp::InitUniformBuffer(const std::string &buffer_name,
                                         const T &buffer_data) {
  as::BufferManager &buffer_manager = gl_managers_->GetBufferManager();
  buffer_manager.GenBuffer(buffer_name);
  buffer_manager.InitBuffer(buffer_name, GL_UNIFORM_BUFFER, sizeof(T), NULL,
                            GL_STATIC_DRAW);
  buffer_manager.UpdateBuffer(buffer_name, GL_UNIFORM_BUFFER, 0, sizeof(T),
                              &buffer_data);
}

}  // namespace app
