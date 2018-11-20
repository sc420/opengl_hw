#version 440

uniform global_mvp
{
  mat4 model;
  mat4 view;
  mat4 proj;
};

uniform obj_trans
{
  mat4 trans;
  vec3 color;
};

layout(location = 0) in vec3 in_vertex;
layout(location = 1) in vec3 in_color;

out vec3 vs_color;

void main()
{
  gl_Position = proj * view * model * trans * vec4(in_vertex, 1.0f);
  vs_color = (in_color + color) / 2.0f;
}
