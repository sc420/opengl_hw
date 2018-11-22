#pragma once

#include "as/common.hpp"
#include "as/model/mesh.hpp"

namespace as {
class Node {
 public:
  Node(const std::string &name, const Node *parent);

  void AddMesh(const Mesh *mesh);

  void AddChild(const Node *child);

  std::string GetName() const;

  const std::vector<const Mesh *> &GetMeshes() const;

  const Node *GetParent() const;

  const std::vector<const Node *> &GetChildren() const;

 private:
  std::string name_;

  std::vector<const Mesh *> meshes_;

  const Node *parent_;

  std::vector<const Node *> children_;
};
}  // namespace as
