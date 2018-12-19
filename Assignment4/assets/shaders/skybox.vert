#version 440

layout(std140) uniform GlobalTrans {
  mat4 model;
  mat4 view;
  mat4 proj;
}
global_trans;

layout(location = 0) in vec3 in_pos;

layout(location = 0) out vec3 vs_tex_coords;

mat4 RemoveTranslation(const mat4 trans) { return mat4(mat3(trans)); }

vec4 ForceDepthTo1(const vec4 pos) { return vec4(vec3(pos), pos.z); }

void main() {
  const mat4 view_wo_translation = RemoveTranslation(global_trans.view);
  const mat4 global_mvp = global_trans.proj * view_wo_translation;
  const vec4 pos = global_mvp * vec4(in_pos, 1.0f);
  gl_Position = ForceDepthTo1(pos);
  vs_tex_coords = in_pos;
}
