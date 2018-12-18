#pragma once

/* MSVC libraries */
#include <codeanalysis\warnings.h>

/* Third-party libraries */
#pragma warning(push)
#pragma warning(disable : ALL_CODE_ANALYSIS_WARNINGS)
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#pragma warning(pop)

/* Project libraries */
#include "as/common.hpp"
#include "as/model/loader.hpp"
#include "as/model/material.hpp"
#include "as/model/mesh.hpp"
#include "as/model/node.hpp"
#include "as/model/texture.hpp"

namespace fs = std::experimental::filesystem;

namespace as {
class Model {
 public:
  void LoadFile(const std::string &path, const unsigned int flags);

  const std::vector<Node> &GetNodes() const;

  const std::vector<Mesh> &GetMeshes() const;

 private:
  std::vector<Node> nodes_;

  std::vector<Mesh> meshes_;

  void Reset();

  void ProcessNode(const fs::path &dir, const aiScene *ai_scene,
                   const aiNode *ai_node);

  const Mesh ProcessMesh(const fs::path &dir, const aiScene *ai_scene,
                         const aiMesh *ai_mesh);

  const std::vector<Vertex> ProcessMeshVertices(const aiMesh *ai_mesh) const;

  const std::vector<size_t> ProcessMeshIdxs(const aiMesh *ai_mesh) const;

  const Material ProcessMeshMaterial(const fs::path &dir,
                                     const aiScene *ai_scene,
                                     const aiMesh *ai_mesh) const;

  const std::set<Texture> ProcessMaterialTextures(
      const fs::path &dir, const aiMaterial *ai_material) const;

  std::set<Texture> ProcessMaterialTexturesOfType(
      const fs::path &dir, const aiMaterial *ai_material,
      const aiTextureType ai_texture_type) const;

  template <class TReturn, class TAiValue>
  const TReturn GetMaterialProperty(const aiMaterial *ai_material,
                                    const std::string &key) const;

  template <>
  const glm::vec4 GetMaterialProperty<glm::vec4, aiColor4D>(
      const aiMaterial *ai_material, const std::string &key) const;

  template <>
  const float GetMaterialProperty<float, float>(const aiMaterial *ai_material,
                                                const std::string &key) const;

  glm::vec3 ConvertAiVectorToVec(const aiVector3D &ai_color) const;

  glm::vec4 ConvertAiColorToVec(const aiColor4D &ai_color) const;
};

template <>
inline const glm::vec4 Model::GetMaterialProperty<glm::vec4, aiColor4D>(
    const aiMaterial *ai_material, const std::string &key) const {
  aiColor4D value;
  if (ai_material->Get(key.c_str(), 0, 0, value) == AI_SUCCESS) {
    return ConvertAiColorToVec(value);
  } else {
    return glm::vec4(0.0f);
  }
}

template <>
inline const float Model::GetMaterialProperty<float, float>(
    const aiMaterial *ai_material, const std::string &key) const {
  float value;
  if (ai_material->Get(key.c_str(), 0, 0, value) == AI_SUCCESS) {
    return value;
  } else {
    return 0.0f;
  }
}

}  // namespace as
