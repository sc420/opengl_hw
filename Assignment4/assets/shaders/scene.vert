#version 440

/*******************************************************************************
 * Uniform Blocks
 ******************************************************************************/

uniform GlobalTrans {
  mat4 model;
  mat4 view;
  mat4 proj;
}
global_trans;

uniform ModelTrans { mat4 trans; }
model_trans;

/*******************************************************************************
 * Inputs
 ******************************************************************************/

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_norm;
layout(location = 2) in vec2 in_tex_coords;

/*******************************************************************************
 * Outputs
 ******************************************************************************/

layout(location = 0) out VSTex { vec2 coords; }
vs_tex;

layout(location = 1) out VSSurface {
  vec3 pos;
  vec3 norm;
}
vs_surface;

void main() {
  // Calculate MVP
  const mat4 global_mvp =
      global_trans.proj * global_trans.view * global_trans.model;
  // Calculate vertex position
  gl_Position = global_mvp * model_trans.trans * vec4(in_pos, 1.0f);
  // Pass texture coordinates
  vs_tex.coords = in_tex_coords;
  // Calculate fragment position
  vs_surface.pos =
      vec3(global_trans.model * model_trans.trans * vec4(in_pos, 1.0f));
  // Pass normal
  vs_surface.norm = in_norm;
}
