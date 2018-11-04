#pragma once

#include <map>
#include <string>

#include "assignment/common.hpp"

class ProgramManager {
 public:
  ~ProgramManager();

  void CreateProgram(const std::string &name);

  void AttachShader(const std::string &name, const GLuint shader_hdlr);

  void LinkProgram(const std::string &name);

  void UseProgram(const std::string &name);

  void DeleteProgram(const std::string &name);

  GLuint GetProgramHdlr(const std::string &name);

 private:
  std::map<std::string, GLuint> hdlrs_;

  void CheckProgramLinkingStatus(const GLuint hdlr);
};
