#include "assignment/uniform.hpp"

void UniformManager::RegisterProgram(const GLuint program_hdlr,
                                     const std::string &name) {
  program_hdlrs_[name] = program_hdlr;
}

void UniformManager::RegisterBuffer(const GLuint buffer_hdlr,
                                    const std::string &name) {
  buffer_hdlrs_[name] = buffer_hdlr;
}

void UniformManager::AssignBindingPoint(const std::string &program_name,
                                        const std::string &block_name,
                                        const GLuint point) {
  GLuint block_hdlr;
  // Get program handler
  const GLuint program_hdlr = get_program_hdlr(program_name);
  // Check whether to retrieve the index of a named uniform block lazily
  bool retrieve = false;
  if (block_idxs_.count(program_name) > 0) {
    const std::map<std::string, GLuint> &block_to_idxs =
        block_idxs_.at(program_name);
    if (block_to_idxs.count(block_name) > 0) {
      block_hdlr = block_to_idxs.at(block_name);
    } else {
      retrieve = true;
    }
  } else {
    retrieve = true;
    std::map<std::string, GLuint> block_to_idxs;
    block_idxs_[program_name] = block_to_idxs;
  }
  // Retrieve the index of a named uniform block
  if (retrieve) {
    block_hdlr = glGetUniformBlockIndex(
        program_hdlr, static_cast<const GLchar *>(block_name.c_str()));
    std::map<std::string, GLuint> &block_to_idxs = block_idxs_.at(program_name);
    block_to_idxs[block_name] = block_hdlr;
  }
  // Assign the binding point
  glUniformBlockBinding(program_hdlr, block_hdlr, point);
  // Save the binding point
  if (binding_points_.count(program_name) == 0) {
    std::map<std::string, GLuint> block_to_point;
    binding_points_[program_name] = block_to_point;
  }
  std::map<std::string, GLuint> &block_to_points = binding_points_.at(program_name);
  block_to_points[block_name] = point;
}

GLuint UniformManager::GetBindingPoint(const std::string &program_name,
                                       const std::string &block_name) {
  const std::map<std::string, GLuint> &block_to_points = binding_points_.at(program_name);
  return block_to_points.at(block_name);
}

void UniformManager::BindBufferToBindingPoint(const GLuint point,
  const std::string &buffer_name) {
  // Get buffer handler
  const GLuint buffer_hdlr = get_buffer_hdlr(buffer_name);
  // Bind the buffer to the binding point
  glBindBufferBase(GL_UNIFORM_BUFFER, point, buffer_hdlr);
}

void UniformManager::BindBufferToBindingPoint(const GLuint point,
                                              const std::string &buffer_name,
                                              const GLintptr offset,
                                              const GLsizeiptr size) {
  // Get buffer handler
  const GLuint buffer_hdlr = get_buffer_hdlr(buffer_name);
  // Bind the buffer to the binding point
  glBindBufferRange(GL_UNIFORM_BUFFER, point, buffer_hdlr, offset, size);
}

GLuint UniformManager::get_program_hdlr(const std::string &name)
{
  if (program_hdlrs_.count(name) == 0) {
    throw std::runtime_error("Could not find the program name '" +
      name + "'");
  }
  return program_hdlrs_.at(name);
}

GLuint UniformManager::get_buffer_hdlr(const std::string &name)
{
  if (buffer_hdlrs_.count(name) == 0) {
    throw std::runtime_error("Could not find the buffer name '" + name +
      "'");
  }
  const GLuint buffer_hdlr = buffer_hdlrs_.at(name);
}
