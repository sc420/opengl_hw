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

  void Init();

  void RegisterProgramManager(const ProgramManager &program_manager);

  void RegisterBufferManager(const BufferManager &buffer_manager);

  void SetUniform1Float(const std::string &program_name,
                        const std::string &var_name, const GLfloat v0);

  void SetUniform1Int(const std::string &program_name,
                      const std::string &var_name, const GLint v0);

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

  GLint GetUniformVarHdlr(const std::string &program_name,
                          const std::string &block_name);

  GLuint GetUniformBlockHdlr(const std::string &program_name,
                             const std::string &block_name);

 private:
  GLuint max_uniform_buffer_bindings_;

  const ProgramManager *program_manager_;

  const BufferManager *buffer_manager_;

  std::map<std::string, std::map<std::string, GLint>> var_hdlrs_;

  std::map<std::string, std::map<std::string, GLuint>> block_hdlrs_;

  std::set<GLuint> used_binding_idxs_;

  std::map<std::string, GLuint> binding_name_to_idxs_;

  std::map<std::tuple<std::string, std::string>, GLuint>
      uniform_side_to_binding_idx_;

  std::map<std::string, GLuint> buffer_side_to_binding_idx_;

  std::map<GLuint, std::set<std::tuple<std::string, std::string>>>
      binding_idx_to_used_uniform_sides_;

  std::map<GLuint, std::set<std::string>> binding_idx_to_used_buffer_sides_;

  void GetLimits();

  void InitUsedBindingIdxs();

  void CheckBindingIdx(const GLuint binding_idx) const;

  GLuint GetUnusedBindingIdx();

  void MarkBindingIdxUsedByUniformSide(const std::string &program_name,
                                       const std::string &block_name,
                                       const GLuint binding_idx);

  void MarkBindingIdxUsedByBufferSide(const std::string &buffer_name,
                                      const GLuint binding_idx);

  void UnmarkBindingIdxUsedByUniformSide(const std::string &program_name,
                                         const std::string &block_name);

  void UnmarkBindingIdxUsedByBufferSide(const std::string &buffer_name);
};
}  // namespace as
