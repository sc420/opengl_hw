#pragma once

#include <map>
#include <string>

#include "assignment/common.hpp"
#include "assignment/shader.hpp"

class ProgramManager {
 public:
  ~ProgramManager();

  void RegisterShaderManager(const ShaderManager &shader_manager);

  void CreateProgram(const std::string &name);

  void AttachShader(const std::string &program_name, const std::string &shader_name) const;

  void LinkProgram(const std::string &name) const;

  void UseProgram(const std::string &name) const;

  void DeleteProgram(const std::string &name) const;

  GLuint GetProgramHdlr(const std::string &name) const;

 private:
   const ShaderManager * shader_manager_;

  std::map<std::string, GLuint> hdlrs_;

  void CheckProgramLinkingStatus(const GLuint hdlr) const;
};
