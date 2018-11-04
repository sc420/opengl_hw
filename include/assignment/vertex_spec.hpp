#pragma once

#include <map>
#include <string>

#include "assignment/common.hpp"

class VertexSpecManager {
 public:
  ~VertexSpecManager();

  void GenVertexArray(const std::string &name);

  void BindVertexArray(const std::string &name);

  void SpecifyVertexArrayOrg(const std::string &name, const GLuint attrib_idx, const GLint size,
                             const GLenum type, const GLboolean normalized,
                             const GLuint relative_ofs);

  void AssocVertexAttribToBindingPoint(const std::string &va_name,
                                       const GLuint attrib_idx,
                                       const GLuint binding_idx);

  void BindBufferToBindingPoint(const std::string &va_name,
                                const GLuint binding_idx,
                                const GLuint buffer_hdlr, const GLintptr ofs,
                                const GLsizei stride);

  void DeleteVertexArray(const std::string &name);

  GLuint GetVertexArrayHdlr(const std::string &name);

  GLuint GetVertexAttribBindingPoint(const std::string &va_name,
                                     const GLuint attrib_idx);

 private:
  std::map<std::string, GLuint> hdlrs_;

  std::map<std::string, std::map<GLuint, GLuint>> binding_points_;
};
