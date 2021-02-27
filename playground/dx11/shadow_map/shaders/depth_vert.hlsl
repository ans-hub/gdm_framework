// Shader for drawing depth image for further manipulations

cbuffer Flat_PFCB : register(b0)
{
  matrix u_view_proj_;
  matrix u_unused_;
}

cbuffer Flat_POCB : register(b1)
{
  matrix u_model_;
}

struct Output
{
  float4 pos : SV_POSITION;
  float4 posw : TEXTURE0;
};

struct Input
{
  float3 pos : COORD;
};

Output main(Input IN)
{
  matrix mvp = mul(u_view_proj_, u_model_);

  Output OUT;
  OUT.pos = mul(mvp, float4(IN.pos, 1.f));
  OUT.posw = OUT.pos;
  return OUT;
}
