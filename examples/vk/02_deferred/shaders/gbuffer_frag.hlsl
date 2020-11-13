[[vk::binding(2)]] sampler Sampler;
[[vk::binding(3)]] Texture2D<float4> Textures[];

struct Input
{
  float3 pos_WS						: TEXCOORD0;
  float3 norm_WS					: TEXCOORD1;
  float2 texuv_TS   	   	: TEXCOORD2;
  float3 view_pos_WS			: TEXCOORD3;
  uint   material_index   : TEXCOORD4;
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

Output main(Input input)
{
	Output output = (Output)0;
  // float4 tex = float4(0.5f,0.5f,0.5f,1.f);
  float4 tex = Textures[NonUniformResourceIndex(input.material_index)].Sample(Sampler, input.texuv_TS);
  output.diff = tex;
	output.pos = float4(input.pos_WS, 1.0);
  output.norm = float4(input.nm_WS, 1.0);

  return output;
}
