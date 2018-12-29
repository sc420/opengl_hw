#version 440

/*******************************************************************************
 * Textures
 ******************************************************************************/

uniform sampler2D obj_tex;
uniform sampler2D no_obj_tex;
uniform sampler2D bg_tex;

/*******************************************************************************
 * Inputs
 ******************************************************************************/

layout(location = 0) in vec2 vs_tex_coords;

/*******************************************************************************
 * Outputs
 ******************************************************************************/

layout(location = 0) out vec4 fs_color;

/*******************************************************************************
 * Entry Point
 ******************************************************************************/

void main() {
  const vec4 obj_color = texture(obj_tex, vs_tex_coords);
  const vec4 no_obj_color = texture(no_obj_tex, vs_tex_coords);
  const vec4 bg_color = texture(bg_tex, vs_tex_coords);
  fs_color = bg_color + (obj_color - no_obj_color);
}
