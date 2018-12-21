/**
 * Uniform Manager
 *
 * References:
 * https://github.com/progschj/OpenGL-Examples/blob/master/06instancing3_uniform_buffer.cpp
 */
#pragma once

#include "as/common.hpp"
#include "as/gl/buffer_manager.hpp"
#include "as/gl/index_manager.hpp"
#include "as/gl/program_manager.hpp"

namespace as {
class UniformManager {
 public:
  UniformManager();

  /* Initializations */

  void Init();

  /* Manager registrations */

  void RegisterProgramManager(const ProgramManager &program_manager);

  void RegisterBufferManager(const BufferManager &buffer_manager);

  /* Setting value methods */

  void SetUniform1Float(const std::string &program_name,
                        const std::string &var_name, const GLfloat v0);

  void SetUniform1Int(const std::string &program_name,
                      const std::string &var_name, const GLint v0);

  /* Binding connections */

  void AssignUniformBlockToBindingPoint(const std::string &program_name,
                                        const std::string &block_name,
                                        const GLuint binding_idx);

  void AssignUniformBlockToBindingPoint(const std::string &program_name,
                                        const std::string &block_name,
                                        const std::string &binding_name);

  void BindBufferBaseToBindingPoint(const std::string &buffer_name,
                                    const GLuint binding_idx);

  void BindBufferBaseToBindingPoint(const std::string &buffer_name,
                                    const std::string &binding_name);

  void UnassignUniformBlock(const std::string &program_name,
                            const std::string &block_name);

  void UnbindBufferBase(const std::string &buffer_name);

  /* Handler getters */

  GLint GetUniformVarHdlr(const std::string &program_name,
                          const std::string &block_name);

  GLuint GetUniformBlockHdlr(const std::string &program_name,
                             const std::string &block_name);

  /* Debugging */

  /**
   * Example: GetUniformBlockMemoryOfs("scene", "ExampleBlock.model");
   * Note that "ExampleBlock" is the uniform declaration name rather than
   * instance name.
   */
  GLint GetUniformBlockMemoryOfs(const std::string &program_name,
                                 const std::string &block_member_name) const;

 private:
  const ProgramManager *program_manager_;

  const BufferManager *buffer_manager_;

  std::map<std::string, std::map<std::string, GLint>> var_hdlrs_;

  std::map<std::string, std::map<std::string, GLuint>> block_hdlrs_;

  IndexManager<std::tuple<std::string, std::string>, std::string, GLuint>
      index_manager_;

  /* Initializations */

  void InitLimits();
};
}  // namespace as
