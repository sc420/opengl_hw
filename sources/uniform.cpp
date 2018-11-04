#include "assignment/uniform.hpp"

void UniformManager::RegisterProgram(const GLuint program_hdlr,
                                     const std::string &name) {
  program_hdlrs_[name] = program_hdlr;
}

void UniformManager::RegisterBuffer(const GLuint buffer_hdlr,
                                    const std::string &name) {
  buffer_hdlrs_[name] = buffer_hdlr;
}

void UniformManager::AssignUniformBlockToBindingPoint(
    const std::string &program_name, const std::string &block_name,
    const GLuint bind_idx) {
  // Get program handler
  const GLuint program_hdlr = get_program_hdlr(program_name);
  // Check whether to retrieve the index of a named uniform block lazily
  GLuint block_hdlr = NULL;
  if (block_idxs_.count(program_name) > 0 &&
      block_idxs_.at(program_name).count(block_name) > 0) {
    block_hdlr = block_idxs_.at(program_name).at(block_name);
  } else {
    // Retrieve the index of a named uniform block
    block_hdlr = glGetUniformBlockIndex(program_hdlr, block_name.c_str());
    // Save the block handler
    block_idxs_[program_name][block_name] = block_hdlr;
  }
  // Assign the binding point
  glUniformBlockBinding(program_hdlr, block_hdlr, bind_idx);
  // Save the binding point
  binding_points_[program_name][block_name] = bind_idx;
}

void UniformManager::BindBufferToBindingPoint(const GLuint bind_idx,
                                              const std::string &buffer_name) {
  // Get buffer handler
  const GLuint buffer_hdlr = get_buffer_hdlr(buffer_name);
  // Bind the buffer to the binding point
  glBindBufferBase(GL_UNIFORM_BUFFER, bind_idx, buffer_hdlr);
}

void UniformManager::BindBufferToBindingPoint(const GLuint bind_idx,
                                              const std::string &buffer_name,
                                              const GLintptr offset,
                                              const GLsizeiptr size) {
  // Get buffer handler
  const GLuint buffer_hdlr = get_buffer_hdlr(buffer_name);
  // Bind the buffer to the binding point
  glBindBufferRange(GL_UNIFORM_BUFFER, bind_idx, buffer_hdlr, offset, size);
}

GLuint UniformManager::GetUniformBlockBindingPoint(
    const std::string &program_name, const std::string &block_name) {
  if (binding_points_.count(program_name) == 0) {
    throw std::runtime_error("Could not find the program name '" +
                             program_name + "'");
  }
  const std::map<std::string, GLuint> &block_to_points =
      binding_points_.at(program_name);
  if (block_to_points.count(block_name) == 0) {
    throw std::runtime_error("Could not find the block name '" + block_name +
                             "'");
  }
  return block_to_points.at(block_name);
}

GLuint UniformManager::get_program_hdlr(const std::string &name) {
  if (program_hdlrs_.count(name) == 0) {
    throw std::runtime_error("Could not find the program name '" + name + "'");
  }
  return program_hdlrs_.at(name);
}

GLuint UniformManager::get_buffer_hdlr(const std::string &name) {
  if (buffer_hdlrs_.count(name) == 0) {
    throw std::runtime_error("Could not find the buffer name '" + name + "'");
  }
  return buffer_hdlrs_.at(name);
}
