// Simple test shader with diffuse mapping

cbuffer Flat_PFCB : register(b0)
{
  matrix u_view_proj_;
}

cbuffer Flat_POCB : register(b1)
{
  matrix u_model_;
  float4 u_color_;
}

struct Input
{
  float3 pos : COORD;
  float2 texuv : TEXUV;
  float3 normal : NORMAL;
  float3 tangent : TANGENT;
};

struct Output
{
  float4 pos : SV_POSITION;
  float2 texuv : TEXUV;
  nointerpolation float4 color : TEXCOORD1;
};

Output main(Input IN)
{
  matrix mvp = mul(u_view_proj_, u_model_);

  Output OUT;
  OUT.pos = mul(mvp, float4(IN.pos, 1.f));
  OUT.texuv = IN.texuv;
  OUT.color = u_color_;
  return OUT;
}
