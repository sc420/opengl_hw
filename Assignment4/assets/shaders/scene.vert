#version 440

uniform GlobalMvp {
  mat4 model;
  mat4 view;
  mat4 proj;
}
global_mvp;

uniform ModelTrans { mat4 trans; }
model_trans;

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_norm;
layout(location = 2) in vec2 in_tex_coords;

layout(location = 0) out vec2 vs_tex_coords;

void main() {
  const mat4 global_trans =
      global_mvp.proj * global_mvp.view * global_mvp.model;
  const mat4 trans = global_trans * model_trans.trans;
  gl_Position = trans * vec4(in_pos, 1.0f);
  vs_tex_coords = in_tex_coords;
}
