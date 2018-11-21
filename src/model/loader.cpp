#include "as/model/loader.hpp"

void as::LoadModelByTinyobj(const std::string &path,
                            std::vector<glm::vec3> &vertices,
                            std::vector<glm::vec3> &normals,
                            std::vector<glm::vec2> &tex_coords) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.c_str());
  if (!err.empty()) {
    std::cerr << err << std::endl;
  }
  if (!ret) {
    exit(1);
  }
  vertices.clear();
  normals.clear();
  tex_coords.clear();
  // Loop over shapes
  for (size_t s = 0; s < shapes.size(); s++) {
    // Loop over faces(polygon)
    size_t index_offset = 0;
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      int fv = shapes[s].mesh.num_face_vertices[f];
      // Loop over vertices in the face.
      for (int v = 0; v < fv; v++) {
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
        tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
        tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
        tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
        tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
        tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
        tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
        tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
        tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];
        vertices.push_back(glm::vec3(vx, vy, vz));
        normals.push_back(glm::vec3(nx, ny, nz));
        tex_coords.push_back(glm::vec2(tx, ty));
      }
      index_offset += fv;
    }
  }
}

void as::LoadTextureByStb(const std::string &path, const GLint req_comp,
                          GLsizei &width, GLsizei &height, GLint &comp,
                          std::vector<GLubyte> &texels) {
  unsigned char *data =
      stbi_load(path.c_str(), &width, &height, &comp, req_comp);
  unsigned int len = width * height * comp;
  texels.assign(data, data + len);
  stbi_image_free(data);
}
