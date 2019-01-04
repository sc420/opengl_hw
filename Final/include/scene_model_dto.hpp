#pragma once

#include "as/common.hpp"
#include "as/gl/gl_tools.hpp"
#include "as/model/model.hpp"

namespace dto {
class SceneModel {
 public:
  SceneModel();

  SceneModel(const std::string &path, const unsigned int flags,
             const std::string &tex_unit_group_name,
             const GLsizei num_mipmap_levels, as::GLManagers *gl_managers);

  const as::Model &GetModel() const;

 private:
  as::Model model_;

  void LoadFile(const std::string &path, const unsigned int flags);

  void InitTextures(const std::string &tex_unit_group_name,
                    const GLsizei num_mipmap_levels,
                    as::GLManagers *gl_managers);

  std::string GetTextureUnitName(const std::string &tex_unit_group_name,
                                 const as::Texture &texture) const;
};

}  // namespace dto
