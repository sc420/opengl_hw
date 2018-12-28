#pragma once

#include "depth_shader.hpp"
#include "shader.hpp"
#include "skybox_shader.hpp"
#include "trans_dto.hpp"

namespace shader {
class DepthShader;

class SkyboxShader;

class SceneShader : public Shader {
 public:
  struct ModelTrans {
    glm::mat4 trans;  // 64*0=0, +64->64
  };

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
  };

  struct Lighting {
    // NOTE: Don't use mat2 or mat3 because each column is padded like vec4
    // Reference: https://www.khronos.org/opengl/wiki/Talk:Uniform_Buffer_Object
    glm::mat4 fixed_norm_model;  // 64*0=0, +64->64
    glm::mat4 light_trans;       // 64*1=64, +64->128
    glm::vec3 light_color;       // 16*8=128, +12->140

    bool pad_light_pos[4];  // +4->144
    glm::vec3 light_pos;    // 16*9=144, +12->156

    bool pad_light_intensity[4];  // +4->160
    glm::vec3 light_intensity;    // 16*10=160, +12->172

    bool pad_view_pos[4];  // +4->176
    glm::vec3 view_pos;    // 16*11=176, +12->188
  };

  SceneShader();

  /* Shader Registrations */

  void RegisterDepthShader(const DepthShader &depth_shader);

  void RegisterSkyboxShader(const SkyboxShader &skybox_shader);

  /* Model Handlers */

  void LoadModel();

  /* GL Initializations */

  void Init() override;

  void InitVertexArrays();

  void InitUniformBlocks();

  void InitLightTrans();

  void InitTextures();

  void ReuseSkyboxTexture();

  /* GL Drawing Methods */

  void Draw() override;

  void DrawDepth();

  void UpdateQuadLighting();

  void UpdateSceneLighting();

  /* State Getters */

  dto::GlobalTrans GetGlobalTrans() const;

  glm::vec3 GetLightPos() const;

  /* State Updaters */

  void UpdateGlobalTrans(const dto::GlobalTrans &global_trans);

  void UpdateQuadModelTrans();

  void UpdateSceneModelTrans(const float add_rotation = 0.0f);

  void UpdateModelMaterial(const as::Material &material);

  void UpdateViewPos(const glm::vec3 &view_pos);

  /* Name Management */

  std::string GetId() const override;

  std::string GetGlobalTransBufferName() const;

  std::string GetGlobalTransUniformBlockName() const;

 protected:
  /* Model Handlers */

  as::Model &GetSceneModel();

  as::Model &GetQuadModel();

  /* GL Initializations */

  GLsizei GetNumMipmapLevels() const;

  /* Name Management */

  std::string GetModelTransBufferName() const;

  std::string GetModelMaterialBufferName() const;

  std::string GetLightingBufferName() const;

  std::string GetModelTransUniformBlockName() const;

  std::string GetModelMaterialUniformBlockName() const;

  std::string GetLightingUniformBlockName() const;

  std::string GetSceneGroupName() const;

  std::string GetQuadGroupName() const;

  std::string GetTextureUnitName(const as::Texture &texture) const;

  std::string GetSkyboxTextureUnitName() const;

 private:
  /* Shaders */
  const DepthShader *depth_shader_;
  const SkyboxShader *skybox_shader_;

  /* Models */
  as::Model scene_model_;
  as::Model quad_model_;

  /* Model States */
  float model_rotation;

  /* GL States */
  dto::GlobalTrans global_trans_;
  ModelTrans model_trans_;
  ModelMaterial model_material_;
  Lighting lighting_;

  /* State Updaters */

  void UpdateFixedNormModel();

  /* GL Drawing Methods */

  void DrawModel(const as::Model &model, const std::string &group_name);

  void DrawModelWithoutTextures(const as::Model &model,
                                const std::string &group_name);
};
}  // namespace shader
