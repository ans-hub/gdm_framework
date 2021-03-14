[[vk::binding(0)]] cbuffer Text_POCB
{
  float4 color_;
}

[[vk::binding(1)]] sampler Sampler;
[[vk::binding(2)]] Texture2D textureFont;

float4 main([[vk::location(0)]] float2 inUV : TEXCOORD0) : SV_TARGET
{
	float4 sampled = float4(1.f, 1.f, 1.f, textureFont.Sample(Sampler, inUV).r);
  return color_ * sampled;
}
