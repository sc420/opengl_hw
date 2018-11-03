#version 410

layout(location = 0) in vec3 iv3vertex;
layout(location = 1) in vec3 iv3color;

uniform mat4 um4mvp;

uniform mvp
{
    mat4 model;
    mat4 view;
    mat4 proj;
};

out vec3 vv3color;

void main()
{
    //gl_Position = um4mvp * vec4(iv3vertex, 1);
    gl_Position = proj * view * model * vec4(iv3vertex, 1);
    vv3color = iv3color;
}
