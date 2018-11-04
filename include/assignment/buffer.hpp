#pragma once

#include <map>
#include <string>

#include "assignment/common.hpp"

class BufferManager {
public:
  
  ~BufferManager();

  void GenBuffer(const std::string &name);

  void BindBuffer(const std::string &name, const GLenum target);

  void InitBuffer(const std::string &name, const GLenum target, const GLsizeiptr size, const GLvoid*data, const GLenum usage);

  void UpdateBuffer(const std::string &name, const GLenum target, const GLintptr ofs, const GLsizeiptr size, const GLvoid *data);

  void DeleteBuffer(const std::string &name);

  GLuint GetBufferHdlr(const std::string &name);

private:

  std::map<std::string, GLuint> hdlrs_;
};
