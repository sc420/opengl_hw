#pragma once

#include "as/common.hpp"
#include "as/gl/shader.hpp"

namespace as {

class ProgramManager {
 public:
  ProgramManager();

  ~ProgramManager();

  void RegisterShaderManager(const ShaderManager &shader_manager);

  void CreateProgram(const std::string &program_name);

  void AttachShader(const std::string &program_name,
                    const std::string &shader_name) const;

  void LinkProgram(const std::string &program_name) const;

  void UseProgram(const std::string &program_name) const;

  void DeleteProgram(const std::string &program_name) const;

  GLuint GetProgramHdlr(const std::string &program_name) const;

 private:
  const ShaderManager *shader_manager_;

  std::map<std::string, GLuint> hdlrs_;

  void CheckProgramLinkingStatus(const GLuint program_hdlr) const;
};

}  // namespace as
