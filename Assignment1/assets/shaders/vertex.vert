#version 440

uniform GlobalMvp {
  mat4 model;
  mat4 view;
  mat4 proj;
} global_mvp;

uniform ModelTrans {
  mat4 trans;
  vec3 color;
} model_trans;

layout(location = 0) in vec3 in_vertex;
layout(location = 1) in vec3 in_color;

out vec3 vs_color;

void main() {
  mat4 global_trans = global_mvp.proj * global_mvp.view * global_mvp.model;
  mat4 trans = global_trans * model_trans.trans;
  gl_Position = trans * vec4(in_vertex, 1.0f);
  vs_color = (in_color + model_trans.color) / 2.0f;
}
