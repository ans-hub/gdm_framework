// Shader uses blinn-phong shading

#define LIGHTS_COUNT 4

#define DIR_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

sampler Sampler : register(s0);

Texture2D DiffuseMap : register(t0);
Texture2D NormalMap : register(t1);
Texture2D SpecularMap : register(t2);

struct MaterialProps
{
  float4 emissive_;
  float4 ambient_;
  float4 diffuse_;
  float4 specular_;
  float specular_power_;

}; // struct MaterialProps

struct LightProps
{
  float4 pos_WS;
  float4 dir_WS;
  float4 color_;
  float spot_angle_;
  float attenuation_const_;
  float attenuation_linear_;
  float attenuation_quadr_;
  int type_;
  int enabled_;
  float2 padding_;

}; // struct LightProps

cbuffer MaterialPropsBuffer : register(b0)
{
  MaterialProps material_props_;
};

cbuffer LightPropsBuffer : register(b1)
{
  float4 g_global_ambient_;
  float4 g_camera_pos_;
  LightProps g_lights_[LIGHTS_COUNT];
};

struct Input
{
  float4 position   : SV_Position;
  float4 pos_WS     : TEXCOORD0;
  float4 normal_WS  : TEXCOORD1;
  float2 texuv_TS   : TEXCOORD2;
  float4 cam_pos_WS : TEXCOORD3;
};

float4 main(Input IN) : SV_TARGET
{
  float4 diff_total = {0,0,0,0};
  float4 spec_total = {0,0,0,0};

  [unroll] for(int i = 0; i < LIGHTS_COUNT; ++i)
  {
    if (!g_lights_[i].enabled_)
      continue;

    float4 diff_curr = {0,0,0,0};
    float4 spec_curr = {0,0,0,0};
    
    switch(g_lights_[i].type_)
    {
      case DIR_LIGHT:
      {
        float3 LD = normalize(g_lights_[i].dir_WS.xyz);
        float3 N = normalize(IN.normal_WS.xyz);
        float3 R = normalize(reflect(-LD,N));
        float3 V = normalize(g_camera_pos_.xyz - IN.pos_WS.xyz);
        int visibility = max(0,dot(LD,N)) + 0.999f;

        float diff_dot = max(0.f, dot(LD,N));
        diff_curr = g_lights_[i].color_ * diff_dot;
       
        float spec_dot = max(0.f, dot(R,V));
        float spec_pow = material_props_.specular_power_;
        spec_curr = g_lights_[i].color_ * pow(spec_dot, spec_pow) * visibility;
        
        break;
      }
      case POINT_LIGHT:
      {
        float3 LD = g_lights_[i].pos_WS.xyz - IN.pos_WS.xyz;
        float3 N = normalize(IN.normal_WS.xyz);
        float dist = length(LD);
        LD = normalize(LD);
        float3 R = normalize(reflect(-LD,N));
        float3 V = normalize(IN.cam_pos_WS.xyz - IN.pos_WS.xyz);
        int visibility = max(0,dot(LD,N)) + 0.999f;
        // int visibility = 1;
        float A = 1.0f / (g_lights_[i].attenuation_const_ + g_lights_[i].attenuation_linear_ * dist +
                          g_lights_[i].attenuation_quadr_ * dist * dist);

        float diff_dot = max(0.f,dot(LD,N));
        diff_curr = g_lights_[i].color_ * diff_dot * A;
       
        float spec_dot = max(0.f, dot(R,V));
        float spec_pow = material_props_.specular_power_;
        spec_curr = g_lights_[i].color_ * pow(spec_dot, spec_pow) * A * visibility;

        break;
        }
      }
    diff_total += diff_curr;
    spec_total += spec_curr;
  }

  diff_total = saturate(diff_total);
  spec_total = saturate(spec_total);

  float4 spec_map = SpecularMap.Sample(Sampler, IN.texuv_TS);
  int has_spec_own = clamp(0, 1, ceil(length(material_props_.specular_)));
  int has_spec_map = !has_spec_own;

  float4 emissive = material_props_.emissive_;
  float4 ambient = material_props_.ambient_ * g_global_ambient_;
  float4 diffuse = material_props_.diffuse_ * diff_total;
  float4 specular = spec_map * has_spec_map * spec_total + material_props_.specular_ * has_spec_own * spec_total;

  float4 tex_color = DiffuseMap.Sample(Sampler, IN.texuv_TS);

  if (length(emissive) == 0)
    return (ambient + diffuse + specular) * tex_color;
  else
    return (emissive);
}
