#pragma once

/* MSVC Libraries */
#include <codeanalysis\warnings.h>

/* Third-party Libraries */
#pragma warning(push)
#pragma warning(disable : ALL_CODE_ANALYSIS_WARNINGS)
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#pragma warning(pop)

/* Project Libraries */
#include "as/common.hpp"
#include "as/model/converter.hpp"
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
};

}  // namespace as
