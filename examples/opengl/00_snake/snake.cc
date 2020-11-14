// *************************************************************
// File:    snake.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include <iostream>
#include <vector>
#include <array>
#include <cassert>
#include <cstdlib>
#include <ctime>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// INIT SETTINGS

namespace Dirs
{
  const glm::vec2 UP {0.f, 1.f};
  const glm::vec2 DOWN {0.f, -1.f};
  const glm::vec2 LEFT {-1.f, 0.f};
  const glm::vec2 RIGHT {1.f, 0.f};

} // namespace Dirs

namespace Params
{
  const float lb = -10.f;
  const float rb = 10.f;
  const float bb = -10.f;
  const float ub = 10.f;
  const int snake_sz_min = 3;
  const int snake_sz_max = 50;
  const int snake_speed_min = 500;
  const int snake_speed_max = 10;
  int snake_speed_curr = 500;
  glm::vec2 snake_initial_dir = Dirs::RIGHT;
  const int gift_sz_max = 10; 
  const int gift_init_cnt = 2;
  const int gift_seeding_rate_min = 8000;
  const int gift_seeding_rate_max = 3000;
  int gift_seeding_rate = 8000;
  const int poison_chance = 5;

  int win_w = 800;
  int win_h = 600;
  float ar = (float)win_w / (float)win_h;
  int fps = 0;

} // namespace Params

struct Quad
{
  using ui = unsigned int;
  enum Color : ui
  {
    SNAKE = ((ui)229 << 24) | ((ui)127 << 16) | ((ui)51 << 8),
    GIFT = ((ui)0 << 24) | ((ui)127 << 16) | ((ui)20 << 8),
    PSN = ((ui)127 << 24) | ((ui)20 << 16) | ((ui)20 << 8),
    WALL = ((ui)220 << 24) | ((ui)220 << 16) | ((ui)220 << 8),
    GRAY = ((ui)127 << 24) | ((ui)127 << 16) | ((ui)127 << 8)
  };

  float coords[12] =
  {
    -0.5f, -0.5f,
     0.5f,  0.5f,
    -0.5f,  0.5f,
    -0.5f, -0.5f,
     0.5f, -0.5f,
     0.5f,  0.5f
  };

}; // struct Quad

struct Shader
{
  Shader(const char* vx_sh, const char* frag_sh);
  void Use() { glUseProgram(prog_); }

  GLuint prog_;
  GLuint uni_mvp_;
  GLuint uni_color_;

}; // struct Shader

struct Game
{
  enum State { PLAY, GAME_OVER };
  Game(State state);
  static void process_keys_cb(unsigned char key, int x, int y);
  static void process_state_cb(int value);
  
  State state_;

}; // struct Game

struct Snake
{
  explicit Snake(int initial_size);
  void Init(int initial_sz);
  static void process_keys_cb(int key, int x, int y);
  static void moving_cb(int value);
  static void intersection_cb(int value);

  std::vector<glm::vec2> world_poses_ = {};
  glm::vec2 curr_dir_ = Params::snake_initial_dir;
  glm::vec2 prev_tail_pos_ = {};

}; // struct Snake

struct Gift
{
  Gift(int init_count, unsigned int seeding_time);
  void Init(int init_count);
  void Seed();
  static void seeding_cb(int value);

  std::vector<glm::vec2> world_poses_gft_ = {};
  std::vector<glm::vec2> world_poses_psn_ = {};

}; //struct Gift


struct Wall
{
  Wall();
  std::vector<glm::vec2> world_poses_ = {};

}; // struct Wall

struct Renderer
{
  Renderer();
  static void draw_quad(const glm::vec2& world_pos_2d, Quad::Color color);
  static void display_cb();

  Quad quad_;
  GLuint vao_;
  glm::mat4 scale_mx_;
  glm::mat4 proj_mx_;

}; //struct Renderer

struct Window
{
  Window(int argc, char** argv);
  void InitGlew();
  static void idle_cb(void);
  static void timer_cb(int value);
  static void reshape_cb(int w, int h);

}; // struct Window

// HELPER FUNCTIONS

