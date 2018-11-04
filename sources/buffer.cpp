#include "assignment/buffer.hpp"

BufferManager::~BufferManager()
{
  // Delete all buffer objects
  for (const auto& pair : hdlrs_) {
    glDeleteBuffers(1, &pair.second);
  }
}

void BufferManager::GenBuffer(const std::string & name)
{
  // Generate a buffer object
  GLuint hdlr;
  glGenBuffers(1, &hdlr);
  // Save the handler
  hdlrs_[name] = hdlr;
}

void BufferManager::BindBuffer(const std::string & name, const GLenum target)
{
  const GLuint hdlr = GetBufferHdlr(name);
  glBindBuffer(target, hdlr);
}

void BufferManager::InitBuffer(const std::string & name, const GLenum target, const GLsizeiptr size, const GLvoid * data, const GLenum usage)
{
  BindBuffer(name, target);
  glBufferData(target, size, data, usage);
}

void BufferManager::UpdateBuffer(const std::string & name, const GLenum target, const GLintptr ofs, const GLsizeiptr size, const GLvoid * data)
{
  BindBuffer(name, target);
  glBufferSubData(target, ofs, size, data);
}

void BufferManager::DeleteBuffer(const std::string & name)
{
  const GLuint hdlr = GetBufferHdlr(name);
  glDeleteBuffers(1, &hdlr);
}

GLuint BufferManager::GetBufferHdlr(const std::string & name)
{
  if (hdlrs_.count(name) == 0) {
    throw std::runtime_error("Could not find the buffer name '" + name + "'");
  }
  return hdlrs_.at(name);
}
