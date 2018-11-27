/**
 * Uniform Manager
 *
 * References:
 * https://github.com/progschj/OpenGL-Examples/blob/master/06instancing3_uniform_buffer.cpp
 */
#pragma once

#include "as/common.hpp"
#include "as/gl/buffer_manager.hpp"
#include "as/gl/program_manager.hpp"

namespace as {
class UniformManager {
 public:
  UniformManager();

  void RegisterProgramManager(const ProgramManager &program_manager);

  void RegisterBufferManager(const BufferManager &buffer_manager);

  void SetUniform1Float(const std::string &program_name,
                        const std::string &var_name, const GLfloat v0);

  void SetUniform1Int(const std::string &program_name,
                      const std::string &var_name, const GLint v0);

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

  GLint GetUniformVarHdlr(const std::string &program_name,
                          const std::string &block_name);

  GLuint GetUniformBlockHdlr(const std::string &program_name,
                             const std::string &block_name);

 private:
  const ProgramManager *program_manager_;

  const BufferManager *buffer_manager_;

  std::map<std::string, GLuint> program_hdlrs_;

  std::map<std::string, GLuint> buffer_hdlrs_;

  std::map<std::string, std::map<std::string, GLint>> var_hdlrs_;

  std::map<std::string, std::map<std::string, GLuint>> block_hdlrs_;

  std::map<std::string, std::map<std::string, GLuint>> binding_points_;
};
}  // namespace as
