// Shader for drawing 2d plane

sampler SamplerFlat : register(s0);

Texture2D DiffuseMap : register(t0);

struct Input
{
  float4 pos : SV_POSITION;
  float2 texuv : TEXUV;
};

float4 main(Input IN) : SV_TARGET
{
  return DiffuseMap.Sample(SamplerFlat, IN.texuv);
}