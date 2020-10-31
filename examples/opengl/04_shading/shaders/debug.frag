#version 330

in vec3 ex_color;

out vec4 frag_color;

void main()
{
  frag_color = vec4(ex_color, 1.f);
}
