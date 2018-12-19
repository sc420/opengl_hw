#pragma once

#include "shader.hpp"
#include "skybox_shader.hpp"

namespace shader {
class SkyboxShader;

class SceneShader : public Shader {
 public:
  struct GlobalTrans {
    glm::mat4 model;  // 64*0=0, +64->64
    glm::mat4 view;   // 64*1=64, +64->128
    glm::mat4 proj;   // 64*2=128, +64->192
  };

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
    glm::mat4 fixed_norm_model;  // 64*0=0, +64->64
    glm::vec4 light_color;       // 4*16=64, +16->80
    glm::vec4 light_pos;         // 4*20=80, +16->96
    glm::vec4 light_intensity;   // 4*24=96, +16->112
    glm::vec4 view_pos;          // 4*28=112, +16->128
  };

  SceneShader();

  /* Shader registrations */

  void RegisterSkyboxShader(const SkyboxShader &skybox_shader);

  /* Model handlers */

  void LoadModel();

  /* GL initialization methods */

  void Init() override;

  void InitVertexArrays();

  void InitUniformBlocks();

  void InitTextures();

  void InitLighting();

  void SetSkyboxTexture();

  /* GL drawing methods */

  void Draw() override;

  /* State updating methods */

  void UpdateGlobalTrans(const GlobalTrans &global_trans);

  void UpdateModelTrans(const float add_rotation = 0.0f);

  void UpdateModelMaterial(const as::Material &material);

  void UpdateViewPos(const glm::vec3 &view_pos);

  /* Name management */

  std::string GetId() const override;

  std::string GetGlobalTransBufferName() const;

  std::string GetGlobalTransUniformBlockName() const;

 protected:
  /* Model handlers */

  virtual as::Model &GetModel() override;

  /* GL initialization methods */

  GLsizei GetNumMipmapLevels() const;

  /* Name management */

  std::string GetModelTransBufferName() const;

  std::string GetModelMaterialBufferName() const;

  std::string GetLightingBufferName() const;

  std::string GetModelTransUniformBlockName() const;

  std::string GetModelMaterialUniformBlockName() const;

  std::string GetLightingUniformBlockName() const;

  std::string GetTextureUnitName(const as::Texture &texture) const;

  std::string GetSkyboxTextureUnitName() const;

 private:
  /* Shaders */
  std::shared_ptr<SkyboxShader> skybox_shader_;

  /* Models */
  as::Model scene_model_;

  /* Model states */
  float model_rotation;

  /* GL states */
  GlobalTrans global_trans_;
  ModelTrans model_trans_;
  ModelMaterial model_material_;
  Lighting lighting_;

  /* State updating methods */

  void UpdateFixedNormModel();
};
}  // namespace shader
