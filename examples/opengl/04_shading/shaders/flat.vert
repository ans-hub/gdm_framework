#version 330

layout(location = 0) in vec3 in_coord_MS;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_norm_MS;

uniform mat4 mx_mvp;

void main()
{
  gl_Position = mx_mvp * vec4(in_coord_MS * 0.2f, 1.f);
}
