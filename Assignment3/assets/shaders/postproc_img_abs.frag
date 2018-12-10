#version 440

uniform sampler2D screen_tex;

layout(location = 0) in vec2 vs_tex_coords;

layout(location = 0) out vec4 fs_color;

const float img_size = 600.0f;

vec4 CalcBlur() {
  const int half_size = 1;
  vec3 color_sum = vec3(0.0f);
  for (int i = -half_size; i <= half_size; ++i) {
    for (int j = -half_size; j <= half_size; ++j) {
      vec2 coord = vec2(vs_tex_coords) + vec2(i, j) / img_size;
      color_sum += vec3(texture(screen_tex, coord));
    }
  }
  float sample_count = (half_size * 2 + 1) * (half_size * 2 + 1);
  return vec4(color_sum / sample_count, 1.0f);
}

vec4 CalcQuantization() {
  const float nbins = 8.0f;
  const vec3 color = vec3(texture(screen_tex, vs_tex_coords));
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
      vec4 c = texture(screen_tex, vs_tex_coords + vec2(i, j) / img_size);
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

void main() {
  const vec4 color = CalcBlur() + CalcQuantization() + CalcDoG();
  const vec4 normalized = normalize(color);
  fs_color = vec4(vec3(normalized), 1.0f);
}
