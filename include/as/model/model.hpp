#pragma once

/* Third-party libraries */
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

/* Project libraries */
#include "as/common.hpp"
#include "as/model/loader.hpp"
#include "as/model/mesh.hpp"
#include "as/model/node.hpp"
#include "as/model/texture.hpp"

namespace fs = std::experimental::filesystem;

namespace as {
class Model {
 public:
  void LoadFile(const std::string &path);

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

  std::vector<Vertex> ProcessMeshVertices(const aiMesh *ai_mesh) const;

  std::vector<size_t> ProcessMeshIdxs(const aiMesh *ai_mesh) const;

  std::set<Texture> ProcessMeshTextures(const fs::path &dir,
                                        const aiScene *ai_scene,
                                        const aiMesh *ai_mesh) const;

  std::set<Texture> ProcessMaterialTextures(
      const fs::path &dir, const aiMaterial *ai_material,
      const aiTextureType ai_texture_type) const;

  std::string AiTextureTypeToStr(const aiTextureType ai_texture_type) const;
};
}  // namespace as
