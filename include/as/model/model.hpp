#pragma once

/* Third-party libraries */
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

/* Project libraries */
#include "as/common.hpp"
#include "as/model/mesh.hpp"

namespace as {
class Model {
 public:
  void LoadFile(const std::string &path);

  std::vector<Mesh> meshes;

 private:
  void ProcessNode(const aiNode *node, const aiScene *scene);

  Mesh ProcessMesh(const aiMesh *mesh, const aiScene *scene);
};
}  // namespace as
