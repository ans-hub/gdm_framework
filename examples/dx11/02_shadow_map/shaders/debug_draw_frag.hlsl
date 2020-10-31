// Shader for debug drawing

struct Input
{
  float4 pos : SV_POSITION;
  float4 col : TEXTURE1;
};

float4 main(Input IN) : SV_TARGET
{
  return float4(IN.col.r, IN.col.g, IN.col.b, IN.col.a);
}