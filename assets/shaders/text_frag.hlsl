SamplerState samplerFont : register(s1);
Texture2D textureFont : register(t2);

float4 main([[vk::location(0)]] float2 inUV : TEXCOORD0) : SV_TARGET
{
	float color = textureFont.Sample(samplerFont, inUV).r;
	return color.xxxx;
}