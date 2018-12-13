#include "as/gl/texture_manager.hpp"

as::TextureManager::TextureManager() {}

as::TextureManager::~TextureManager() {
  // Delete all textures
  for (const auto &pair : hdlrs_) {
    glDeleteTextures(1, &pair.second);
  }
}

void as::TextureManager::Init() { InitLimits(); }

void as::TextureManager::GenTexture(const std::string &tex_name) {
  GLuint tex_hdlr;
  glGenTextures(1, &tex_hdlr);
  hdlrs_[tex_name] = tex_hdlr;
}

void as::TextureManager::BindTexture(const std::string &tex_name,
                                     const GLenum target,
                                     const GLuint unit_idx) {
  const GLuint tex_hdlr = GetTextureHdlr(tex_name);
  // Check the unit index
  index_manager_.CheckMaxIdx(unit_idx);
  // Select the texture unit
  glActiveTexture(GL_TEXTURE0 + unit_idx);
  // Bind the texture
  glBindTexture(target, tex_hdlr);
  // Save the parameters
  BindTexturePrevParams prev_params = {target, unit_idx};
  bind_texture_prev_params_[tex_name] = prev_params;
}

void as::TextureManager::BindTexture(const std::string &tex_name,
                                     const GLenum target,
                                     const std::string &unit_name) {
  const auto index_manager_target = std::make_tuple(tex_name, target);
  const GLuint unit_idx =
      index_manager_.BindTarget1(index_manager_target, unit_name);
  // Bind with the unit index
  BindTexture(tex_name, target, unit_idx);
}

void as::TextureManager::BindTexture(const std::string &tex_name,
                                     const GLenum target) {
  const BindTexturePrevParams &prev_params = GetBindTexturePrevParams(tex_name);
  BindTexture(tex_name, target, prev_params.unit_idx);
}

void as::TextureManager::BindTexture(const std::string &tex_name) {
  const BindTexturePrevParams &prev_params = GetBindTexturePrevParams(tex_name);
  BindTexture(tex_name, prev_params.target, prev_params.unit_idx);
}

void as::TextureManager::InitTexture2D(const std::string &tex_name,
                                       const GLenum target,
                                       const GLsizei num_mipmap_level,
                                       const GLenum internal_fmt,
                                       const GLsizei width,
                                       const GLsizei height) {
  BindTexture(tex_name, target);
  glTexStorage2D(target, num_mipmap_level, internal_fmt, width, height);
}

void as::TextureManager::UpdateTexture2D(
    const std::string &tex_name, const GLenum target, const GLint mipmap_level,
    const GLint x_ofs, const GLint y_ofs, const GLsizei width,
    const GLsizei height, const GLenum fmt, const GLenum type,
    const GLvoid *data) {
  BindTexture(tex_name, target);
  glTexSubImage2D(target, mipmap_level, x_ofs, y_ofs, width, height, fmt, type,
                  data);
  // Save the parameters
  UpdateTexture2DPrevParams prev_params = {
      target, mipmap_level, x_ofs, y_ofs, width, height, fmt, type, data};
  update_texture_2d_prev_params_[tex_name] = prev_params;
}

void as::TextureManager::UpdateCubeMapTexture2D(
    const std::string &tex_name, const GLenum target, const GLint mipmap_level,
    const GLint x_ofs, const GLint y_ofs, const GLsizei width,
    const GLsizei height, const GLenum fmt, const GLenum type,
    const GLvoid *data) {
  BindTexture(tex_name, GL_TEXTURE_CUBE_MAP);
  glTexSubImage2D(target, mipmap_level, x_ofs, y_ofs, width, height, fmt, type,
                  data);
  // Save the parameters
  UpdateTexture2DPrevParams prev_params = {
      target, mipmap_level, x_ofs, y_ofs, width, height, fmt, type, data};
  update_texture_2d_prev_params_[tex_name] = prev_params;
}

