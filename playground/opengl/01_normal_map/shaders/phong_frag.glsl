#version 330

// Interpolated values from inital data in vertex shader

in vec2 ex_uv;
in vec4 ex_normal_CS;
in vec4 ex_normal_WS;
in vec4 ex_light_dir_CS;
in vec4 ex_light_dir_WS;
in vec4 ex_vx_pos_WS;     // i made it in world space just in educational purposes

uniform vec3 view_pos;
uniform vec3 light_col;
uniform vec3 light_pos;
uniform sampler2D tex_sampler;

out vec4 frag_color;

const float amb_intense = 0.15f;
vec4 amb_color = vec4(light_col, 1.f);
const float inf_intense = 0.5f;
vec4 inf_color = vec4(light_col, 1.f);

void main()
{
  vec4 texel_color = vec4(texture(tex_sampler, ex_uv).rgb, 1.f);

  // Ambient comp

  vec4 amb_total = texel_color * amb_color * amb_intense;

  // Diffuse comp for infinite lighting

  vec4 n = normalize(ex_normal_WS);
  vec4 l = normalize(vec4(light_pos, 0.f) - ex_vx_pos_WS );
  float cos_theta = clamp(dot(n,l), 0.f, 1.f); 
  vec4 inf_total = texel_color * inf_color * inf_intense * cos_theta;

  // Specular comp

  const float spec_intense = 0.5f;
  const int spec_pow = 32;
  vec4 view_dir = normalize(vec4(view_pos, 1.f) - ex_vx_pos_WS);
  vec4 reflect_dir = normalize(reflect(-l, ex_normal_WS));
  float spec_mul = pow(max(dot(view_dir, reflect_dir), 0.f), spec_pow);

  // Blinn-Phong

  // vec3 halfway_dir = normalize(vec3(l) + vec3(view_dir));  
  // float spec_mul = pow(max(dot(vec3(n), halfway_dir), 0.0), spec_pow);

  vec4 spec_total = spec_intense * spec_mul * inf_color;  

  // Total
  frag_color = amb_total + inf_total + spec_total;
}
