#version 330

// TS - tangent space
// MS - model space
// CS - camera space
// WS - world space

layout(location = 0) in vec3 in_coord_MS;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_norm_MS;

uniform mat4 mx_view;
uniform mat4 mx_model;
uniform mat4 mx_mvp;
uniform vec3 light_dir;

out vec2 ex_uv;
out vec4 ex_normal_CS;
out vec4 ex_normal_WS;
out vec4 ex_light_dir_CS;
out vec4 ex_light_dir_WS;
out vec4 ex_vx_pos_WS;  // see note in pixel shader why ws

void main()
{
  gl_Position = mx_mvp * vec4(in_coord_MS, 1.f);
  vec4 in_color = vec4(0.f,1.f,0.f,1.f);
  vec3 in_light_dir_WS = normalize(light_dir) * -1.f;

  ex_normal_CS = normalize(mx_view * mx_model * vec4(in_norm_MS, 0.f)); // 0.f to prevent translate
  ex_normal_WS = normalize(mx_model * vec4(in_norm_MS, 0.f));
  ex_light_dir_CS = normalize(mx_view * vec4(in_light_dir_WS, 0.f));
  ex_light_dir_WS = vec4(in_light_dir_WS, 0.f);
  ex_uv = in_uv;
  ex_vx_pos_WS = mx_model * vec4(in_coord_MS, 1.f);
}
