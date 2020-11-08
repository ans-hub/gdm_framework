struct OUT
{
  float3 pos: SV_Target0;
  float3 normal: SV_Target1;
  float4 diff: SV_Target2;
};

struct Input
{
  float3 pos_WS						: TEXCOORD0;
  float3 normal_WS 	   	  : TEXCOORD1;
  float2 texuv_TS   	   	: TEXCOORD2;
  float3 cam_pos_WS	  		: TEXCOORD3;
  float3 tg_WS						: TANGENT;
  float3 bt_WS						: BINORMAL;
  float3 nm_WS						: NORMAL;
};

[[vk::binding(2)]] sampler Sampler;
[[vk::binding(3)]] Texture2D<float4> DiffuseMaps[];

float4 main(Input IN) : OUT
{
  OUT output;
  output.pos = pos_WS;
  output.normal = normal_WS;
  output.diff = Textures[NonUniformResourceIndex(IN.material_index)].Sample(Sampler, IN.texuv_TS);
  return output;
}
