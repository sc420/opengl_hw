#pragma once

#include "as/common.hpp"
#include "as/gl/index_manager.hpp"

namespace as {
class TextureManager {
 public:
  struct BindTexturePrevParams {
    GLenum target;
    GLuint unit_idx;
  };

  struct UpdateTexture2DPrevParams {
    GLenum target;
    GLint mipmap_level;
    GLint x_ofs;
    GLint y_ofs;
    GLsizei width;
    GLsizei height;
    GLenum fmt;
    GLenum type;
    const GLvoid *data;
  };

  TextureManager();

  ~TextureManager();

  /* Initializations */

  void Init();

  /* Generations */

  void GenTexture(const std::string &tex_name);

  /* Bindings */

  void BindTexture(const std::string &tex_name, const GLenum target,
                   const GLuint unit_idx);

  void BindTexture(const std::string &tex_name, const GLenum target,
                   const std::string &unit_name);

  void BindTexture(const std::string &tex_name, const GLenum target);

  void BindTexture(const std::string &tex_name);

  void BindDefaultTexture(const GLenum target, const GLuint unit_idx);

  /* Memory initializations */

  void InitTexture2D(const std::string &tex_name, const GLenum target,
                     const GLsizei num_mipmap_level, const GLenum internal_fmt,
                     const GLsizei width, const GLsizei height);

  /* Memory updaters */

  void UpdateTexture2D(const std::string &tex_name, const GLenum target,
                       const GLint mipmap_level, const GLint x_ofs,
                       const GLint y_ofs, const GLsizei width,
                       const GLsizei height, const GLenum fmt,
                       const GLenum type, const GLvoid *data);

  void UpdateCubeMapTexture2D(const std::string &tex_name, const GLenum target,
                              const GLint mipmap_level, const GLint x_ofs,
                              const GLint y_ofs, const GLsizei width,
                              const GLsizei height, const GLenum fmt,
                              const GLenum type, const GLvoid *data);

  void UpdateTexture2D(const std::string &tex_name, const GLenum target);

  void UpdateCubeMapTexture2D(const std::string &tex_name, const GLenum target);

  /* Mipmap generations */

  void GenMipmap(const std::string &tex_name, const GLenum target);

  /* Texture parameters setters */

  void SetTextureParamFloat(const std::string &tex_name, const GLenum target,
                            const GLenum pname, const GLfloat param);

  void SetTextureParamInt(const std::string &tex_name, const GLenum target,
                          const GLenum pname, const GLint param);

  void SetTextureParamFloatVector(const std::string &tex_name,
                                  const GLenum target, const GLenum pname,
                                  const GLfloat *params);

  void SetTextureParamIntVector(const std::string &tex_name,
                                const GLenum target, const GLenum pname,
                                const GLint *params);

  /* Unbindings */

  void UnbindTexture(const std::string &tex_name, const GLenum target);

  void UnbindTexture(const std::string &tex_name);

  /* Deletions */

  void DeleteTexture(const std::string &tex_name);

  /* Handler getters */

  GLuint GetTextureHdlr(const std::string &tex_name) const;

  /* Status checkings */

  bool HasTexture(const std::string &tex_name) const;

  /* Unit index getters */

  GLuint GetUnitIdx(const std::string &tex_name, const GLenum target) const;

  GLuint GetUnitIdx(const std::string &tex_name) const;

 private:
  std::map<std::string, GLuint> hdlrs_;

  IndexManager<std::tuple<std::string, GLenum>, std::string, GLuint>
      index_manager_;

  std::map<std::string, BindTexturePrevParams> bind_texture_prev_params_;

  std::map<std::string, UpdateTexture2DPrevParams>
      update_texture_2d_prev_params_;

  /* Previous paramter getters */

  const BindTexturePrevParams &GetBindTexturePrevParams(
      const std::string &tex_name) const;

  const UpdateTexture2DPrevParams &GetUpdateTexture2DPrevParams(
      const std::string &tex_name) const;

  /* Initialization */

  void InitLimits();
};
}  // namespace as
