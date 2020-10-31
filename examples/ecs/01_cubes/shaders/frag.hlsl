// Flat px shader

sampler Sampler : register(s0);

Texture2D DiffuseMap : register(t0);

struct Input
{
  float4 position   : SV_Position;
  float2 texuv      : TEXCOORD0;
};

float4 main(Input IN) : SV_TARGET
{
   return DiffuseMap.Sample(Sampler, IN.texuv);
}
