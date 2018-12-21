#include "as/gl/buffer_manager.hpp"

as::BufferManager::~BufferManager() {
  // Delete all buffer objects
  for (const auto &pair : hdlrs_) {
    glDeleteBuffers(1, &pair.second);
  }
}

/*******************************************************************************
 * Generations
 ******************************************************************************/

void as::BufferManager::GenBuffer(const std::string &buffer_name) {
  // Generate a buffer object
  GLuint hdlr;
  glGenBuffers(1, &hdlr);
  // Save the handler
  hdlrs_[buffer_name] = hdlr;
}

/*******************************************************************************
 * Bindings
 ******************************************************************************/

void as::BufferManager::BindBuffer(const std::string &buffer_name,
                                   const GLenum target) {
  const GLuint hdlr = GetBufferHdlr(buffer_name);
  glBindBuffer(target, hdlr);
  // Save the parameters
  BindBufferPrevParams prev_params = {target};
  bind_buffer_prev_params_[buffer_name] = prev_params;
}

void as::BufferManager::BindBuffer(const std::string &buffer_name) {
  const BindBufferPrevParams &prev_params =
      GetBindBufferPrevParams(buffer_name);
  BindBuffer(buffer_name, prev_params.target);
}

/*******************************************************************************
 * Deselections
 ******************************************************************************/

void as::BufferManager::DeselectBuffer(const GLenum target) {
  glBindBuffer(target, 0);
}

/*******************************************************************************
 * Memory Initializations
 ******************************************************************************/

void as::BufferManager::InitBuffer(const std::string &buffer_name,
                                   const GLenum target, const GLsizeiptr size,
                                   const GLvoid *data, const GLenum usage) {
  BindBuffer(buffer_name, target);
  glBufferData(target, size, data, usage);
}

/*******************************************************************************
 * Memory Updaters
 ******************************************************************************/

void as::BufferManager::UpdateBuffer(const std::string &buffer_name,
                                     const GLenum target, const GLintptr ofs,
                                     const GLsizeiptr size,
                                     const GLvoid *data) {
  BindBuffer(buffer_name, target);
  glBufferSubData(target, ofs, size, data);
  // Save the parameters
  UpdateBufferPrevParams prev_params = {target, ofs, size, data};
  update_buffer_prev_params_[buffer_name] = prev_params;
}

void as::BufferManager::UpdateBuffer(const std::string &buffer_name) {
  const UpdateBufferPrevParams &prev_params =
      GetUpdateBufferPrevParams(buffer_name);
  UpdateBuffer(buffer_name, prev_params.target, prev_params.ofs,
               prev_params.size, prev_params.data);
}

/*******************************************************************************
 * Deletions
 ******************************************************************************/

void as::BufferManager::DeleteBuffer(const std::string &buffer_name) {
  const GLuint hdlr = GetBufferHdlr(buffer_name);
  glDeleteBuffers(1, &hdlr);
  // Delete previous parameters
  bind_buffer_prev_params_.erase(buffer_name);
  update_buffer_prev_params_.erase(buffer_name);
}

/*******************************************************************************
 * Handler Getters
 ******************************************************************************/

GLuint as::BufferManager::GetBufferHdlr(const std::string &buffer_name) const {
  if (hdlrs_.count(buffer_name) == 0) {
    throw std::runtime_error("Could not find the buffer name '" + buffer_name +
                             "'");
  }
  return hdlrs_.at(buffer_name);
}

/*******************************************************************************
 * Previous Parameter Getters (Private)
 ******************************************************************************/

const as::BufferManager::BindBufferPrevParams &
as::BufferManager::GetBindBufferPrevParams(
    const std::string &buffer_name) const {
  if (bind_buffer_prev_params_.count(buffer_name) == 0) {
    throw std::runtime_error(
        "Could not find the previous parameters for buffer name '" +
        buffer_name + "'");
  }
  return bind_buffer_prev_params_.at(buffer_name);
}

const as::BufferManager::UpdateBufferPrevParams &
as::BufferManager::GetUpdateBufferPrevParams(
    const std::string &buffer_name) const {
  if (update_buffer_prev_params_.count(buffer_name) == 0) {
    throw std::runtime_error(
        "Could not find the previous parameters for buffer name '" +
        buffer_name + "'");
  }
  return update_buffer_prev_params_.at(buffer_name);
}
