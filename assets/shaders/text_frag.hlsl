cbuffer Text_POCB : register(b0)
{
  float4 color_;
}

SamplerState samplerFont : register(s1);
Texture2D textureFont : register(t2);

float4 main([[vk::location(0)]] float2 inUV : TEXCOORD0) : SV_TARGET
{
	float4 sampled = float4(1.f, 1.f, 1.f, textureFont.Sample(samplerFont, inUV).r);
  return color_ * sampled;
}
