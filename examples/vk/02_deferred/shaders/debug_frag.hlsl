struct Input
{
	float4 pos_ : SV_POSITION;
	float4 color_ : TEXCOORD1;
};


float4 main(Input IN) : SV_TARGET
{
  return IN.color_;
}