// Simple test shader with diffuse mapping (affine mapping)

Texture2D Texture : register(t0);
sampler Sampler : register(s0);

struct Input
{
  float4 pos : SV_POSITION;
  noperspective float2 texuv : TEXUV;
};

float4 main(Input IN) : SV_TARGET
{
  return Texture.Sample(Sampler, IN.texuv);
}
