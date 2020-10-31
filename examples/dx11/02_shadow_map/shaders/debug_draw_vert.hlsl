// Shader for debug drawing

cbuffer Flat_PFCB : register(b0)
{
  matrix u_view_proj_;
  matrix u_unused_;
}

struct Input
{
  float3 pos : COORD;
  float3 col : COLOR;
};

struct Output
{
  float4 pos : SV_POSITION;
  float4 col : TEXTURE1;
};

Output main(Input IN)
{
  Output OUT;
  OUT.pos = mul(u_view_proj_, float4(IN.pos, 1.f));
  OUT.col = float4(IN.col, 1.f);
  return OUT;
}
