#include "as/model/node.hpp"

as::Node::Node(const std::string& name, const Node* parent)
    : name_(name), parent_(parent) {}

void as::Node::AddMesh(const Mesh* mesh) { meshes_.push_back(mesh); }

void as::Node::AddChild(const Node* child) { children_.push_back(child); }

std::string as::Node::GetName() const { return name_; }

const std::vector<const as::Mesh*> as::Node::GetMeshes() const {
  return meshes_;
}

const as::Node* as::Node::GetParent() const { return parent_; }

const std::vector<const as::Node*> as::Node::GetChildren() const {
  return children_;
}
