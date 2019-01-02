#version 440

layout(std140) uniform GlobalTrans {
  mat4 model;
  mat4 view;
  mat4 proj;
}
global_trans;

layout(std140) uniform ModelTrans { mat4 trans; }
model_trans;

layout(location = 0) in vec3 in_pos;

mat4 CalcTrans() {
  return global_trans.proj * global_trans.view * global_trans.model *
         model_trans.trans;
}

void main() { gl_Position = CalcTrans() * vec4(in_pos, 1.0f); }
