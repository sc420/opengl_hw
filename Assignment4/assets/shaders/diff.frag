#version 440

/*******************************************************************************
 * Uniform Blocks
 ******************************************************************************/

layout(std140) uniform Diff { int display_mode; }
diff;

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
  switch (diff.display_mode) {
    case 0: {
      fs_color = bg_color + (obj_color - no_obj_color);
    } break;
    case 1: {
      fs_color = obj_color;
    } break;
    case 2: {
      fs_color = no_obj_color;
    } break;
    case 3: {
      fs_color = bg_color;
    } break;
    default: { fs_color = vec4(1.0f, 0.0f, 1.0f, 1.0f); }
  }
}
