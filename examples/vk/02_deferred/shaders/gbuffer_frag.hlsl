struct MaterialProps
{
  float4 emissive_;
  float4 ambient_;
  float4 diffuse_;
  float4 specular_;
  float specular_power_;
  float3 padding_;
};

[[vk::binding(2)]] cbuffer GbufferPs_POCB
{
  MaterialProps material_props_;
  uint material_index_;
}

[[vk::binding(3)]] sampler Sampler;
[[vk::binding(4)]] Texture2D<float4> Textures[];

struct Input
{
  float3 pos_WS						: TEXCOORD0;
  float2 texuv_TS   	   	: TEXCOORD1;
  float3 view_pos_WS			: TEXCOORD2;
  float4 color            : TEXCOORD3;
  float3 tg_WS						: TANGENT;
  float3 bt_WS						: BINORMAL;
  float3 nm_WS						: NORMAL;
  float4 position   	   	: SV_POSITION;
};

struct Output
{
	float4 pos : SV_TARGET0;
	float4 diff : SV_TARGET1;
	float4 norm : SV_TARGET2;
};

static const int v_max_materials = 32;
static const int v_diff_offset = 0;
static const int v_norm_offset = 1;
static const int v_spec_offset = 2;

Output main(Input IN)
{
	Output output = (Output)0;

  const int diff_idx = NonUniformResourceIndex(material_index_ * 3 + v_diff_offset);
  const int norm_idx = NonUniformResourceIndex(material_index_ * 3 + v_norm_offset);

  if (IN.color.w >= -(1e-10))
  {
    output.diff = IN.color + Textures[diff_idx].Sample(Sampler, IN.texuv_TS) * IN.color;
    output.norm = float4(normalize(IN.nm_WS), 0.f);
	  output.pos = float4(IN.pos_WS, 1.0);
  }
  else
  {
    float3 normal_TS = Textures[norm_idx].Sample(Sampler, IN.texuv_TS).xyz;
    bool no_normal_map = normal_TS.x < 0.01 && normal_TS.y < 0.01 && normal_TS.z < 0.01;
    normal_TS = normalize(normal_TS * 2.f - 1.f);

    float3x3 tbn = float3x3(normalize(IN.tg_WS), normalize(IN.bt_WS), normalize(IN.nm_WS));

    float3 normal_WS;
    if (no_normal_map)
      normal_WS = normalize(IN.nm_WS);
    else
      normal_WS = normalize(mul(normal_TS, tbn));

    output.diff = Textures[diff_idx].Sample(Sampler, IN.texuv_TS) * material_props_.diffuse_;
    output.norm = float4(normal_WS, 0.0);
  	output.pos = float4(IN.pos_WS, 1.0);

    output.diff.w = material_props_.specular_power_;
    output.norm.w = material_props_.emissive_;
  }

  return output;
}
