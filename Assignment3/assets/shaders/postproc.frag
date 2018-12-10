#version 440

/*******************************************************************************
 * Constants
 ******************************************************************************/

/* Display modes */
const int kDisplayModeOriginal = 0;
const int kDisplayModeBar = 1;
const int kDisplayModePostproc = 2;
/* Post-processing effects */
const int kPostprocEffectImgAbs = 0;
const int kPostprocEffectLaplacian = 1;
const int kPostprocEffectSharpness = 2;
const int kPostprocEffectPixelation = 3;
const int kPostprocEffectBloomEffect = 4;
const int kPostprocEffectMagnifier = 5;
/* Display */
const float kComparisonBarWidth = 4;
/* Colors */
const vec4 kWhiteColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
const vec4 kBlackColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
const vec4 kErrorColor = vec4(1.0f, 0.0f, 1.0f, 1.0f);

/*******************************************************************************
 * Uniform Blocks
 ******************************************************************************/

/* Comparison bar */
uniform ComparisonBar {
  int enabled[2];
  vec2 mouse_pos;
}
comparison_bar;

/* Post-processing inputs */
uniform PostprocInputs {
  int effect_idx[2];
  vec2 window_size;
  int pass_idx[2];
}
postproc_inputs;

/*******************************************************************************
 * Textures
 ******************************************************************************/

uniform sampler2D screen_tex;
uniform sampler2D multipass_tex1;
uniform sampler2D multipass_tex2;

/*******************************************************************************
 * Inputs
 ******************************************************************************/

layout(location = 0) in vec2 vs_tex_coords;

/*******************************************************************************
 * Outputs
 ******************************************************************************/

layout(location = 0) out vec4 fs_color;
layout(location = 1) out vec4 fs_multipass_color;

/*******************************************************************************
 * Display Mode Handlers
 ******************************************************************************/

int CalcDisplayMode() {
  if (comparison_bar.enabled[0] == 0) {
    return kDisplayModeOriginal;
  }
  const vec2 dist = vec2(gl_FragCoord) - comparison_bar.mouse_pos;
  if (dist.x < -1.0f * kComparisonBarWidth / 2.0f) {
    return kDisplayModePostproc;
  } else if (dist.x > 1.0f * kComparisonBarWidth / 2.0f) {
    return kDisplayModeOriginal;
  } else {
    return kDisplayModeBar;
  }
}

/*******************************************************************************
 * Texture Handlers
 ******************************************************************************/

vec4 GetTexel(vec2 coord) { return texture(screen_tex, coord); }

vec4 GetMultipassTexel(sampler2D tex, vec2 coord) {
  return texture(tex, coord);
}

/*******************************************************************************
 * Post-processing / Image Processing
 ******************************************************************************/

mat3 NormalizeKernel(mat3 kernel) {
  const int kHalfWidth = 1;
  const int kHalfHeight = 1;
  float sum = 0.0f;
  for (int x = -1 * kHalfWidth; x <= kHalfWidth; x++) {
    for (int y = -1 * kHalfHeight; y <= kHalfHeight; y++) {
      sum += kernel[y + kHalfHeight][x + kHalfWidth];
    }
  }
  if (sum > 0.0f || sum < 0.0f) {
    return kernel / sum;
  } else {
    return kernel;
  }
}

vec4 ApplyKernel(mat3 kernel) {
  const int kHalfWidth = 1;
  const int kHalfHeight = 1;
  const mat3 norm_kernel = NormalizeKernel(kernel);
  vec4 sum = vec4(0.0f);
  for (int x = -1 * kHalfWidth; x <= kHalfWidth; x++) {
    for (int y = -1 * kHalfHeight; y <= kHalfHeight; y++) {
      const float kernel_val = norm_kernel[y + kHalfHeight][x + kHalfWidth];
      const vec2 ofs = vec2(x, y) / postproc_inputs.window_size;
      sum += kernel_val * GetTexel(vs_tex_coords + ofs);
    }
  }
  return sum;
}

/*******************************************************************************
 * Post-processing / Image Abstraction
 ******************************************************************************/

vec4 CalcBlur() {
  const mat3 kKernel = mat3(
      // 1st row
      1.0f, 1.0f, 1.0f,
      // 2nd row
      1.0f, 1.0f, 1.0f,
      // 3rd row
      1.0f, 1.0f, 1.0f);

  return ApplyKernel(kKernel);
}

