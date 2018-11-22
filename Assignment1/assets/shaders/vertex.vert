#version 440

uniform GlobalMvp {
  mat4 model;
  mat4 view;
  mat4 proj;
}
global_mvp;

uniform ModelTrans {
  mat4 trans;
  vec3 color;
}
model_trans;

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_tex_coord;

layout(location = 0) out vec2 vs_tex_coord;
layout(location = 1) out vec3 vs_color;

void main() {
  mat4 global_trans = global_mvp.proj * global_mvp.view * global_mvp.model;
  mat4 trans = global_trans * model_trans.trans;
  gl_Position = trans * vec4(in_pos, 1.0f);
  vs_tex_coord = in_tex_coord;
  vs_color = model_trans.color;
}
