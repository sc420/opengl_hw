#include "as/model/model.hpp"

void as::Model::LoadFile(const std::string & path)
{
  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(path, aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_Triangulate);
  // Check errors
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
  {
    std::cerr << "Assimp error:" << std::endl;
    std::cerr << importer.GetErrorString() << std::endl;
    throw std::runtime_error("Could not read the file by Assimp");
  }
  const std::string dir = path.substr(0, path.find_last_of('/'));
  // Clear all meshes
  meshes.clear();
  // Process the root node
  ProcessNode(scene->mRootNode, scene);
}

void as::Model::ProcessNode(const aiNode * node, const aiScene * scene)
{
  // process all the node's meshes (if any)
  for (size_t i = 0; i < node->mNumMeshes; i++)
  {
    const aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    meshes.push_back(ProcessMesh(mesh, scene));
  }
  // then do the same for each of its children
  for (size_t i = 0; i < node->mNumChildren; i++)
  {
    ProcessNode(node->mChildren[i], scene);
  }
}

as::Mesh as::Model::ProcessMesh(const aiMesh * mesh, const aiScene * scene)
{
  std::vector<Vertex> vertices;
  std::vector<size_t> idxs;
  // Iterate through each vertex
  for (size_t vtx_idx = 0; vtx_idx < mesh->mNumVertices; vtx_idx++)
  {
    Vertex vertex;
    const aiVector3D &m_vertex = mesh->mVertices[vtx_idx];
    const aiVector3D &m_normal = mesh->mNormals[vtx_idx];
    vertex.pos = glm::vec3(m_vertex.x, m_vertex.y, m_vertex.z);
    vertex.normal = glm::vec3(m_normal.x, m_normal.y, m_normal.z);
    if (mesh->mTextureCoords[0]) {
      const aiVector3D & m_tex_coords = mesh->mTextureCoords[0][vtx_idx];
      vertex.tex_coords = glm::vec2(m_tex_coords.x, m_tex_coords.y);
    }
    else {
      vertex.tex_coords = glm::vec2(0.0f);
    }
    vertices.push_back(vertex);
  }
  // Iterate through each face
  for (size_t face_idx = 0; face_idx < mesh->mNumFaces; face_idx++)
  {
    const aiFace &face = mesh->mFaces[face_idx];
    // Iterate through each triangle index
    for (size_t tri_idx = 0; tri_idx < face.mNumIndices; tri_idx++) {
      idxs.push_back(face.mIndices[tri_idx]);
    }
  }
  return Mesh(vertices, idxs);
}
