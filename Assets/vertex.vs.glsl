#version 410

uniform mvp
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

layout(location = 0) in vec3 iv3vertex;
layout(location = 1) in vec3 iv3color;

out vec3 vv3color;

void main()
{
  gl_Position = proj * view * model * vec4(iv3vertex, 1);
  vv3color = iv3color;
}
