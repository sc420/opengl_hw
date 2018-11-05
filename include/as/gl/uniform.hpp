/**
 * Uniform Manager
 *
 * References:
 * https://github.com/progschj/OpenGL-Examples/blob/master/06instancing3_uniform_buffer.cpp
 */
#pragma once

#include <map>
#include <string>

#include "as/common.hpp"
#include "as/gl/buffer.hpp"
#include "as/gl/program.hpp"

namespace as {
class UniformManager {
 public:
  UniformManager();

  void RegisterProgramManager(const ProgramManager &program_manager);

  void RegisterBufferManager(const BufferManager &buffer_manager);

  void AssignUniformBlockToBindingPoint(const std::string &program_name,
                                        const std::string &block_name,
                                        const GLuint binding_idx);

  void BindBufferBaseToBindingPoint(const std::string &buffer_name,
                                    const GLuint binding_idx) const;

  void BindBufferRangeToBindingPoint(const std::string &buffer_name,
                                     const GLuint binding_idx,
                                     const GLintptr ofs,
                                     const GLsizeiptr size) const;

  GLuint GetUniformBlockBindingPoint(const std::string &program_name,
                                     const std::string &block_name) const;

 private:
  const ProgramManager *program_manager_;

  const BufferManager *buffer_manager_;

  std::map<std::string, GLuint> program_hdlrs_;

  std::map<std::string, GLuint> buffer_hdlrs_;

  std::map<std::string, std::map<std::string, GLuint>> block_idxs_;

  std::map<std::string, std::map<std::string, GLuint>> binding_points_;
};
}  // namespace as
