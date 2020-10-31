// Simple test shader with diffuse mapping

[[vk::binding(2)]] Texture2D Texture;
[[vk::binding(3)]] sampler Sampler;

struct Input
{
  float4 pos : SV_POSITION;
  float2 texuv : TEXUV;
  float4 color : TEXCOORD1;
};

float4 main(Input IN) : SV_TARGET
{
  float4 res = Texture.Sample(Sampler, IN.texuv);
  // res.w = 0.5f;
  return res;
  // return float4(0.5f, 0.5f, 0.5f, 0.5f);
}
