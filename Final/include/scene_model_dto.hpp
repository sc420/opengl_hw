#pragma once

#include "as/common.hpp"
#include "as/gl/gl_tools.hpp"
#include "as/model/model.hpp"
#include "as/trans/camera.hpp"

namespace dto {
class SceneModel {
 public:
  SceneModel();

  SceneModel(const std::string &id, const std::string &path,
             const unsigned int flags, const std::string &tex_unit_group_name,
             const GLsizei num_mipmap_levels, as::GLManagers *gl_managers);

  /* Name Management */

  std::string GetId() const;

  std::string GetVertexArrayGroupName() const;

  /* Model Getters */

  const as::Model &GetModel() const;

  /* State Setters */

  void SetTranslation(const glm::vec3 &translation);

  void SetRotation(const glm::vec3 &rotation);

  void SetScaling(const glm::vec3 &scaling);

  void SetInstancingTranslations(const std::vector<glm::vec3> &translations);

  void SetInstancingRotation(const std::vector<glm::vec3> &rotations);

  void SetInstancingScaling(const std::vector<glm::vec3> &scalings);

  void SetLightPos(const glm::vec3 &light_pos);

  void SetLightColor(const glm::vec3 &light_color);

  void SetLightIntensity(const glm::vec3 &light_intensity);

  void SetUseEnvMap(const bool use_env_map);

  /* State Getters */

  glm::mat4 GetTrans() const;

  std::vector<glm::mat4> GetInstancingTransforms() const;

  glm::vec3 GetLightPos() const;

  glm::vec3 GetLightColor() const;

  glm::vec3 GetLightIntensity() const;

  bool GetUseEnvMap() const;

 private:
  /* Name Management */
  std::string id_;

  /* Model */
  as::Model model_;

  /* Transformation */
  glm::vec3 translation_;
  glm::vec3 rotation_;
  glm::vec3 scaling_;

  /* Instancing */
  std::vector<glm::vec3> instancing_translations_;
  std::vector<glm::vec3> instancing_rotations_;
  std::vector<glm::vec3> instancing_scalings_;

  /* Lighting */
  glm::vec3 light_pos_;
  glm::vec3 light_color_;
  glm::vec3 light_intensity_;

  /* Material */
  bool use_env_map_;

  /* Model Initialization */

  void LoadFile(const std::string &path, const unsigned int flags);

  /* GL Initialization */

  void InitTextures(const std::string &tex_unit_group_name,
                    const GLsizei num_mipmap_levels,
                    as::GLManagers *gl_managers);

  /* GL Drawing Methods */

  glm::mat4 GetTransformMatrix(const glm::vec3 &translation,
                               const glm::vec3 &rotation,
                               const glm::vec3 &scaling) const;

  /* Name Management */

  std::string GetTextureUnitName(const std::string &tex_unit_group_name,
                                 const as::Texture &texture) const;
};

}  // namespace dto
