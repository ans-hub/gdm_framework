// Shader for drawing 2d plane

cbuffer Flat_PFCB : register(b0)
{
  matrix u_view_proj_;
}

cbuffer Flat_POCB : register(b1)
{
  matrix u_model_;
}

struct Output
{
  float4 pos : SV_POSITION;
  float2 texuv : TEXUV;
};

struct Input
{
  float3 pos : COORD;
  float2 texuv : TEXUV;
};

Output main(Input IN)
{
  matrix mvp = mul(u_view_proj_, u_model_);

  Output OUT;
  OUT.pos = mul(mvp, float4(IN.pos, 1.f));
  OUT.texuv = IN.texuv;
  return OUT;
}
