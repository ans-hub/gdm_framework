// Shader for shading mapping

sampler SamplerFlat : register(s0);

Texture2D DiffuseMap : register(t0);
Texture2D ShadowMap : register(t1);

struct MaterialProps
{
  float4 emissive_;
  float4 ambient_;
  float4 diffuse_;
  float4 specular_;
  float specular_power_;

}; // struct MaterialProps

cbuffer MaterialPropsBuffer : register(b0)
{
  MaterialProps material_props_;
};

struct Input
{
  float4 pos : SV_POSITION;
  float2 texuv : TEXUV;
  float4 pos_light_space: TEXTURE1;
};

float4 main(Input IN) : SV_TARGET
{
  float ambient = 0.4f;
  float4 diffuse = material_props_.diffuse_ * 0.3f;
  float4 tex_color = DiffuseMap.Sample(SamplerFlat, IN.texuv);
  float3 proj_coords = IN.pos_light_space.xyz / IN.pos_light_space.w;	// not needed for ortho
  proj_coords.x = proj_coords.x * 0.5f + 0.5f;
  proj_coords.y = -proj_coords.y * 0.5f + 0.5f;
  float closest_depth = ShadowMap.Sample(SamplerFlat, proj_coords.xy).r;
  float curr_depth = proj_coords.z;
  float bias = 0.005f; // todo: implement smart bias as on opengl-tutorials
  float shadow = (curr_depth - bias) > closest_depth ? 1.f : 0.f;
  if (curr_depth > 1.f)
	  shadow = 0;
  return (ambient + (1.f - shadow) * diffuse) * tex_color;    
}
