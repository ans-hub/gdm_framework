#version 330

layout(location = 0) in vec3 in_coord_MS;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_norm_MS;
layout(location = 3) in vec3 in_tg_MS;

uniform mat4 mx_model;
uniform mat4 mx_mvp;
uniform vec3 light_pos;
uniform vec3 view_pos;

out vec2 ex_uv;
out vec3 ex_light_dir_TS;
out vec3 ex_view_dir_TS;

void main()
{
  gl_Position = mx_mvp * vec4(in_coord_MS, 1.f);
  vec4 vx_pos_WS = mx_model * vec4(in_coord_MS, 1.f);

  // Prepare vxs tangent space matrix

  mat3 mx_model_it = transpose(inverse(mat3(mx_model)));
  vec3 tg_WS = normalize(vec3(mat3(mx_model_it) * in_tg_MS));
  vec3 norm_WS = normalize(vec3(mat3(mx_model_it) * in_norm_MS));
  tg_WS = normalize(tg_WS - dot(tg_WS, norm_WS) * norm_WS);
  vec3 bt_WS = cross(norm_WS, tg_WS);

  mat3 itbn = transpose(mat3(tg_WS, bt_WS, norm_WS));
  
  // Convert vxs vectors in tangent space

  ex_uv = in_uv;
  ex_light_dir_TS = normalize(itbn * normalize(light_pos - vec3(vx_pos_WS)));
  ex_view_dir_TS = normalize(itbn * normalize(view_pos - vec3(vx_pos_WS)));
}
