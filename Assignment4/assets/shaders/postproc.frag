#version 440

/*******************************************************************************
 * Constants
 ******************************************************************************/

/* Display modes */
const int kDisplayModeOriginal = 0;
const int kDisplayModeBar = 1;
const int kDisplayModePostproc = 2;
const int kDisplayModeMagnified = 3;
/* Post-processing effects */
const int kPostprocEffectImgAbs = 0;
const int kPostprocEffectLaplacian = 1;
const int kPostprocEffectSharpness = 2;
const int kPostprocEffectPixelation = 3;
const int kPostprocEffectBloomEffect = 4;
const int kPostprocEffectMagnifier = 5;
const int kPostprocEffectSpecial = 6;
/* Display */
const float kComparisonBarWidth = 4;
const float kMagnifierRadius = 0.2f;
/* Math */
const float kPi = 3.1415926535897932384626433832795;
/* Colors */
const vec4 kWhiteColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
const vec4 kBlackColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
const vec4 kErrorColor = vec4(1.0f, 0.0f, 1.0f, 1.0f);

/*******************************************************************************
 * Uniform Blocks
 ******************************************************************************/

/* Post-processing inputs */
layout(std140) uniform PostprocInputs {
  int enabled[2];
  vec2 mouse_pos;
  int effect_idx[2];
  int pass_idx[2];
  float time[2];
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

/*******************************************************************************
 * Texture Handlers
 ******************************************************************************/

vec4 GetTexel(vec2 coord) { return texture(screen_tex, coord); }

vec4 GetMultipassTexel(sampler2D tex, vec2 coord) {
  return texture(tex, coord);
}

vec2 GetTextureSize() { return textureSize(screen_tex, 0); }

/*******************************************************************************
 * Display Mode Handlers
 ******************************************************************************/

int CalcDisplayMode() {
  if (postproc_inputs.enabled[0] == 0) {
    return kDisplayModeOriginal;
  }
  const int effect_idx = postproc_inputs.effect_idx[0];
  const vec2 mouse_pos = postproc_inputs.mouse_pos;
  const vec2 window_size = GetTextureSize();
  const vec2 reversed_mouse_pos =
      vec2(mouse_pos.x, window_size.y - mouse_pos.y);
  const vec2 dist = vec2(gl_FragCoord) - reversed_mouse_pos;
  if (effect_idx == kPostprocEffectMagnifier) {
    const float radius = sqrt(pow(dist.x, 2.0f) + pow(dist.y, 2.0f));
    const float min_window_len = min(window_size.x, window_size.y);
    if (radius / min_window_len <= kMagnifierRadius) {
      return kDisplayModeMagnified;
    } else {
      return kDisplayModeOriginal;
    }
  } else {
    if (dist.x < -1.0f * kComparisonBarWidth / 2.0f) {
      return kDisplayModePostproc;
    } else if (dist.x > 1.0f * kComparisonBarWidth / 2.0f) {
      return kDisplayModeOriginal;
    } else {
      return kDisplayModeBar;
    }
  }
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
  const vec2 window_size = GetTextureSize();
  const int kHalfWidth = 1;
  const int kHalfHeight = 1;
  const mat3 norm_kernel = NormalizeKernel(kernel);
  vec4 sum = vec4(0.0f);
  for (int x = -1 * kHalfWidth; x <= kHalfWidth; x++) {
    for (int y = -1 * kHalfHeight; y <= kHalfHeight; y++) {
      const float kernel_val = norm_kernel[y + kHalfHeight][x + kHalfWidth];
      const vec2 ofs = vec2(x, y) / window_size;
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

  const vec2 window_size = GetTextureSize();
  vec2 sum = vec2(0.0f);
  vec2 norm = vec2(0.0f);
  for (int i = -halfWidth; i <= halfWidth; ++i) {
    for (int j = -halfWidth; j <= halfWidth; ++j) {
      float d = length(vec2(i, j));
      vec2 kernel =
          vec2(exp(-d * d / twoSigmaESquared), exp(-d * d / twoSigmaRSquared));
      vec4 c = GetTexel(vs_tex_coords + vec2(i, j) / window_size);
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
  const vec2 window_size = GetTextureSize();
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
  const vec2 window_size = GetTextureSize();
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

vec4 CalcBloomEffectMixedColor(vec4 orig_color, vec4 blurred_color) {
  return 0.3f * orig_color + 1.0f * blurred_color;
}

vec4 CalcBloomEffect() {
  const int kNumMultipass = 10;

  const int pass_idx = postproc_inputs.pass_idx[0];
  if (pass_idx == 0) {
    return 0.8f * GetTexel(vs_tex_coords) +
           0.5f * CalcBrightness(GetTexel(vs_tex_coords));
  } else if (pass_idx < 1 + kNumMultipass * 2) {
    const bool horizontal = (pass_idx % 2 == 0);
    if ((pass_idx + 1) % 2 == 0) {
      return CalcGaussianBlur(multipass_tex1, horizontal);
    } else {
      return CalcGaussianBlur(multipass_tex2, horizontal);
    }
  } else {
    const vec4 orig_color = GetTexel(vs_tex_coords);
    const vec4 blurred_color = GetMultipassTexel(multipass_tex1, vs_tex_coords);
    return CalcBloomEffectMixedColor(orig_color, blurred_color);
  }
}

/*******************************************************************************
 * Post-processing / Magnifier
 ******************************************************************************/

vec4 CalcMagnifier() {
  const float kMagnifyFactor = 2.0f;

  const vec2 window_size = GetTextureSize();
  const vec2 mouse_pos = postproc_inputs.mouse_pos;
  const vec2 center =
      vec2(mouse_pos.x, window_size.y - mouse_pos.y) / window_size;
  const vec2 dist = vs_tex_coords - center;
  const vec2 magnified_coords = center + dist / kMagnifyFactor;
  return GetTexel(magnified_coords);
}

/*******************************************************************************
 * Post-processing / Special
 ******************************************************************************/

float sat(float t) { return clamp(t, 0.0, 1.0); }

vec2 sat(vec2 t) { return clamp(t, 0.0, 1.0); }

// remaps inteval [a;b] to [0;1]
float remap(float t, float a, float b) { return sat((t - a) / (b - a)); }

// note: /\ t=[0;0.5;1], y=[0;1;0]
float linterp(float t) { return sat(1.0 - abs(2.0 * t - 1.0)); }

vec3 spectrum_offset(float t) {
  float t0 = 3.0 * t - 1.5;
  return clamp(vec3(-t0, 1.0 - abs(t0), t0), 0.0, 1.0);
  /*
      vec3 ret;
      float lo = step(t,0.5);
      float hi = 1.0-lo;
      float w = linterp( remap( t, 1.0/6.0, 5.0/6.0 ) );
      float neg_w = 1.0-w;
      ret = vec3(lo,1.0,hi) * vec3(neg_w, w, neg_w);
      return pow( ret, vec3(1.0/2.2) );
*/
}

// note: [0;1]
float rand(vec2 n) {
  return fract(sin(dot(n.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

// note: [-1;1]
float srand(vec2 n) { return rand(n) * 2.0 - 1.0; }

float mytrunc(float x, float num_levels) {
  return floor(x * num_levels) / num_levels;
}

vec2 mytrunc(vec2 x, float num_levels) {
  return floor(x * num_levels) / num_levels;
}

vec2 mytrunc(vec2 x, vec2 num_levels) {
  return floor(x * num_levels) / num_levels;
}

vec3 rgb2yuv(vec3 rgb) {
  vec3 yuv;
  yuv.x = dot(rgb, vec3(0.299, 0.587, 0.114));
  yuv.y = dot(rgb, vec3(-0.14713, -0.28886, 0.436));
  yuv.z = dot(rgb, vec3(0.615, -0.51499, -0.10001));
  return yuv;
}

vec3 yuv2rgb(vec3 yuv) {
  vec3 rgb;
  rgb.r = yuv.x + yuv.z * 1.13983;
  rgb.g = yuv.x + dot(vec2(-0.39465, -0.58060), yuv.yz);
  rgb.b = yuv.x + yuv.y * 2.03211;
  return rgb;
}

float noise(vec2 p) {
  vec2 ip = floor(p);
  vec2 u = fract(p);
  u = u * u * (3.0 - 2.0 * u);

  float res =
      mix(mix(rand(ip), rand(ip + vec2(1.0, 0.0)), u.x),
          mix(rand(ip + vec2(0.0, 1.0)), rand(ip + vec2(1.0, 1.0)), u.x), u.y);
  return res * res;
}

vec3 rgb2hsv(vec3 c) {
  vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
  vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
  vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

  float d = q.x - min(q.w, q.y);
  float e = 1.0e-10;
  return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c) {
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 posterize(vec3 color, float steps) { return floor(color * steps) / steps; }

float quantize(float n, float steps) { return floor(n * steps) / steps; }

vec4 downsample(sampler2D sampler, vec2 uv, float pixelSize) {
  const vec2 iResolution = GetTextureSize();

  return texture(sampler, uv - mod(uv, vec2(pixelSize) / iResolution));
}

vec3 edge(sampler2D sampler, vec2 uv, float sampleSize) {
  const vec2 iResolution = GetTextureSize();
  const float iTime = postproc_inputs.time[0];

  float dx = sampleSize / iResolution.x;
  float dy = sampleSize / iResolution.y;
  return (mix(downsample(sampler, uv - vec2(dx, 0.0), sampleSize),
              downsample(sampler, uv + vec2(dx, 0.0), sampleSize),
              mod(uv.x, dx) / dx) +
          mix(downsample(sampler, uv - vec2(0.0, dy), sampleSize),
              downsample(sampler, uv + vec2(0.0, dy), sampleSize),
              mod(uv.y, dy) / dy))
                 .rgb /
             2.0 -
         texture(sampler, uv).rgb;
}

float rand(float n) { return fract(sin(n) * 43758.5453123); }

float noise(float p) {
  float fl = floor(p);
  float fc = fract(p);
  return mix(rand(fl), rand(fl + 1.0), fc);
}

vec3 distort(sampler2D sampler, vec2 uv, float edgeSize) {
  const float kTileSize = 16;

  const vec2 iResolution = GetTextureSize();
  const float iTime = postproc_inputs.time[0];
  const float Amount = 0.1f + 0.5f * abs(sin(iTime));

  vec2 pixel = vec2(1.0) / iResolution;
  vec3 field = rgb2hsv(edge(sampler, uv, edgeSize));
  vec2 distort = pixel * sin((field.rb) * kPi * 2.0);
  float shiftx =
      noise(vec2(quantize(uv.y + 31.5, iResolution.y / kTileSize) * iTime,
                 fract(iTime) * 300.0));
  float shifty =
      noise(vec2(quantize(uv.x + 11.5, iResolution.x / kTileSize) * iTime,
                 fract(iTime) * 100.0));
  vec3 rgb = texture(sampler, uv + (distort + (pixel - pixel / 2.0) *
                                                  vec2(shiftx, shifty) *
                                                  (50.0 + 100.0 * Amount)) *
                                       Amount)
                 .rgb;
  vec3 hsv = rgb2hsv(rgb);
  hsv.y = mod(hsv.y + shifty * pow(Amount, 5.0) * 0.25, 1.0);
  return posterize(
      hsv2rgb(hsv),
      floor(mix(256.0, pow(1.0 - hsv.z - 0.5, 2.0) * 64.0 * shiftx + 4.0,
                1.0 - pow(1.0 - Amount, 5.0))));
}

// Reference: https://www.shadertoy.com/view/MdfGD2
vec4 ShampainGlitch01(vec2 fragCoord) {
  vec4 fragColor;
  const vec2 iResolution = GetTextureSize();
  const float iTime = postproc_inputs.time[0];
  const vec4 iMouse = vec4(postproc_inputs.mouse_pos, 0.0f, 0.0f);

  float THRESHOLD = 0.1f + abs(sin(iTime)) * 0.5f;
  float time_s = mod(iTime, 32.0);

  float glitch_threshold = 1.0 - THRESHOLD;
  const float max_ofs_siz = 0.05;
  const float yuv_threshold = 0.9;
  const float time_frq = 16.0;

  vec2 uv = fragCoord.xy / iResolution;
  //  uv.y = 1.0 - uv.y;

  const float min_change_frq = 4.0;
  float ct = mytrunc(time_s, min_change_frq);
  float change_rnd = rand(mytrunc(uv.yy, vec2(16)) + 150.0 * ct);

  float tf = time_frq * change_rnd;

  float t = 5.0 * mytrunc(time_s, tf);
  float vt_rnd = 0.5 * rand(mytrunc(uv.yy + t, vec2(11)));
  vt_rnd += 0.5 * rand(mytrunc(uv.yy + t, vec2(7)));
  vt_rnd = vt_rnd * 2.0 - 1.0;
  vt_rnd = sign(vt_rnd) *
           sat((abs(vt_rnd) - glitch_threshold) / (1.0 - glitch_threshold));

  vec2 uv_nm = uv;
  uv_nm = sat(uv_nm + vec2(max_ofs_siz * vt_rnd, 0));

  float rnd = rand(vec2(mytrunc(time_s, 8.0)));
  uv_nm.y = (rnd > mix(1.0, 0.975, sat(THRESHOLD))) ? 1.0 - uv_nm.y : uv_nm.y;

  vec4 smpl = GetTexel(uv_nm);
  vec3 smpl_yuv = rgb2yuv(smpl.rgb);
  smpl_yuv.y /= 1.0 - 3.0 * abs(vt_rnd) * sat(yuv_threshold - vt_rnd);
  smpl_yuv.z += 0.125 * vt_rnd * sat(vt_rnd - yuv_threshold);
  fragColor = vec4(yuv2rgb(smpl_yuv), smpl.a);
  return fragColor;
}

// Reference: https://www.shadertoy.com/view/lsfGD2
vec4 ShampainGlitch02(vec2 fragCoord) {
  vec4 fragColor;
  const vec2 iResolution = GetTextureSize();
  const float iTime = postproc_inputs.time[0];

  float aspect = iResolution.x / iResolution.y;
  vec2 uv = fragCoord.xy / iResolution;
  // uv.y = 1.0 - uv.y;

  float time = mod(iTime, 32.0);  // + modelmat[0].x + modelmat[0].z;

  float GLITCH = 0.01f + 0.2f * abs(sin(iTime));

  // float rdist = length( (uv - vec2(0.5,0.5))*vec2(aspect, 1.0) )/1.4;
  // GLITCH *= rdist;

  float gnm = sat(GLITCH);
  float rnd0 = rand(mytrunc(vec2(time, time), 6.0));
  float r0 = sat((1.0 - gnm) * 0.7 + rnd0);
  float rnd1 = rand(vec2(mytrunc(uv.x, 10.0 * r0), time));  // horz
  // float r1 = 1.0f - sat( (1.0f-gnm)*0.5f + rnd1 );
  float r1 = 0.5 - 0.5 * gnm + rnd1;
  r1 = 1.0 - max(0.0, ((r1 < 1.0)
                           ? r1
                           : 0.9999999));  // note: weird ass bug on old drivers
  float rnd2 = rand(vec2(mytrunc(uv.y, 40.0 * r1), time));  // vert
  float r2 = sat(rnd2);

  float rnd3 = rand(vec2(mytrunc(uv.y, 10.0 * r0), time));
  float r3 = (1.0 - sat(rnd3 + 0.8)) - 0.1;

  float pxrnd = rand(uv + time);

  float ofs = 0.05 * r2 * GLITCH * (rnd0 > 0.5 ? 1.0 : -1.0);
  ofs += 0.5 * pxrnd * ofs;

  uv.y += 0.1 * r3 * GLITCH;

  const int NUM_SAMPLES = 10;
  const float RCP_NUM_SAMPLES_F = 1.0 / float(NUM_SAMPLES);

  vec4 sum = vec4(0.0);
  vec3 wsum = vec3(0.0);
  for (int i = 0; i < NUM_SAMPLES; ++i) {
    float t = float(i) * RCP_NUM_SAMPLES_F;
    uv.x = sat(uv.x + ofs * t);
    vec4 samplecol = GetTexel(uv);
    vec3 s = spectrum_offset(t);
    samplecol.rgb = samplecol.rgb * s;
    sum += samplecol;
    wsum += s;
  }
  sum.rgb /= wsum;
  sum.a *= RCP_NUM_SAMPLES_F;

  // fragColor = vec4( sum.bbb, 1.0 ); return;

  fragColor.a = sum.a;
  fragColor.rgb = sum.rgb;  // * outcol0.a;
  return fragColor;
}

// Reference: https://www.shadertoy.com/view/4dtGzl
vec4 GlitchShaderB(vec2 fragCoord) {
  vec4 fragColor;

  const vec2 iResolution = GetTextureSize();
  const float iTime = postproc_inputs.time[0];

  vec2 uv = fragCoord.xy / iResolution;

  float wow = clamp(mod(noise(iTime + uv.y), 1.0), 0.0, 1.0) * 2.0 - 1.0;
  vec3 finalColor;
  finalColor += distort(screen_tex, uv, 4.0);
  fragColor = vec4(finalColor, 1.0);

  return fragColor;
}

vec4 CalcSpecial() {
  const vec2 window_size = GetTextureSize();
  const vec2 window_tex_coords = vs_tex_coords * window_size;
  return 0.4f * ShampainGlitch01(window_tex_coords) +
         0.4f * ShampainGlitch02(window_tex_coords) +
         0.2f * GlitchShaderB(window_tex_coords);
}

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
    case kPostprocEffectSpecial: {
      return CalcSpecial();
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
    case kDisplayModeMagnified: {
      fs_color = CalcPostproc();
    } break;
    default: { fs_color = kErrorColor; }
  }
}