void as::TextureManager::UpdateTexture2D(const std::string &tex_name,
                                         const GLenum target) {
  const UpdateTexture2DPrevParams &prev_params =
      GetUpdateTexture2DPrevParams(tex_name);
  UpdateTexture2D(tex_name, prev_params.target, prev_params.mipmap_level,
                  prev_params.x_ofs, prev_params.y_ofs, prev_params.width,
                  prev_params.height, prev_params.fmt, prev_params.type,
                  prev_params.data);
}

void as::TextureManager::UpdateCubeMapTexture2D(const std::string &tex_name,
                                                const GLenum target) {
  const UpdateTexture2DPrevParams &prev_params =
      GetUpdateTexture2DPrevParams(tex_name);
  UpdateCubeMapTexture2D(tex_name, prev_params.target, prev_params.mipmap_level,
                         prev_params.x_ofs, prev_params.y_ofs,
                         prev_params.width, prev_params.height, prev_params.fmt,
                         prev_params.type, prev_params.data);
}

void as::TextureManager::GenMipmap(const std::string &tex_name,
                                   const GLenum target) {
  BindTexture(tex_name);
  glGenerateMipmap(target);
}

void as::TextureManager::SetTextureParamFloat(const std::string &tex_name,
                                              const GLenum target,
                                              const GLenum pname,
                                              const GLfloat param) {
  BindTexture(tex_name, target);
  glTexParameterf(target, pname, param);
}

void as::TextureManager::SetTextureParamInt(const std::string &tex_name,
                                            const GLenum target,
                                            const GLenum pname,
                                            const GLint param) {
  BindTexture(tex_name, target);
  glTexParameteri(target, pname, param);
}

void as::TextureManager::SetTextureParamFloatVector(const std::string &tex_name,
                                                    const GLenum target,
                                                    const GLenum pname,
                                                    const GLfloat *params) {
  BindTexture(tex_name, target);
  glTexParameterfv(target, pname, params);
}

void as::TextureManager::SetTextureParamIntVector(const std::string &tex_name,
                                                  const GLenum target,
                                                  const GLenum pname,
                                                  const GLint *params) {
  BindTexture(tex_name, target);
  glTexParameteriv(target, pname, params);
}

void as::TextureManager::UnbindTexture(const std::string &tex_name,
                                       const GLenum target) {
  const auto index_manager_target = std::make_tuple(tex_name, target);
  index_manager_.UnbindTarget1(index_manager_target);
  // Bind to unit index 0 to avoid programming errors
  BindTexture(tex_name, target, 0);
}

void as::TextureManager::DeleteTexture(const std::string &tex_name) {
  const GLuint tex_hdlr = GetTextureHdlr(tex_name);
  glDeleteTextures(1, &tex_hdlr);
  // Delete previous parameters
  bind_texture_prev_params_.erase(tex_name);
  update_texture_2d_prev_params_.erase(tex_name);
}

GLuint as::TextureManager::GetTextureHdlr(const std::string &tex_name) const {
  if (!HasTexture(tex_name)) {
    throw std::runtime_error("Could not find the texture name '" + tex_name +
                             "'");
  }
  return hdlrs_.at(tex_name);
}

bool as::TextureManager::HasTexture(const std::string &tex_name) const {
  return hdlrs_.count(tex_name) > 0;
}

const as::TextureManager::BindTexturePrevParams &
as::TextureManager::GetBindTexturePrevParams(
    const std::string &tex_name) const {
  if (bind_texture_prev_params_.count(tex_name) == 0) {
    throw std::runtime_error(
        "Could not find the previous parameters for texture name '" + tex_name +
        "'");
  }
  return bind_texture_prev_params_.at(tex_name);
}

const as::TextureManager::UpdateTexture2DPrevParams &
as::TextureManager::GetUpdateTexture2DPrevParams(
    const std::string &tex_name) const {
  if (update_texture_2d_prev_params_.count(tex_name) == 0) {
    throw std::runtime_error(
        "Could not find the previous parameters for texture name '" + tex_name +
        "'");
  }
  return update_texture_2d_prev_params_.at(tex_name);
}

void as::TextureManager::InitLimits() {
  GLint value;
  glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &value);
  index_manager_.SetMaxIdx(value);
}
