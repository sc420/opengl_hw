#pragma once

/* Third-party libraries */
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

/* Project libraries */
#include "as/common.hpp"
#include "as/model/mesh.hpp"
#include "as/model/node.hpp"

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

  void ProcessNode(const aiNode *ai_node, const aiScene *ai_scene);

  const Mesh ProcessMesh(const aiMesh *ai_mesh, const aiScene *ai_scene);
};
}  // namespace as
