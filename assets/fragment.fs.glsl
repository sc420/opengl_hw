#version 410

in vec3 vv3color;

layout(location = 0) out vec4 frag_color;

void main()
{
  frag_color = vec4(vv3color, 1.0f);
}
