/**
 * Uniform Manager
 *
 * References:
 * https://github.com/progschj/OpenGL-Examples/blob/master/06instancing3_uniform_buffer.cpp
 */
#pragma once

#include <map>
#include <string>

#include "assignment/common.hpp"

class UniformManager {
 public:
  void RegisterProgram(const GLuint program_hdlr, const std::string &name);

  void RegisterBuffer(const GLuint buffer_hdlr, const std::string &name);

  void AssignUniformBlockToBindingPoint(const std::string &program_name,
                          const std::string &block_name, const GLuint bind_idx);

  void BindBufferToBindingPoint(const GLuint bind_idx,
                                const std::string &buffer_name);

  void BindBufferToBindingPoint(const GLuint bind_idx,
                                const std::string &buffer_name,
                                const GLintptr offset, const GLsizeiptr size);

  GLuint GetUniformBlockBindingPoint(const std::string &program_name,
    const std::string &block_name);

 private:
  std::map<std::string, GLuint> program_hdlrs_;

  std::map<std::string, GLuint> buffer_hdlrs_;

  std::map<std::string, std::map<std::string, GLuint>> block_idxs_;

  std::map<std::string, std::map<std::string, GLuint>> binding_points_;

  GLuint get_program_hdlr(const std::string &name);

  GLuint get_buffer_hdlr(const std::string &name);
};
