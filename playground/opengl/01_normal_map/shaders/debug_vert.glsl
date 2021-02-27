#version 330

layout(location = 0) in vec4 in_vertex;

uniform mat4 mx_mvp;

out vec3 ex_color;

void conv_float_to_vec_color(in float f_color, inout vec3 v_color)
{
  highp int i_color = int(f_color);
  v_color.r = ((i_color >> 24) & 0xff) / 255.f;
  v_color.g = ((i_color >> 16) & 0xff) / 255.f;
  v_color.b = ((i_color >> 8) & 0xff) / 255.f;
}

void main()
{
  conv_float_to_vec_color(in_vertex.w, ex_color);

  vec4 vx = in_vertex;
  vx.w = 1.f;
  gl_Position = mx_mvp * vx;
}
