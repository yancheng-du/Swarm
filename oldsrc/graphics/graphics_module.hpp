#ifndef GRAPHICS_MODULE_H
#define GRAPHICS_MODULE_H

#define GLM_ENABLE_EXPERIMENTAL
#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <algorithm>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
using namespace glm;
using namespace std;

// CPU representation of a particle
typedef struct Particle Particle;

//struct for holding info about each text object we want to display
//representation of displayed text
typedef struct Text {
	string text;
	float x;
	float y;
	float size;
} Text;

class GraphicsModule{
private:
  glm::vec3 camera_position;
  int MaxParticles;
  float max_x;
  float max_y;
  float min_x;
  float min_y;
  int pixelsX;
  int pixelsY;
  float scale;
  float screen_ratio;
  float screen_scale;
  bool is_init;
  Particle* ParticlesContainer;
  GLfloat* g_particule_position_size_data;
  GLfloat* g_particule_stage_data;

  //for bees
  GLuint particles_position_buffer;
  GLuint billboard_vertex_buffer;
  GLuint particles_stage_buffer;
  GLuint TextureID;
  GLuint programID;
  GLuint Texture;
  GLuint CameraRight_worldspace_ID;
  GLuint CameraUp_worldspace_ID;
  GLuint ViewProjMatrixID;
  glm::mat4 ViewMatrix;
  glm::mat4 ViewProjectionMatrix;

  //for QR code
  GLuint QRProgramID;
  GLuint QRCameraRight_worldspace_ID;
  GLuint QRCameraUp_worldspace_ID;
  GLuint QRViewProjMatrixID;
  GLuint QRPosID;
  GLuint QRSizeID;
  GLuint QRTextureID;
  GLuint QRTexture;
  GLuint qr_vertex_buffer;
  int qr_x;
  int qr_y;
  int qr_size;
  bool qr_enabled;
  bool qr_was_enabled;

  //for text
  vector<Text> texts;

  //for video recording
  FILE *ffmpeg;
  bool record;
  int frame_total;
  int frame_count;
  string cmd;

public:
  void SortParticles();
  GraphicsModule( int num_particles, int maxX, int maxY,
                  float screenScale, float beeSize, bool fullscreen,
                  const char* texture_fp,
                  const char* module_dir);
  int update_particles(vector<int> x, vector<int> y, vector<int> stage, vector<int> direction);
  void init_qr(string mdir);
  void update_qr(bool enabled, const char* qrcode_fp, int x, int y, int size);
  void render_qr();
  void add_text(string text, int x, int y, float size);
  bool remove_text(string text);
  float to_opengl_world_x(int x);
  float to_opengl_world_y(int y);
  bool update_display(bool start_recording);
  void cleanup();
  bool should_close();
	void generate();
};

#endif
