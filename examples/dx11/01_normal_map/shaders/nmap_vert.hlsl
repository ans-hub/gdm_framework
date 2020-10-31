// Shader uses blinn-phong shading

#define LIGHTS_COUNT 4
#define DIR_LIGHT 0
#define POINT_LIGHT 1
// #define SPOT_LIGHT 2

struct Light_PFCB
{
  float4 vector_WS_;
  int type_;
  float3 padding_;

}; // struct Light_PFCB

cbuffer PFCB : register(b0)
{
  float4x4 u_view_proj;
  float4 u_cam_pos_WS;
  Light_PFCB u_lights_[4];
}

cbuffer POCB : register(b1)
{
  matrix u_model;
}

struct Input
{
  float3 pos_MS   : COORD;
  float2 texuv_MS : TEXUV;
  float3 norm_MS 	: NORMAL;
  float3 tg_MS 	  : TANGENT;
};

struct Output
{
  float3 pos_WS						: TEXCOORD0;
  float3 norm_WS					: TEXCOORD1;
  float2 texuv_TS   	   	: TEXCOORD2;
  float3 view_pos_WS			: TEXCOORD3;
  float3 tg_WS						: TANGENT;
  float3 bt_WS						: BINORMAL;
  float3 nm_WS						: NORMAL;
  float4 position   	   	: SV_POSITION;
};

Output main(Input IN)
{
  matrix mvp = mul(u_view_proj, u_model);

  float3 tg_WS = normalize(mul((float3x3)u_model, IN.tg_MS));
  float3 norm_WS = normalize(mul((float3x3)u_model, IN.norm_MS));
  tg_WS = normalize(tg_WS - dot(tg_WS, norm_WS) * norm_WS);
  float3 bt_WS = normalize(cross(norm_WS, tg_WS));
  
  Output OUT;
  OUT.pos_WS = mul(u_model, float4(IN.pos_MS, 1.f)).xyz;
  OUT.norm_WS = mul(u_model, float4(IN.norm_MS, 0.f)).xyz;
  OUT.texuv_TS = IN.texuv_MS;
  OUT.view_pos_WS = u_cam_pos_WS.xyz;
  OUT.tg_WS = tg_WS;
  OUT.bt_WS = bt_WS;
  OUT.nm_WS = norm_WS;
  OUT.position = mul(mvp, float4(IN.pos_MS, 1.f));;

  return OUT;
}