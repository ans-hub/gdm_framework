#version 330

layout(location = 0) in vec3 in_vec;
layout(location = 1) in vec3 in_col;
layout(location = 2) in vec3 in_norm;

uniform mat4 mvp;
out vec4 ex_col;

void main() {
  gl_Position = mvp * vec4(in_vec,1.f);
  ex_col = vec4(in_col,1.f);
}
