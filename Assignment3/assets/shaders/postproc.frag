#version 440

/*******************************************************************************
 * Constants
 ******************************************************************************/

/* Display modes */
const int DISPLAY_MODE_ORIGINAL = 0;
const int DISPLAY_MODE_BAR = 1;
const int DISPLAY_MODE_POSTPROC = 2;
/* Post-processing effects */
const int POSTPROC_EFFECT_IMG_ABS = 0;
const int POSTPROC_EFFECT_LAPLACIAN = 1;
/* Display */
const float IMG_SIZE = 600.0f;
const float COMPARISON_BAR_WIDTH = 4;

/*******************************************************************************
 * Uniform Blocks
 ******************************************************************************/

/* Comparison bar */
uniform ComparisonBar {
  vec2 enabled;
  vec2 mouse_pos;
}
comparison_bar;

/* Post-processing inputs */
uniform PostprocInputs { int effect_idx; }
postproc_inputs;

/*******************************************************************************
 * Textures
 ******************************************************************************/

uniform sampler2D screen_tex;

/*******************************************************************************
 * Inputs
 ******************************************************************************/

layout(location = 0) in vec2 vs_tex_coords;

/*******************************************************************************
 * Outputs
 ******************************************************************************/

layout(location = 0) out vec4 fs_color;

/*******************************************************************************
 * Display Mode Handlers
 ******************************************************************************/

int CalcDisplayMode() {
  if (comparison_bar.enabled.x <= 0.0f) {
    return DISPLAY_MODE_ORIGINAL;
  }
  const vec2 dist = gl_FragCoord.xy - comparison_bar.mouse_pos;
  if (dist.x < -1.0f * COMPARISON_BAR_WIDTH / 2.0f) {
    return DISPLAY_MODE_ORIGINAL;
  } else if (dist.x > 1.0f * COMPARISON_BAR_WIDTH / 2.0f) {
    return DISPLAY_MODE_POSTPROC;
  } else {
    return DISPLAY_MODE_BAR;
  }
}

/*******************************************************************************
 * Texture Handlers
 ******************************************************************************/

vec4 GetTexel(vec2 coord) { return texture(screen_tex, coord); }

/*******************************************************************************
 * Post-processing / Image Abstraction
 ******************************************************************************/

vec4 CalcBlur() {
  const int half_size = 1;
  vec3 color_sum = vec3(0.0f);
  for (int i = -half_size; i <= half_size; ++i) {
    for (int j = -half_size; j <= half_size; ++j) {
      vec2 coord = vec2(vs_tex_coords) + vec2(i, j) / IMG_SIZE;
      color_sum += vec3(GetTexel(coord));
    }
  }
  float sample_count = (half_size * 2 + 1) * (half_size * 2 + 1);
  return vec4(color_sum / sample_count, 1.0f);
}

vec4 CalcQuantization() {
  const float nbins = 8.0f;
  const vec3 color = vec3(GetTexel(vs_tex_coords));
  const vec3 quantized = floor(color * nbins) / nbins;
  return vec4(quantized, 1.0f);
}

vec4 CalcDoG() {
  const float sigma_e = 2.0f;
  const float sigma_r = 2.8f;
  const float phi = 3.4f;
  const float tau = 0.99f;
  const float twoSigmaESquared = 2.0f * sigma_e * sigma_e;
  const float twoSigmaRSquared = 2.0f * sigma_r * sigma_r;
  const int halfWidth = int(ceil(2.0f * sigma_r));

  vec2 sum = vec2(0.0f);
  vec2 norm = vec2(0.0f);
  for (int i = -halfWidth; i <= halfWidth; ++i) {
    for (int j = -halfWidth; j <= halfWidth; ++j) {
      float d = length(vec2(i, j));
      vec2 kernel =
          vec2(exp(-d * d / twoSigmaESquared), exp(-d * d / twoSigmaRSquared));
      vec4 c = GetTexel(vs_tex_coords + vec2(i, j) / IMG_SIZE);
      vec2 L = vec2(0.299f * c.r + 0.587f * c.g + 0.114f * c.b);
      norm += 2.0f * kernel;
      sum += kernel * L;
    }
  }
  sum /= norm;
  const float H = 100.0f * (sum.x - tau * sum.y);
  const float edge =
      (H > 0.0f) ? 1.0f : 2.0f * smoothstep(-2.0f, 2.0f, phi * H);
  return vec4(edge, edge, edge, 1.0f);
}

/*******************************************************************************
 * Post-processing / Center
 ******************************************************************************/

vec4 CalcPostproc() {
  switch (postproc_inputs.effect_idx) {
    case POSTPROC_EFFECT_IMG_ABS: {
      const vec4 color = CalcBlur() + CalcQuantization() + CalcDoG();
      return normalize(color);
    } break;
    case POSTPROC_EFFECT_LAPLACIAN: {
      return CalcBlur();
    } break;
    default: { return GetTexel(vs_tex_coords); }
  }
}

/*******************************************************************************
 * Entry Point
 ******************************************************************************/

void main() {
  const int display_mode = CalcDisplayMode();
  switch (display_mode) {
    case DISPLAY_MODE_ORIGINAL: {
      fs_color = GetTexel(vs_tex_coords);
    } break;
    case DISPLAY_MODE_BAR: {
      fs_color = vec4(vec3(1.0f, 0.0f, 0.0f), 1.0f);
    } break;
    case DISPLAY_MODE_POSTPROC: {
      fs_color = CalcPostproc();
    } break;
  }
}
