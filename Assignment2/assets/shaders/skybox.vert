#version 440

uniform GlobalMvp {
  mat4 model;
  mat4 view;
  mat4 proj;
}
global_mvp;

layout(location = 0) in vec3 in_pos;

layout(location = 0) out vec3 vs_tex_coords;

void main() {
  mat4 view_wo_translation = mat4(mat3(global_mvp.view));
  mat4 global_trans = global_mvp.proj * view_wo_translation;
  vec4 pos = global_trans * vec4(in_pos, 1.0f);
  gl_Position = pos;
  gl_Position.z = pos.w;
  vs_tex_coords = in_pos;
}
