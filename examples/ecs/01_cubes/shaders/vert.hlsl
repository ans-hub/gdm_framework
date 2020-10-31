// Flat vx shader

cbuffer Flat_PFCB : register(b0)
{
  matrix u_view_proj;
}

cbuffer Flat_POCB : register(b1)
{
  matrix u_model;
}

struct Input
{
  float3 pos 			: COORD;
  float2 texuv 		: TEXUV;
  float3 normal 	: NORMAL;
  float3 tangent 	: TANGENT;
};

struct Output
{
  float4 position   : SV_POSITION;
  float2 texuv      : TEXCOORD0;
};

Output main(Input IN)
{
  matrix mvp = mul(u_view_proj, u_model);

  Output OUT;
  OUT.position = mul(mvp, float4(IN.pos, 1.f));
  OUT.texuv = IN.texuv;

  return OUT;
}
