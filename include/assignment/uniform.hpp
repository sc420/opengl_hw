#pragma once

#include <map>
#include <string>

#include "assignment/common.hpp"

class UniformManager {
public:
  void RegisterProgram(const GLuint program_hdlr, const std::string &name);

  void RegisterBuffer(const GLuint buffer_hdlr, const std::string &name);

  void AssignBindingPoint(const std::string &program_name,
    const std::string &block_name, const GLuint point);

  GLuint GetBindingPoint(const std::string &program_name,
    const std::string &block_name);

  void BindBufferToBindingPoint(const GLuint point,
    const std::string &buffer_name);

  void BindBufferToBindingPoint(const GLuint point,
    const std::string &buffer_name,
    const GLintptr offset, const GLsizeiptr size);

private:
  std::map<std::string, GLuint> program_hdlrs_;

  std::map<std::string, GLuint> buffer_hdlrs_;

  std::map<std::string, std::map<std::string, GLuint>> block_idxs_;

  std::map<std::string, std::map<std::string, GLuint>> binding_points_;

  GLuint get_program_hdlr(const std::string &name);

  GLuint get_buffer_hdlr(const std::string &name);
};
