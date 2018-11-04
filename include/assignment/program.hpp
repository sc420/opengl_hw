#pragma once

#include <map>
#include <string>

#include "assignment/common.hpp"

class ProgramManager {
 public:
  ~ProgramManager();

  void CreateProgram(const std::string &name);

  void AttachShader(const std::string &name, const GLuint shader_hdlr) const;

  void LinkProgram(const std::string &name) const;

  void UseProgram(const std::string &name) const;

  void DeleteProgram(const std::string &name) const;

  GLuint GetProgramHdlr(const std::string &name) const;

 private:
  std::map<std::string, GLuint> hdlrs_;

  void CheckProgramLinkingStatus(const GLuint hdlr) const;
};
