// Shader for shading mapping

cbuffer Flat_PFCB : register(b0)
{
  matrix u_view_proj_;
  matrix u_light_view_proj_;
}

cbuffer Flat_POCB : register(b1)
{
  matrix u_model_;
}

struct Output
{
  float4 pos : SV_POSITION;
  float2 texuv : TEXUV;
  float4 pos_light_space: TEXTURE1;
};

struct Input
{
  float3 pos : COORD;
  float2 texuv : TEXUV;
};

Output main(Input IN)
{
  Output OUT;
	
  matrix mvp = mul(u_view_proj_, u_model_);
  OUT.pos = mul(mvp, float4(IN.pos, 1.f));
  OUT.texuv = IN.texuv;
	
  matrix mvp_light = mul(u_light_view_proj_, u_model_);
  OUT.pos_light_space = mul(mvp_light, float4(IN.pos, 1.f));
  return OUT;
}

// note: OUT.pos = OUT.pos_light_space; // to check of fs and ensure that pixels are converted to correct space