#define DIR_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2
#define lights_COUNT 4

SamplerState sampler_gen : register(s1);
Texture2D gbuffer_pos : register(t2);
Texture2D gbuffer_diff : register(t3);
Texture2D gbuffer_norm : register(t4);

struct LightProps
{
  float4 pos_WS;
  float3 dir_WS;
  float padding0;
  float4 color_;
  float spot_angle_;
  float attenuation_const_;
  float attenuation_linear_;
  float attenuation_quadr_;
  int type_;
  int enabled_;
  float2 padding1;
};

cbuffer LightPropsBuffer : register(b0)
{
  float4 g_global_ambient_;
  float4 g_camera_pos_;
  LightProps g_lights_[lights_COUNT];
};

struct VSOutput
{
  float2 UV : TEXUV;
	float4 Pos : SV_POSITION;
};

float4 main(VSOutput input) : SV_TARGET
{
	float3 pixel_diff = gbuffer_diff.Sample(sampler_gen, input.UV).xyz;
	float3 pos_WS = gbuffer_pos.Sample(sampler_gen, input.UV).xyz;
	float3 normal_WS = gbuffer_norm.Sample(sampler_gen, input.UV).xyz;

  float4 diff_total = {0,0,0,0};
  float4 spec_total = {0,0,0,0};

  float spec_pow_from_material = gbuffer_pos.Sample(sampler_gen, input.UV).w;
  float emissive_from_material = gbuffer_norm.Sample(sampler_gen, input.UV).w;

  for(int i = 0; i < lights_COUNT; ++i)
  {
    if (!g_lights_[i].enabled_)
      continue;

    float4 diff_curr = {0,0,0,0};
    float4 spec_curr = {0,0,0,0};

    switch(g_lights_[i].type_)
    {
      case DIR_LIGHT:
      {
        float3 LD = -normalize(g_lights_[i].dir_WS);
        float3 N = normalize(normal_WS.xyz);
        float3 R = normalize(reflect(-LD,N));
        float3 V = normalize(g_camera_pos_.xyz - pos_WS.xyz);
        int visibility = max(0,dot(LD,N)) + 0.999f;
        float diff_dot = max(0.f, dot(LD,N));
        diff_curr = g_lights_[i].color_ * diff_dot;
        // float spec_dot = max(0.f, dot(R,V));
        // spec_curr = g_lights_[i].color_ * pow(spec_dot, spec_pow_from_material) * visibility;
        break;
      }
      case POINT_LIGHT:
      {
        float3 LD = g_lights_[i].pos_WS.xyz - pos_WS.xyz;
        float3 N = normalize(normal_WS.xyz);
        float dist = length(LD);
        LD = normalize(LD);
        float3 R = normalize(reflect(-LD,N));
        float3 V = normalize(g_camera_pos_.xyz - pos_WS.xyz);
        int visibility = max(0,dot(LD,N)) + 0.999f;
        float diff_dot = max(0.f, dot(LD,N));
        diff_curr = g_lights_[i].color_ * diff_dot;
        // float spec_dot = max(0.f, dot(R,V));
        // spec_curr = g_lights_[i].color_ * pow(spec_dot, spec_pow_from_material) * visibility;
        break;
      }
    }
    diff_total += diff_curr;
    spec_total += spec_curr;
  }

  diff_total = saturate(diff_total);
  spec_total = saturate(spec_total);

  // float4 spec_map = SpecularMap.Sample(Sampler, IN.texuv_TS);
  // int has_spec_own = clamp(0, 1, ceil(length(material_props_.specular_)));
  // int has_spec_map = !has_spec_own;
  // float4 specular = spec_map * has_spec_map * spec_total + material_props_.specular_ * has_spec_own * spec_total;
  float4 emissive = 0; //emissive_from_material;
  float4 ambient = g_global_ambient_;
  float4 diffuse = diff_total;//material_props_.diffuse_ * diff_total;
	float4 specular = spec_total;
  float4 tex_color = float4(pixel_diff, 1.f);

  // return tex_color;
  // return float4(normal_WS, 1.f);
  if (length(emissive) == 0)
    return (ambient + diffuse) * tex_color;
  else
    return (emissive);
}