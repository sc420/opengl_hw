#pragma once

#include "depth_shader.hpp"
#include "scene_model_dto.hpp"
#include "shader.hpp"
#include "skybox_shader.hpp"
#include "trans_dto.hpp"

namespace shader {
class DepthShader;

class SkyboxShader;

class SceneShader : public Shader {
 public:
  struct ModelMaterial {
    bool use_ambient_tex;  // 4*0=0, +1->1

    bool pad_use_diffuse_tex[3];  // +3->4
    bool use_diffuse_tex;         // 4*1=4

    bool pad_use_specular_tex[3];  // +3->8
    bool use_specular_tex;         // 4*2=8, +1->9

    bool pad_use_height_tex[3];  // +3->12
    bool use_height_tex;         // 4*3=12, +1->13

    bool pad_use_normals_tex[3];  // +3->16
    bool use_normals_tex;         // 4*4=16, +1->17

    bool pad_ambient_color[15];  // +15->32
    glm::vec4 ambient_color;     // 16*1=32, +16->48
    glm::vec4 diffuse_color;     // 16*2=48, +16->64
    glm::vec4 specular_color;    // 16*3=64, +16->80
    float shininess;             // 4*20=80, +4->84

    bool use_env_map;  // 4*21=84, +1->85

    bool pad_use_fog[3];  // +3->88
    bool use_fog;         // 4*22=88, +1->89

    bool pad_pad_mix_fog_with_skybox[3];  // +3->92
    bool mix_fog_with_skybox;             // 4*23=92, +1->93

    bool pad[3];  // +3->96
  };

  struct Lighting {
    glm::mat4 light_trans;  // 64*1=64, +64->128
    glm::vec3 light_color;  // 16*8=128, +12->140

    bool pad_light_pos[4];  // +4->144
    glm::vec3 light_pos;    // 16*9=144, +12->156

    bool pad_light_intensity[4];  // +4->160
    glm::vec3 light_intensity;    // 16*10=160, +12->172

    bool pad_view_pos[4];  // +4->176
    glm::vec3 view_pos;    // 16*11=176, +12->188

    bool pad[4];  // +4->192=16*12
  };

  SceneShader();

  /* Shader Registrations */

  void RegisterDepthShader(const DepthShader &depth_shader);

  void RegisterSkyboxShader(const SkyboxShader &skybox_shader);

  /* Model Getters */

  const std::map<std::string, dto::SceneModel> &GetSceneModels() const;

  dto::SceneModel &GetSceneModel(const std::string &scene_model_name);

  /* GL Initializations */

  void Init();

  void ReuseSkyboxTexture();

  void BindTextures();

  /* GL Drawing Methods */

  void Draw();

  /* State Getters */

  dto::GlobalTrans GetGlobalTrans() const;

  glm::vec3 GetLightPos() const;

  glm::vec3 GetLightAngles() const;

  glm::mat4 GetLightProjection() const;

  float GetMinDistanceToModel(const glm::vec3 &pos,
                              const std::string &scene_model_name) const;

  /* State Updaters */

  void UpdateGlobalTrans(const dto::GlobalTrans &global_trans);

  void UpdateSceneModelTrans(const float add_rotation = 0.0f);

  void UpdateViewPos(const glm::vec3 &view_pos);

  void UpdateSceneModel(const dto::SceneModel &scene_model);

  void ToggleNormalHeight(const bool toggle);

  void ToggleFog(const bool toggle);

  void ToggleMixFogWithSkybox(const bool toggle);

  /* Name Management */

  std::string GetId() const override;

  std::string GetGlobalTransBufferName() const;

  std::string GetGlobalTransUniformBlockName() const;

 protected:
  /* GL Initializations */

  GLsizei GetNumMipmapLevels() const;

  /* Name Management */

  std::string GetModelTransBufferName() const;

  std::string GetModelMaterialBufferName() const;

  std::string GetLightingBufferName() const;

  std::string GetInstancingTranslationsBufferName(
      const dto::SceneModel &scene_model) const;

  std::string GetInstancingRotationsBufferName(
      const dto::SceneModel &scene_model) const;

  std::string GetInstancingScalingsBufferName(
      const dto::SceneModel &scene_model) const;

  std::string GetSkyboxTextureUnitName() const;

  std::string GetModelTransUniformBlockName() const;

  std::string GetModelMaterialUniformBlockName() const;

  std::string GetLightingUniformBlockName() const;

 private:
  /* Model States */
  float model_rotation;

  /* Shaders */
  const DepthShader *depth_shader_;
  const SkyboxShader *skybox_shader_;

  /* Models */
  std::map<std::string, dto::SceneModel> scene_models_;

  /* GL States */
  dto::GlobalTrans global_trans_;
  dto::ModelTrans model_trans_;
  ModelMaterial model_material_;
  Lighting lighting_;
  bool use_normal_height;

  /* Model Initialization */

  void LoadModels();

  void InitModels();

  /* GL Initialization */

  void InitVertexArrays();

  void InitInstancingVertexArrays();

  void InitUniformBlocks();

  void InitLightTrans();

  /* State Updaters */

  void UpdateModelTrans(const dto::SceneModel &scene_model);

  void UpdateLighting(const dto::SceneModel &scene_model);

  void UpdateModelMaterial(const dto::SceneModel &scene_model);

  void UpdateModelMaterial(const as::Material &material);

  /* GL Drawing Methods */

  void DrawModel(const dto::SceneModel &scene_model);
};
}  // namespace shader
