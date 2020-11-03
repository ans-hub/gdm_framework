// Simple test shader with diffuse mapping

[[vk::binding(2)]] sampler Sampler;
[[vk::binding(3)]] Texture2D<float4> Textures[];

struct Input
{
  float4 pos : SV_POSITION;
  float2 texuv : TEXUV;
  float4 color : TEXCOORD1;
};

float4 main(Input IN) : SV_TARGET
{
  float4 res = Textures[NonUniformResourceIndex(1)].Sample(Sampler, IN.texuv);
  return res;
  return float4(0.5f, 0.5f, 0.5f, 0.5f);
}
