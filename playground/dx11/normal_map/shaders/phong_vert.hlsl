// Shader uses blinn-phong shading

cbuffer Phong_PFCB : register(b0)
{
  matrix u_view_proj;
  float4 u_cam_pos_WS;
}

cbuffer Phong_POCB : register(b1)
{
  matrix u_model;
}

struct Input
{
  float3 pos 			: COORD;
  float2 texuv 		: TEXUV;
  float3 normal 	: NORMAL;
  float3 tangent 	: TANGENT;
};

struct Output
{
  float4 position   : SV_POSITION;
  float4 pos_WS   	: TEXCOORD0;
  float4 normal_WS  : TEXCOORD1;
  float2 texuv_TS   : TEXCOORD2;
  float4 cam_pos_WS : TEXCOORD3;
};

Output main(Input IN)
{
  matrix mvp = mul(u_view_proj, u_model);

  Output OUT;
  OUT.position = mul(mvp, float4(IN.pos, 1.f));
  OUT.cam_pos_WS = u_cam_pos_WS;
  OUT.pos_WS = mul(u_model, float4(IN.pos, 1.f));
  OUT.normal_WS = mul(u_model, float4(IN.normal, 0.f));
  OUT.texuv_TS = IN.texuv;

  return OUT;
}
