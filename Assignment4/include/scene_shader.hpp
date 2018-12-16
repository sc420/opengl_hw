#pragma once

#include "shader.hpp"

namespace shader {
class SceneShader : public Shader {
 public:
  struct GlobalTrans {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
  };

  struct ModelTrans {
    glm::mat4 trans;
  };

  struct Light {
    glm::vec3 pos;
  };

  SceneShader();

  /* Model handlers */

  void LoadModel();

  /* GL initialization methods */

  void Init() override;

  void InitVertexArrays();

  void InitUniformBlocks();

  void InitTextures();

  /* GL drawing methods */

  void Draw() override;

  /* State updating methods */

  void UpdateGlobalTrans(const GlobalTrans &global_trans);

  void UpdateModelTrans(const ModelTrans &model_trans);

  /* Name management */

  std::string GetId() const override;

 protected:
  /* Model handlers */

  virtual as::Model &GetModel() override;

  /* GL initialization methods */

  GLsizei GetNumMipmapLevels() const;

  /* Name management */

  std::string GetGlobalTransBufferName() const;

  std::string GetModelTransBufferName() const;

  std::string GetGlobalTransUniformBlockName() const;

  std::string GetModelTransUniformBlockName() const;

 private:
  as::Model scene_model_;

  GlobalTrans global_trans_;

  ModelTrans model_trans_;
};
}  // namespace shader
