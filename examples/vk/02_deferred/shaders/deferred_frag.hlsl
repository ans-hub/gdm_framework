#define DIR_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

SamplerState sampler_gen : register(s1);
Texture2D gbuffer_pos : register(t2);
Texture2D gbuffer_diff : register(t3);
Texture2D gbuffer_norm : register(t4);
// struct MaterialProps
// {
//   float4 emissive_;
//   float4 ambient_;
//   float4 diffuse_;
//   float4 specular_;
//   float specular_power_;
// };

// struct LightProps
// {
//   float4 pos_WS;
//   float4 dir_WS;
//   float4 color_;
//   float spot_angle_;
//   float attenuation_const_;
//   float attenuation_linear_;
//   float attenuation_quadr_;
//   int type_;
//   int enabled_;
//   float2 padding_;
// };

// cbuffer MaterialPropsBuffer : register(b0)
// {
//   MaterialProps material_props_;
// };

// cbuffer LightPropsBuffer : register(b0)
// {
//   float4 g_global_ambient_;
//   float4 g_camera_pos_;
//   LightProps g_lights_[4];
// };

struct VSOutput
{
  float2 UV : TEXUV;
	float4 Pos : SV_POSITION;
};

float4 main(VSOutput input) : SV_TARGET
{
  return gbuffer_diff.Sample(sampler_gen, input.UV);
  // return float4(1,1,1,1);
	// Get G-Buffer values
	// float3 fragPos = textureposition.Sample(samplerposition, inUV).rgb;
	// float3 normal = textureNormal.Sample(samplerNormal, inUV).rgb;
	// float4 albedo = textureAlbedo.Sample(samplerAlbedo, inUV);

	// float3 fragcolor;

	// // Debug display
	// if (ubo.displayDebugTarget > 0) {
	// 	switch (ubo.displayDebugTarget) {
	// 		case 1: 
	// 			fragcolor.rgb = fragPos;
	// 			break;
	// 		case 2: 
	// 			fragcolor.rgb = normal;
	// 			break;
	// 		case 3: 
	// 			fragcolor.rgb = albedo.rgb;
	// 			break;
	// 		case 4: 
	// 			fragcolor.rgb = albedo.aaa;
	// 			break;
	// 	}		
	// 	return float4(fragcolor, 1.0);
	// }

	// #define lightCount 6
	// #define ambient 0.0

	// // Ambient part
	// fragcolor = albedo.rgb * ambient;

	// for(int i = 0; i < lightCount; ++i)
	// {
	// 	// Vector to light
	// 	float3 L = ubo.lights[i].position.xyz - fragPos;
	// 	// Distance from light to fragment position
	// 	float dist = length(L);

	// 	// Viewer to fragment
	// 	float3 V = ubo.viewPos.xyz - fragPos;
	// 	V = normalize(V);

	// 	//if(dist < ubo.lights[i].radius)
	// 	{
	// 		// Light to fragment
	// 		L = normalize(L);

	// 		// Attenuation
	// 		float atten = ubo.lights[i].radius / (pow(dist, 2.0) + 1.0);

	// 		// Diffuse part
	// 		float3 N = normalize(normal);
	// 		float NdotL = max(0.0, dot(N, L));
	// 		float3 diff = ubo.lights[i].color * albedo.rgb * NdotL * atten;

	// 		// Specular part
	// 		// Specular map values are stored in alpha of albedo mrt
	// 		float3 R = reflect(-L, N);
	// 		float NdotR = max(0.0, dot(R, V));
	// 		float3 spec = ubo.lights[i].color * albedo.a * pow(NdotR, 16.0) * atten;

	// 		fragcolor += diff + spec;
	// 	}
	// }

  // return float4(fragcolor, 1.0);
}