namespace helpers {

static float randf(float a, float b)
{
  float rnd = ((float)std::rand()) / (float)RAND_MAX;
  float dt = b - a;
  return a + (rnd * dt);
}

int Overlapped(const std::vector<glm::vec2>& v, const glm::vec2& c, int start_idx = 0)
{
  for (std::size_t i = start_idx; i < v.size(); ++i)
    if (glm::uvec2(v[i]) == glm::uvec2(c))
      return i;
  return -1;
}

void GLAPIENTRY message_cb(GLenum source, GLenum type, GLuint id, 
                           GLenum severity, GLsizei length,
                           const GLchar* message, const void* userParam)
{
  fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
         (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
          type, severity, message);
}

template<class T>
T cvt(T in_min, T in_max, T out_min, T out_max, T in_val)
{
  return (in_val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

} // namespace helpers

// GLOBALS

Window* g_window = nullptr;
Renderer* g_renderer = nullptr;
Snake* g_snake = nullptr;
Wall* g_wall = nullptr;
Gift* g_gift = nullptr;
Shader* g_shader = nullptr;
Game* g_game = nullptr;

static const char* vertex_shader = 
  "#version 400\n"
  "layout(location=0) in vec2 in_pos;"
  "uniform uint color;"
  "uniform mat4 mvp;"
  "out vec4 ex_col;"
  "void main() {"
  "  gl_Position = mvp * vec4(in_pos,0.f,1.f);"
  "  float r = ((color >> 24) & 0xff) / 255.f;" 
  "  float g = ((color >> 16) & 0xff) / 255.f;" 
  "  float b = ((color >> 8) & 0xff) / 255.f;" 
  "  ex_col = vec4(r,g,b,1.f);"
  "}";

static const char* fragment_shader = 
  "#version 400\n"
  "in vec4 ex_col;"
  "out vec4 frag_col;"
  "void main() {"
  "  frag_col = ex_col;"
  "}";

// WINDOW IMPLEMENTATION

void Window::idle_cb(void)
{
  glutPostRedisplay();
}

void Window::timer_cb(int Value)
{  
  Params::fps = 0;
  glutTimerFunc(250, timer_cb, 1);
}

void Window::reshape_cb(int w, int h)
{
  Params::win_w = w;
  Params::win_h = h;
  Params::ar = float(w) / float(h);
  glViewport(0, 0, w, h);
}

Window::Window(int argc, char** argv)
{
  glutInit(&argc, argv);
  glutInitContextVersion(4,3);
  glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
  glutInitContextProfile(GLUT_CORE_PROFILE);
  glutSetOption(
    GLUT_ACTION_ON_WINDOW_CLOSE,
    GLUT_ACTION_GLUTMAINLOOP_RETURNS
  );
  glutInitWindowSize(Params::win_w, Params::win_h);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

  if(glutCreateWindow("Snake") < 1)
  {
    std::cerr << "Failed to create window\n";
    exit(EXIT_FAILURE);
  }
  glutReshapeFunc(Window::reshape_cb);
  glutIdleFunc(Window::idle_cb);
  glutTimerFunc(0, Window::timer_cb, 0);
  glutDisplayFunc(Renderer::display_cb);

  InitGlew();
}

void Window::InitGlew()
{
  GLenum init_res = glewInit();
  if (init_res != GLEW_OK)
  {
    std::cerr << "Can't init glew: " << glewGetErrorString(init_res) << '\n';
    exit(EXIT_FAILURE);
  }
  std::cout << "OpenGL version is " << glGetString(GL_VERSION) << '\n';
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glEnable(GL_DEBUG_OUTPUT);  
  glDebugMessageCallback(helpers::message_cb, 0);
}

// SHADER IMPLEMENTATION

Shader::Shader(const char* vx_sh, const char* frag_sh)
{
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &vx_sh, NULL);
  glCompileShader(vs);
  
  int status;
  glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE)
  {
    char info[256];
    glGetShaderInfoLog(vs, 256, nullptr, info);
    std::cerr << "Vertex shader compilation error\n" << info << '\n';
  }

  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &frag_sh, NULL);
  glCompileShader(fs);
  
  glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE)
  {
    char info[256];
    glGetShaderInfoLog(fs, 256, nullptr, info);
    std::cerr << "Fragment shader compilation error\n" << info << '\n';
  }

  prog_ = glCreateProgram();
  glAttachShader(prog_, fs);
  glAttachShader(prog_, vs);
  glLinkProgram(prog_);
  glGetProgramiv(prog_, GL_LINK_STATUS, &status);
  assert(status == GL_TRUE && "Shader linking failed");

  Use();

  uni_mvp_ = glGetUniformLocation(prog_, "mvp");
  uni_color_ = glGetUniformLocation(prog_, "color");

  std::cout << "Shader compilation and linking success" << '\n';
}

// RENDERER IMPLEMENTATION

