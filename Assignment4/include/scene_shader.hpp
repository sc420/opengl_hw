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
    // NOTE: Don't use mat2 or mat3 because each column is padded like vec4
    // Reference: https://www.khronos.org/opengl/wiki/Talk:Uniform_Buffer_Object
    glm::mat4 fixed_norm_model;  // 64*0=0, +64->64
    glm::vec3 light_color;       // 16*3=48, +12->60

    bool pad_light_pos[4];  // +4->64
    glm::vec3 light_pos;    // 16*4=64, +12->76

    bool pad_light_intensity[4];  // +4->80
    glm::vec3 light_intensity;    // 16*5=80, +12->92

    bool pad_view_pos[4];  // +4->96
    glm::vec3 view_pos;    // 16*6=96, +12->108

    bool pad[100];
  };

  SceneShader();

  /* Shader Registrations */

  void RegisterSkyboxShader(const SkyboxShader &skybox_shader);

  /* Model Handlers */

  void LoadModel();

  /* GL Initializations */

  void Init() override;

  void InitVertexArrays();

  void InitUniformBlocks();

  void InitTextures();

  void InitLighting();

  void SetSkyboxTexture();

  /* GL Drawing Methods */

  void Draw() override;

  /* State Updaters */

  void UpdateGlobalTrans(const GlobalTrans &global_trans);

  void UpdateModelTrans(const float add_rotation = 0.0f);

  void UpdateModelMaterial(const as::Material &material);

  void UpdateViewPos(const glm::vec3 &view_pos);

  /* Name Management */

  std::string GetId() const override;

  std::string GetGlobalTransBufferName() const;

  std::string GetGlobalTransUniformBlockName() const;

 protected:
  /* Model Handlers */

  virtual as::Model &GetModel() override;

  /* GL Initializations */

  GLsizei GetNumMipmapLevels() const;

  /* Name Management */

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

  /* Model States */
  float model_rotation;

  /* GL States */
  GlobalTrans global_trans_;
  ModelTrans model_trans_;
  ModelMaterial model_material_;
  Lighting lighting_;

  /* State Updaters */

  void UpdateFixedNormModel();
};
}  // namespace shader
