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
  // Save the parameters
  BindBufferPrevParams prev_params  = { target };
  bind_buffer_prev_params_[name] = prev_params;
}

void BufferManager::BindBuffer(const std::string & name)
{
  const BindBufferPrevParams &prev_params = GetBindBufferPrevParams(name);
  BindBuffer(name, prev_params.target);
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
  // Save the parameters
  UpdateBufferPrevParams prev_params = { target, ofs, size, data };
  update_buffer_prev_params_[name] = prev_params;
}

void BufferManager::UpdateBuffer(const std::string & name)
{
  const UpdateBufferPrevParams &prev_params = GetUpdateBufferPrevParams(name);
  UpdateBuffer(name, prev_params.target, prev_params.ofs, prev_params.size, prev_params.data);
}

//TODO: Remember to delete associated maps in all classes
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

//TODO: Change to lowercase for all accessors
const BufferManager::BindBufferPrevParams &BufferManager::GetBindBufferPrevParams(const std::string & name)
{
  if (bind_buffer_prev_params_.count(name) == 0) {
    throw std::runtime_error("Could not find the previous parameters for buffer '" + name + "'");
  }
  return bind_buffer_prev_params_.at(name);
}

const BufferManager::UpdateBufferPrevParams & BufferManager::GetUpdateBufferPrevParams(const std::string & name)
{
  if (update_buffer_prev_params_.count(name) == 0) {
    throw std::runtime_error("Could not find the previous parameters for buffer '" + name + "'");
  }
  return update_buffer_prev_params_.at(name);
}
