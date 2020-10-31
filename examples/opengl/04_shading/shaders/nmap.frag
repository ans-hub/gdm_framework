#version 330

in vec2 ex_uv;
in vec3 ex_light_dir_TS;
in vec3 ex_view_dir_TS;

uniform vec3 light_col;
uniform sampler2D map_sampler;
uniform sampler2D tex_sampler;
uniform sampler2D spec_sampler;

out vec4 frag_color;

const float amb_intense = 0.15f;
const float inf_intense = 0.2f;
vec3 amb_color = vec3(light_col);
vec3 inf_color = vec3(light_col);
vec3 spec_color = 0.2f * vec3(light_col);

// All work here is doing in tangent space

void main()
{
  // Texture and normal map

  vec3 texel_color = texture(tex_sampler, ex_uv).rgb;
  vec3 normal_TS = texture(map_sampler, ex_uv).rgb;
  normal_TS = normalize(normal_TS * 2.f - 1.f);

  // Prepare tangent space variables

  vec3 light_dir_TS = normalize(ex_light_dir_TS);
  vec3 view_dir_TS = normalize(ex_view_dir_TS);

  // // For debug draw: cos normal & light_dir
  // float cos_theta = clamp(dot(normal_TS, light_dir_TS), 0.f, 1.f); 
  // frag_color = vec4(cos_theta, 0.f, 0.f, 1.f);

  // Ambient comp

  vec3 amb_total = texel_color * amb_color * amb_intense;

  // Diffuse comp for infinite lighting

  float cos_theta = clamp(dot(normal_TS, light_dir_TS), 0.f, 1.f); 
  vec3 inf_total = texel_color * inf_color * inf_intense * cos_theta;

  // Specular comp

  const int spec_pow = 32;
  vec3 reflect_dir = reflect(-light_dir_TS, normal_TS);
  float spec_mul = pow(max(dot(view_dir_TS, reflect_dir), 0.0), spec_pow);

  // Blinn-Phong specular model - currently smth going wrong

  // const int spec_pow = 32;
  // vec3 halfway_dir_TS = normalize(light_dir_TS + view_dir_TS);  
  // float spec_mul = pow(max(dot(normal_TS, halfway_dir_TS), 0.0), spec_pow);

  vec3 spec_total = spec_mul * spec_color * texture(spec_sampler, ex_uv).rgb;

  // Total

  frag_color = vec4(amb_total, 1.f) + vec4(inf_total, 1.f) + vec4(spec_total, 1.f);
}