vec4 CalcQuantization() {
  const float kNumColors = 8.0f;

  const vec4 color = GetTexel(vs_tex_coords);
  const vec4 quantized = floor(color * kNumColors) / kNumColors;
  return vec4(vec3(quantized), 1.0f);
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
      vec4 c =
          GetTexel(vs_tex_coords + vec2(i, j) / postproc_inputs.window_size);
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

vec4 CalcImageAbstraction() {
  const vec4 blur_quantized =
      clamp(CalcBlur() + CalcQuantization(), 0.0f, 1.0f);
  const vec4 color = mix(kWhiteColor, blur_quantized, CalcDoG());
  return color;
}

/*******************************************************************************
 * Post-processing / Laplacian Filter
 ******************************************************************************/

vec4 CalcLaplacian() {
  const mat3 kKernel = mat3(
      // 1st row
      -1.0f, -1.0f, -1.0f,
      // 2nd row
      -1.0f, 9.0f, -1.0f,
      // 3rd row
      -1.0f, -1.0f, -1.0f);
  const float kThresh = 1.0f;

  const vec4 color = ApplyKernel(kKernel);
  const float avg = (color.x + color.y + color.z) / 3.0f;
  if (avg < kThresh) {
    return kBlackColor;
  } else {
    return kWhiteColor;
  }
}

/*******************************************************************************
 * Post-processing / Sharpness Filter
 ******************************************************************************/

vec4 CalcSharpness() {
  const mat3 kKernel = mat3(
      // 1st row
      -1.0f, -1.0f, -1.0f,
      // 2nd row
      -1.0f, 9.0f, -1.0f,
      // 3rd row
      -1.0f, -1.0f, -1.0f);

  return ApplyKernel(kKernel);
}

/*******************************************************************************
 * Post-processing / Pixelation
 ******************************************************************************/

vec4 CalcPixelation() {
  const float kCellWidth = 8.0f;
  const float kCellHeight = 8.0f;

  const float texel_size = kCellWidth * kCellHeight;
  const vec2 cell_size = vec2(kCellWidth, kCellHeight);
  const vec2 window_size = postproc_inputs.window_size;
  const vec2 screen_coords = vs_tex_coords * window_size;
  const vec2 low_pos = floor(screen_coords / cell_size) * cell_size;
  const vec2 high_pos = low_pos + cell_size;
  vec4 sum = vec4(0.0f);
  for (float x = low_pos.x; x < high_pos.x; x++) {
    for (float y = low_pos.y; y < high_pos.y; y++) {
      sum += GetTexel(vec2(x, y) / window_size);
    }
  }
  sum /= texel_size;
  return vec4(vec3(sum), 1.0f);
}

/*******************************************************************************
 * Post-processing / Bloom Effect
 ******************************************************************************/

vec4 CalcBrightness(vec4 color) {
  // References:
  // https://learnopengl.com/Advanced-Lighting/Bloom
  // https://en.wikipedia.org/wiki/Relative_luminance
  const float kThresh = 0.4f;

  const float luminance = dot(vec3(color), vec3(0.2126f, 0.7152f, 0.0722f));
  if (luminance > kThresh) {
    return color;
  } else {
    return kBlackColor;
  }
}

vec4 CalcGaussianBlur(sampler2D tex, bool horizontal) {
  const int len = 5;
  const float weight[len] =
      float[](0.227027f, 0.1945946f, 0.1216216f, 0.054054f, 0.016216f);
  vec2 window_size = postproc_inputs.window_size;
  vec4 sum = weight[0] * GetMultipassTexel(tex, vs_tex_coords);
  if (horizontal) {
    for (int x = 1; x < len; x++) {
      const vec2 ofs = vec2(x, 0.0f) / window_size;
      sum += weight[x] * GetMultipassTexel(tex, vs_tex_coords + ofs);
      sum += weight[x] * GetMultipassTexel(tex, vs_tex_coords - ofs);
    }
  } else {
    for (int y = 1; y < len; y++) {
      const vec2 ofs = vec2(0.0f, y) / window_size;
      sum += weight[y] * GetMultipassTexel(tex, vs_tex_coords + ofs);
      sum += weight[y] * GetMultipassTexel(tex, vs_tex_coords - ofs);
    }
  }
  return vec4(vec3(sum), 1.0f);
}

vec4 CalcBloomEffect() {
  const int kNumMultipass = 5;
  const float kExposure = 1.0f;
  const float kGamma = 1.0f;

  const int pass_idx = postproc_inputs.pass_idx[0];
  if (pass_idx == 0) {
    fs_color = GetTexel(vs_tex_coords);
  } else if (pass_idx < 1 + kNumMultipass * 2) {
    if ((pass_idx + 1) % 2 == 0) {
      const bool horizontal = (pass_idx % 2 == 0);
      fs_color = CalcGaussianBlur(multipass_tex1, horizontal);
    } else {
      const bool horizontal = (pass_idx % 2 == 0);
      fs_color = CalcGaussianBlur(multipass_tex2, horizontal);
    }
  } else {
    const bool horizontal = (pass_idx % 2 == 0);
    fs_color = GetMultipassTexel(multipass_tex1, vs_tex_coords);
  }
  return fs_color;
}

/*******************************************************************************
 * Post-processing / Magnifier
 ******************************************************************************/

vec4 CalcMagnifier() { return kErrorColor; }

/*******************************************************************************
 * Post-processing / Center
 ******************************************************************************/

vec4 CalcPostproc() {
  int effect_idx = postproc_inputs.effect_idx[0];
  switch (effect_idx) {
    case kPostprocEffectImgAbs: {
      return CalcImageAbstraction();
    } break;
    case kPostprocEffectLaplacian: {
      return CalcLaplacian();
    } break;
    case kPostprocEffectSharpness: {
      return CalcSharpness();
    } break;
    case kPostprocEffectPixelation: {
      return CalcPixelation();
    } break;
    case kPostprocEffectBloomEffect: {
      return CalcBloomEffect();
    } break;
    case kPostprocEffectMagnifier: {
      return CalcMagnifier();
    } break;
    default: { return kErrorColor; }
  }
}

/*******************************************************************************
 * Entry Point
 ******************************************************************************/

void main() {
  const int display_mode = CalcDisplayMode();
  switch (display_mode) {
    case kDisplayModePostproc: {
      fs_color = CalcPostproc();
    } break;
    case kDisplayModeOriginal: {
      fs_color = GetTexel(vs_tex_coords);
    } break;
    case kDisplayModeBar: {
      fs_color = vec4(vec3(1.0f, 0.0f, 0.0f), 1.0f);
    } break;
    default: { fs_color = kErrorColor; }
  }
}
