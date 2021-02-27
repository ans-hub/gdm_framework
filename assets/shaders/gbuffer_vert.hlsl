[[vk::binding(0)]]  cbuffer GbufferVs_PFCB
{
  float4x4 u_view_proj;
  float3 u_cam_pos_WS;
  float u_dummy;
}

[[vk::binding(1)]] cbuffer GbufferVs_POCB
{
  float4x4 u_model;
  float4 u_color;
}

struct Input
{
  float3 pos_MS : COORD;
  float2 texuv_MS : TEXUV;
  float3 norm_MS : NORMAL;
  float3 tg_MS : TANGENT;
};

struct Output
{
  float3 pos_WS : TEXCOORD0;
  float2 texuv_TS : TEXCOORD1;
  float3 view_pos_WS : TEXCOORD2;
  float4 color : TEXCOORD3;
  float3 tg_WS : TANGENT;
  float3 bt_WS : BINORMAL;
  float3 nm_WS : NORMAL;
  float4 position : SV_POSITION;
};

Output main(Input IN)
{
  float4x4 mvp = mul(u_view_proj, u_model);

  float3 tg_WS = normalize(mul((float3x3)u_model, IN.tg_MS));
  float3 norm_WS = normalize(mul((float3x3)u_model, IN.norm_MS));
  tg_WS = normalize(tg_WS - dot(tg_WS, norm_WS) * norm_WS);
  float3 bt_WS = normalize(cross(norm_WS, tg_WS));
  
  Output OUT;
  OUT.pos_WS = mul(u_model, float4(IN.pos_MS, 1.f)).xyz;
  OUT.texuv_TS = IN.texuv_MS;
  OUT.view_pos_WS = u_cam_pos_WS;
  OUT.tg_WS = tg_WS;
  OUT.bt_WS = bt_WS;
  OUT.nm_WS = norm_WS;
  OUT.color = u_color;
  OUT.position = mul(mvp, float4(IN.pos_MS, 1.f));

  return OUT;
}