Renderer::Renderer()
{
  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);

  GLuint snake_vbo;
  int snake_vbo_sz = sizeof(float) * sizeof(Quad);

  glGenBuffers(1, &snake_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, snake_vbo);
  glBufferData(GL_ARRAY_BUFFER, snake_vbo_sz, &quad_.coords[0], GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  scale_mx_ = glm::scale(glm::mat4(1.f), glm::vec3(0.9f,0.9f,0.9f));
  proj_mx_  = glm::ortho(Params::lb * Params::ar, Params::rb * Params::ar,
                         Params::bb, Params::ub, -1.0f, 1.0f);
}

void Renderer::display_cb()
{
  ++Params::fps;
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  g_shader->Use();

  glBindVertexArray(g_renderer->vao_);
  glEnableVertexAttribArray(0);

  for (const auto pos : g_wall->world_poses_)
    draw_quad(pos, g_game->state_ == Game::PLAY ? Quad::WALL : Quad::GRAY);
  for (const auto pos : g_gift->world_poses_gft_)
    draw_quad(pos, g_game->state_ == Game::PLAY ? Quad::GIFT : Quad::GRAY);
  for (const auto pos : g_gift->world_poses_psn_)
    draw_quad(pos, g_game->state_ == Game::PLAY ? Quad::PSN : Quad::GRAY);
  for (const auto pos : g_snake->world_poses_)
    draw_quad(pos, g_game->state_ == Game::PLAY ? Quad::SNAKE : Quad::GRAY);

  glDisableVertexAttribArray(0);
  glBindVertexArray(0);
  glutSwapBuffers();
}

void Renderer::draw_quad(const glm::vec2& world_pos_2d, Quad::Color color)
{
  const int k_start_vx_idx = 0;
  const int k_point_dimension = 2; 
  int k_vxs_count = sizeof(Quad) / sizeof(float) / k_point_dimension; 

  glm::vec3 world_pos (world_pos_2d, 0.f);
  glm::mat4 translate_mx = glm::translate(glm::mat4(1.f), world_pos);
  glm::mat4 mvp_mx = g_renderer->proj_mx_ * translate_mx * g_renderer->scale_mx_;
  glUniformMatrix4fv(g_shader->uni_mvp_, 1, false, glm::value_ptr(mvp_mx));
  glUniform1ui(g_shader->uni_color_, color);
  glDrawArrays(GL_TRIANGLES, k_start_vx_idx, k_vxs_count);
}

// SNAKE IMPLEMENTATION

Snake::Snake(int snake_sz)
{
  glutTimerFunc(0, Snake::moving_cb, 0);
  glutTimerFunc(0, Snake::intersection_cb, 0);
  glutSpecialFunc(Snake::process_keys_cb);

  Init(snake_sz);
}

void Snake::Init(int initial_sz)
{
  world_poses_.clear();
  curr_dir_ = Params::snake_initial_dir;
  glm::vec2 pos (0.0f, 0.f);
  for (int i = 0; i < initial_sz; ++i)
  {
    world_poses_.push_back(pos);
    pos += Dirs::LEFT;
  }
}

void Snake::process_keys_cb(int key, int x, int y)
{
  switch (key)
  {
    case GLUT_KEY_RIGHT:
      g_snake->curr_dir_ = Dirs::RIGHT; break;
    case GLUT_KEY_LEFT:
      g_snake->curr_dir_ = Dirs::LEFT; break;
    case GLUT_KEY_UP:
      g_snake->curr_dir_ = Dirs::UP; break;
    case GLUT_KEY_DOWN:
      g_snake->curr_dir_ = Dirs::DOWN; break;
    default: break;
  }
}

void Snake::moving_cb(int /*value*/)
{
  glutTimerFunc(Params::snake_speed_curr, Snake::moving_cb, 1);
  if (g_game->state_ == Game::State::GAME_OVER)
    return;
  glm::vec2 right {1.f, 0.f};
  g_snake->prev_tail_pos_ = g_snake->world_poses_.back();
  for (int i = g_snake->world_poses_.size()-1; i > 0; --i)
    g_snake->world_poses_[i] = g_snake->world_poses_[i-1];
  g_snake->world_poses_[0] += g_snake->curr_dir_;
}

