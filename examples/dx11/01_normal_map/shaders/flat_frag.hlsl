// Simple test shader with diffuse mapping

Texture2D Texture : register(t0);
sampler Sampler : register(s0);

struct Input
{
  float4 pos : SV_POSITION;
  float2 texuv : TEXUV;
  float4 color : TEXCOORD1;
};

float4 main(Input IN) : SV_TARGET
{
  float4 res = (IN.color.w == 1.f) ? Texture.Sample(Sampler, IN.texuv) : IN.color;
  res.w = 0.5f;
  return res;
}
