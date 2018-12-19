#include "as/model/model.hpp"

void as::Model::LoadFile(const std::string &path, const unsigned int flags) {
  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(path, flags);
  // Check errors
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    throw std::runtime_error("Could not read the file by Assimp. Error: " +
                             std::string(importer.GetErrorString()));
  }
  // Get the directory
  const fs::path p(path);
  const std::string dir = p.parent_path().string();
  // Reset the model
  Reset();
  // Process the root node
  ProcessNode(dir, scene, scene->mRootNode);
}

const std::vector<as::Node> &as::Model::GetNodes() const { return nodes_; }

const std::vector<as::Mesh> &as::Model::GetMeshes() const { return meshes_; }

void as::Model::Reset() {
  nodes_.clear();
  meshes_.clear();
}

void as::Model::ProcessNode(const fs::path &dir, const aiScene *ai_scene,
                            const aiNode *ai_node) {
  std::queue<const aiNode *> waiting_nodes;
  std::queue<Node *> parents;
  waiting_nodes.push(ai_node);
  parents.push(nullptr);
  while (!waiting_nodes.empty()) {
    // Get the current node from the queue
    const aiNode *cur_ai_node = waiting_nodes.front();
    waiting_nodes.pop();
    // Get the current parent from the queue
    Node *parent = parents.front();
    parents.pop();
    // Create a node
    Node node(ai_node->mName.C_Str(), parent);
    // Process each mesh in the node
    for (size_t i = 0; i < cur_ai_node->mNumMeshes; i++) {
      const aiMesh *ai_mesh = ai_scene->mMeshes[cur_ai_node->mMeshes[i]];
      const Mesh mesh = ProcessMesh(dir, ai_scene, ai_mesh);
      node.AddMesh(&mesh);
      // Add the mesh to the list
      meshes_.push_back(mesh);
    }
    // Add the children nodes to the waiting nodes
    for (size_t i = 0; i < cur_ai_node->mNumChildren; i++) {
      waiting_nodes.push(cur_ai_node->mChildren[i]);
      parents.push(&node);
    }
    if (parent != nullptr) {
      // Add the current node to the children of the parent
      parent->AddChild(&node);
    }
    // Add the current node to the list
    nodes_.push_back(node);
  }
}

const as::Mesh as::Model::ProcessMesh(const fs::path &dir,
                                      const aiScene *ai_scene,
                                      const aiMesh *ai_mesh) {
  const std::vector<Vertex> vertices = ProcessMeshVertices(ai_mesh);
  const std::vector<size_t> idxs = ProcessMeshIdxs(ai_mesh);
  const Material material(dir, ai_scene, ai_mesh);
  return Mesh(ai_mesh->mName.C_Str(), vertices, idxs, material);
}

const std::vector<as::Vertex> as::Model::ProcessMeshVertices(
    const aiMesh *ai_mesh) const {
  std::vector<Vertex> vertices;
  for (size_t vtx_idx = 0; vtx_idx < ai_mesh->mNumVertices; vtx_idx++) {
    Vertex vertex;
    if (ai_mesh->HasPositions()) {
      const aiVector3D &ai_vertex = ai_mesh->mVertices[vtx_idx];
      vertex.pos = ConvertAiVectorToVec(ai_vertex);
    }
    if (ai_mesh->mTextureCoords[0]) {
      const aiVector3D &ai_tex_coords = ai_mesh->mTextureCoords[0][vtx_idx];
      vertex.tex_coords = glm::vec2(ConvertAiVectorToVec(ai_tex_coords));
    }
    if (ai_mesh->HasNormals()) {
      const aiVector3D &ai_normal = ai_mesh->mNormals[vtx_idx];
      vertex.normal = ConvertAiVectorToVec(ai_normal);
    }
    if (ai_mesh->HasTangentsAndBitangents()) {
      const aiVector3D &ai_tangent = ai_mesh->mTangents[vtx_idx];
      const aiVector3D &ai_bitangent = ai_mesh->mBitangents[vtx_idx];
      vertex.tangent = ConvertAiVectorToVec(ai_tangent);
      vertex.bitangent = ConvertAiVectorToVec(ai_bitangent);
    }
    vertices.push_back(vertex);
  }
  return vertices;
}

const std::vector<size_t> as::Model::ProcessMeshIdxs(
    const aiMesh *ai_mesh) const {
  std::vector<size_t> idxs;
  for (size_t face_idx = 0; face_idx < ai_mesh->mNumFaces; face_idx++) {
    const aiFace &face = ai_mesh->mFaces[face_idx];
    // Iterate through each triangle index
    for (size_t tri_idx = 0; tri_idx < face.mNumIndices; tri_idx++) {
      idxs.push_back(face.mIndices[tri_idx]);
    }
  }
  return idxs;
}