void Snake::intersection_cb(int /*value*/)
{
  glutTimerFunc(Params::snake_speed_curr, Snake::intersection_cb, 1);
  if (g_game->state_ == Game::State::GAME_OVER)
    return;

  glm::vec2 head = g_snake->world_poses_.front();
  
  int ov_idx = helpers::Overlapped(g_gift->world_poses_gft_, head);
  if (ov_idx >= 0)  // add tail, remove gift, update speed
  {
    g_snake->world_poses_.push_back(g_snake->prev_tail_pos_);
    g_gift->world_poses_gft_.erase(g_gift->world_poses_gft_.begin() + ov_idx);
  }

  if (helpers::Overlapped(g_snake->world_poses_, head, 1) >= 1)
    g_game->state_ = Game::State::GAME_OVER;
  if (helpers::Overlapped(g_wall->world_poses_, head) >= 0)
    g_game->state_ = Game::State::GAME_OVER;
  if (helpers::Overlapped(g_gift->world_poses_psn_, head) >= 0)
    g_game->state_ = Game::State::GAME_OVER;
}

// GIFT IMPLEMENTATION

Gift::Gift(int initial_cnt, unsigned int seeding_time)
{
  glutTimerFunc(0, Gift::seeding_cb, 0);    
  Init(initial_cnt);
}

void Gift::Init(int initial_cnt)
{
  world_poses_gft_.clear();
  world_poses_psn_.clear();
  for (int i = 0; i < initial_cnt; ++i)
    Seed();
}

void Gift::Seed()
{
  if (!g_snake || !g_wall)
    return;
  const int k_max_tries {16};
  for (int i = 0; i < k_max_tries; ++i)
  {
    int x = static_cast<int>(helpers::randf(Params::lb, Params::rb));
    int y = static_cast<int>(helpers::randf(Params::bb, Params::ub));
    int snake_ov = helpers::Overlapped(g_snake->world_poses_, {x,y});
    int wall_ov = helpers::Overlapped(g_wall->world_poses_, {x,y});
    int gift_ov = helpers::Overlapped(world_poses_gft_, {x,y});
    int psn_ov = helpers::Overlapped(world_poses_psn_, {x,y});
    if (snake_ov < 0 && wall_ov < 0 && gift_ov < 0)
    {
      if (static_cast<int>(helpers::randf(0,Params::poison_chance)) == 1)
        world_poses_psn_.push_back({x,y});
      else
        world_poses_gft_.push_back({x,y});
      break;
    }
  }
}

void Gift::seeding_cb(int /*value*/)
{
  glutTimerFunc(Params::gift_seeding_rate, Gift::seeding_cb, 1);
  if (g_game->state_ == Game::State::GAME_OVER)
    return;
  g_gift->Seed();
}

// WALL IMPLEMENTATION

Wall::Wall()
{
  for (int i = Params::lb; i <= Params::rb; ++i)
  {
    world_poses_.push_back({i,Params::ub - 1.f});
    world_poses_.push_back({i,Params::bb + 1.f});
  }
  for (int i = Params::bb + 1.f; i < Params::ub; ++i)
  {
    world_poses_.push_back({Params::lb,i});
    world_poses_.push_back({Params::rb,i});
  }
}

// GAME IMPLEMENTATION

Game::Game(State state) 
  : state_{state}
{
  glutTimerFunc(50, Game::process_state_cb, 1);  
  glutKeyboardFunc(Game::process_keys_cb);
}

void Game::process_state_cb(int /*value*/)
{
  switch (g_game->state_)
  {
    case Game::State::PLAY :
    {
      Params::snake_speed_curr = helpers::cvt(
        Params::snake_sz_min, Params::snake_sz_max,
        Params::snake_speed_min, Params::snake_speed_max,
        (int)g_snake->world_poses_.size()
      );
      Params::gift_seeding_rate = helpers::cvt(
        Params::snake_sz_min, Params::snake_sz_max,
        Params::gift_seeding_rate_min, Params::gift_seeding_rate_max,
        (int)g_snake->world_poses_.size()
      );
    }
    case Game::State::GAME_OVER : break;
    default: break;
  }
}

void Game::process_keys_cb(unsigned char key, int x, int y)
{
  switch (key)
  {
    case 13:
    {
      g_game->state_ = Game::State::PLAY;
      g_snake->Init(Params::snake_sz_min);
      g_gift->Init(Params::gift_init_cnt);
    }
    default: break;
  }
}

// ****************************************************************************

int main(int argc, char** argv)
{
  std::srand(std::time(0));

  g_window = new Window(argc, argv);
  g_renderer = new Renderer();
  g_game = new Game(Game::State::PLAY);
  g_wall = new Wall();
  g_snake = new Snake(Params::snake_sz_min);
  g_gift = new Gift(Params::gift_init_cnt, Params::gift_seeding_rate);
  g_shader = new Shader(vertex_shader, fragment_shader);

  glutMainLoop();
  return 0;
}