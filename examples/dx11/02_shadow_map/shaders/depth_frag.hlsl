// Shader for drawing depth image for further manipulations

struct Input
{
  float4 pos : SV_POSITION; // it is in NDC already
  float4 posw : TEXTURE0;		// it is in not NDC, divide to w (frustum width)
};

float4 main(Input IN) : SV_TARGET
{
  float depth = IN.posw.z / IN.posw.w;
  return float4(depth, depth, depth, 1);
}
