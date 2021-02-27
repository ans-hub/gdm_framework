[[vk::binding(0)]]  cbuffer DebugVs_PFCB
{
  float4x4 u_view_proj;
}

struct Input
{
  float3 pos_WS : COORD;
  float4 color : TEXCOORD1;
};

struct Output
{
	float4 pos_ : SV_POSITION;
	float4 color_ : TEXCOORD1;
};

Output main(Input IN)
{
	Output output;
	output.pos_ = mul(u_view_proj, float4(IN.pos_WS, 1.f));
	output.color_ = IN.color;

	return output;
}