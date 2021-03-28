#define GLM_ENABLE_EXPERIMENTAL
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>

#include <vector>

#include <unistd.h>
#include <algorithm>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
using namespace glm;
using namespace std;

#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <wchar.h>
#include "common/shader.hpp"
#include "common/texture.hpp"
#include "common/controls.hpp"
#include "common/text2D.hpp"
#include "graphics_module.hpp"

//qr code stuff
#define OUT_FILE					"./test.bmp"								// Output file name
#define OUT_FILE_PIXEL_PRESCALER	8											// Prescaler (number of pixels in bmp file for each QRCode pixel, on each dimension)

#define PIXEL_COLOR_R				0											// Color of bmp pixels
#define PIXEL_COLOR_G				0
#define PIXEL_COLOR_B				0xff
#define fopen_s(pFile,filename,mode) ((*(pFile))=fopen((filename),  (mode)))==NULL

typedef unsigned short	WORD;
typedef unsigned long	DWORD;
typedef signed long		LONG;

#define BI_RGB			0L

#pragma pack(push, 2)

typedef struct
	{
	WORD    bfType;
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
	} BITMAPFILEHEADER;

typedef struct
	{
	DWORD      biSize;
	LONG       biWidth;
	LONG       biHeight;
	WORD       biPlanes;
	WORD       biBitCount;
	DWORD      biCompression;
	DWORD      biSizeImage;
	LONG       biXPelsPerMeter;
	LONG       biYPelsPerMeter;
	DWORD      biClrUsed;
	DWORD      biClrImportant;
	} BITMAPINFOHEADER;

#pragma pack(pop)

// CPU representation of a particle
typedef struct Particle{
	glm::vec3 pos, speed;
	float size;
	int stage;
	int movement_type;
	int direction;
	float cameradistance; // *Squared* distance to the camera. if dead : -1.0f

	bool operator<(const Particle& that) const {
		// Sort in reverse order : far particles drawn first.
		return this->cameradistance > that.cameradistance;
	}
} Particle;

//helper function that sorts our particles, useful for ordering to avoid
//appearance of overlap
void GraphicsModule::SortParticles(){
  std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}

//helper function that does a translation and scale for x values
float GraphicsModule::to_opengl_world_x(int x){
	return x*scale-max_x;
}

//helper function that does a translation and scale for y values
float GraphicsModule::to_opengl_world_y(int y){
	return y*scale-max_y;
}

/*
constructor for the graphics module class, the Graphics module class should be
used with the following format

GraphicsModule gm (..)
...
do{
	...
	gm.update_particles(..);
	gm.update_display();
}while(!gm.should_close());
...
gm.cleanup();

params:
	int num_particles : the number of particles that the graphics module will maintain
	int maxX : the maximum value of X that will be passed to the graphics module
		         as part of a particle position
	int maxY : the maximum value of Y that will be passed to the graphics module
	           as part of a particle position
	int screenScale : how many times bigger the window will be than a maxX x maxY
	                  matrix
	bool fullscreen : whether or not the window should occupy the entire screen
  float beeSize : how big each of the particles should be
	char* texture_fp : the file path to the texture (must be a .png)
	char* module_dir : the path to graphics_module.cpp relative to the caller
	                   of the constructor
*/
GraphicsModule::GraphicsModule(int num_particles, int maxX, int maxY,
	                       float screenScale,
															 float beeSize,
															 bool fullscreen,
															 const char* texture_fp,
															 const char* module_dir){

	pixelsX = (int)(screenScale*maxX);
	pixelsY = (int)(screenScale*maxY);
	//variables for qr, recording and text
	qr_enabled = false;
	record = false;
	ffmpeg = NULL;
	frame_count = 0;
	frame_total = 250;

	//screen scaling factor
	screen_ratio = (float)maxX/maxY;
	screen_scale = screenScale;

	//map our maxX and maxY to reasonable numbers and keep the scaling factor
  scale = 20.0f/maxX;
	max_x = scale*maxX/2.0f; max_y = scale*maxY/2.0f;

  min_x = -max_x; min_y = -max_y;
	std::string width = std::to_string((int)(pixelsX));
	std::string height = std::to_string((int)(pixelsY));
	//std::string cmd = "sudo avconv -y -f rawvideo -s "+width+"x"+height+" -pix_fmt rgb24 -r 25 -i - -vf vflip -an -b:v 1000k test.mp4";
	//fprintf(stderr, cmd.c_str());
	cmd = "sudo ffmpeg -r 25 -f rawvideo -pix_fmt rgb24 -s "+width+"x"+height+" -i - "
                    "-threads 0 -preset fast -y -pix_fmt yuv420p -crf 21 -vf vflip -an test.mp4";

	camera_position = glm::vec3(0, 0, 30/screen_ratio);
	MaxParticles = num_particles;
	ParticlesContainer = (Particle*)malloc(sizeof(Particle)*MaxParticles);

  // Initialise GLFW
  if( !glfwInit() )
  {
    fprintf( stderr, "Failed to initialize GLFW\n" );
    getchar();
		is_init = false;
    return;
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Open a window and create its OpenGL context
	if(fullscreen){
  	window = glfwCreateWindow(pixelsX, pixelsY, "", glfwGetPrimaryMonitor(), NULL);
	}else
		window = glfwCreateWindow(pixelsX, pixelsY, "", NULL, NULL);

	//window = glfwCreateWindow( 1024, 1024, "", NULL, NULL);

  if( window == NULL ){
    fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
    getchar();
    glfwTerminate();
		is_init = false;
    return;
  }
	glfwMakeContextCurrent(window);

  // Initialize GLEW
  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    getchar();
    glfwTerminate();
		is_init = false;
    return;
  }

	// Create and compile our GLSL program from the shaders
	string mdir = string(module_dir);
	if(mdir.substr(mdir.length()-1, mdir.length()-1) != "/")
		mdir = mdir + "/";

  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  // black background
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  // Enable depth test
  glEnable(GL_DEPTH_TEST);

  // Accept fragment if it closer to the camera than the former one
  glDepthFunc(GL_LESS);

	//initialize our text variables
	initText2D( (mdir+"Holstein.DDS").c_str() , mdir.c_str(), max_x, max_y);

	//load shaders for our particles
  programID = LoadShaders((mdir+"shaders/Particle.vertexshader").c_str(),
	                        (mdir+"shaders/Particle.fragmentshader").c_str());
	if(programID == 0){
    getchar();
		glfwTerminate();
		is_init = false;
		return;
	}

  //load our texture
  Texture = loadPNG(texture_fp);

  // Vertex shader
  CameraRight_worldspace_ID  = glGetUniformLocation(programID, "CameraRight_worldspace");
  CameraUp_worldspace_ID  = glGetUniformLocation(programID, "CameraUp_worldspace");
  ViewProjMatrixID = glGetUniformLocation(programID, "VP");

  // fragment shader
  TextureID  = glGetUniformLocation(programID, "myTextureSampler");

  g_particule_position_size_data = new GLfloat[MaxParticles * 4];
  g_particule_stage_data = new GLfloat[MaxParticles * 2];

  for(int i=0; i<MaxParticles; i++){
    ParticlesContainer[i].pos = glm::vec3((rand()%1000-500.0f)*scale*maxX/500.0f,
                                          (rand()%1000-500.0f)*scale*maxY/500.0f,
                                          (rand()%2000-1000.0f)/4000.0f);
    glm::vec3 randomdir = glm::vec3(
      (rand()%2000 - 1000.0f)/1000.0f,
      (rand()%2000 - 1000.0f)/1000.0f,
      (rand()%2000 - 1000.0f)/1000.0f
    );

    ParticlesContainer[i].speed = randomdir;
    ParticlesContainer[i].stage = 0;
    ParticlesContainer[i].size = beeSize;

    //set the camera distance to current position - where camera is placed
    ParticlesContainer[i].cameradistance = glm::length2( ParticlesContainer[i].pos[2] -  camera_position[2]);
  }

  // The VBO containing the 4 vertices of the particles.
  // Thanks to instancing, they will be shared by all particles.
  static const GLfloat g_vertex_buffer_data[] = {
     -0.5f, -0.5f, 0.0f,
      0.5f, -0.5f, 0.0f,
     -0.5f,  0.5f, 0.0f,
      0.5f,  0.5f, 0.0f,
  };
  glGenBuffers(1, &billboard_vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

  // The VBO containing the information (position, size) of the particles
  glGenBuffers(1, &particles_position_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);

  // Initialize with empty (NULL) buffer : it will be updated later, each frame.
  glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

  // The VBO containing the information about stages of each of the particles
  glGenBuffers(1, &particles_stage_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, particles_stage_buffer);

  // Initialize with empty (NULL) buffer : it will be updated later, each frame.
  glBufferData(GL_ARRAY_BUFFER, MaxParticles * 2 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	//initialize our QR code variables
	init_qr(mdir);

	//initialize our
	is_init = true;
}


/*
	updates each of the GraphicsModule instance particles
	x, y, stage and direction must be the same size as the GraphicsModule field
	MaxParticles
*/
int GraphicsModule::update_particles(vector<int> x, vector<int> y, vector<int> stage, vector<int> direction){
  if((int)x.size() != MaxParticles || \
     (int)y.size() != MaxParticles || \
     (int)stage.size() != MaxParticles || \
     (int)direction.size() != MaxParticles){
    return -1;
  }

	//we always assume the passed particles are on a screen with
	//maxX/2, maxY/2 as the center, so we have to translate and scale them
	//to fit our 20x20 (0,0) centered world in openGL
  for(int i = 0; i < MaxParticles; i++){
		ParticlesContainer[i].pos[0] = to_opengl_world_x(x.at(i));
		ParticlesContainer[i].pos[1] = to_opengl_world_y(y.at(i));
    ParticlesContainer[i].stage = stage.at(i);
    ParticlesContainer[i].direction = direction.at(i);
  }
  return 0;
}

/*
update the window created by the instantiated GraphicsModule
based on the array of Particles maintained by the GraphicsModule instance

parameter :
	start_recording => whether or not we should run the command to
	start saving the video

returns :
	whether or not the the user can get the qr code

*/
bool GraphicsModule::update_display(bool start_recording){

	bool ret_val = false;

	if(!is_init)
		return ret_val;

  // Clear the screen
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Camera matrix
  ViewMatrix = glm::lookAt(
                camera_position,
                glm::vec3(0,0,0), // and looks at the origin
                glm::vec3(0,-1,0)  // Head is up (set to 0,-1,0 to look upside-down)
               );

	// Projection matrix : 45� Field of View, screen_ratio, display range (z coords): 0.1 unit <-> 100 units
  glm::mat4 ProjectionMatrix = glm::perspective(glm::radians(45.0f), screen_ratio, .1f, 100.0f);

  // We will need the camera's position in order to sort the particles
  // w.r.t the camera's distance.
  // There should be a getCameraPosition() function in common/controls.cpp,
  // but this works too.
  glm::vec3 CameraPosition(glm::inverse(ViewMatrix)[3]);
  ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;


	//code to render the QR code to the screen, if it has been enabled
	render_qr();


  // Simulate all particles
  for(int i=0; i<MaxParticles; i++){

    Particle& p = ParticlesContainer[i]; // shortcut

    p.cameradistance = glm::length2( p.pos - CameraPosition );

    // Fill the GPU buffer of positions
    g_particule_position_size_data[4*i+0] = p.pos.x;
    g_particule_position_size_data[4*i+1] = p.pos.y;
    g_particule_position_size_data[4*i+2] = p.pos.z;

    g_particule_position_size_data[4*i+3] = p.size;

    //fill the GPU buffer with stage information
    g_particule_stage_data[2*i+0] = p.stage;
    g_particule_stage_data[2*i+1] = p.direction;

  }

  SortParticles();


  // Update the buffers that OpenGL uses for rendering.
  // There are much more sophisticated means to stream data from the CPU to the GPU,
  // but this is outside the scope of this tutorial.
  // http://www.opengl.org/wiki/Buffer_Object_Streaming
  glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
  glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
  glBufferSubData(GL_ARRAY_BUFFER, 0, MaxParticles * sizeof(GLfloat) * 4, g_particule_position_size_data);

  glBindBuffer(GL_ARRAY_BUFFER, particles_stage_buffer);
  glBufferData(GL_ARRAY_BUFFER, MaxParticles * 2 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
  glBufferSubData(GL_ARRAY_BUFFER, 0, MaxParticles * sizeof(GLfloat) * 2, g_particule_stage_data);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Use our shader
  glUseProgram(programID);

  // Bind our texture in Texture Unit 0
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, Texture);
  // Set our "myTextureSampler" sampler to use Texture Unit 0
  glUniform1i(TextureID, 0);

  // Same as the billboards tutorial
  glUniform3f(CameraRight_worldspace_ID, ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
  glUniform3f(CameraUp_worldspace_ID   , ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);

  glUniformMatrix4fv(ViewProjMatrixID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

  // 1rst attribute buffer : vertices
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
  glVertexAttribPointer(
    0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
    3,                  // size
    GL_FLOAT,           // type
    GL_FALSE,           // normalized?
    0,                  // stride
    (void*)0            // array buffer offset
  );

  // 2nd attribute buffer : positions of particles' centers
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
  glVertexAttribPointer(
    1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
    4,                                // size : x + y + z + size => 4
    GL_FLOAT,                         // type
    GL_FALSE,                         // normalized?
    0,                                // stride
    (void*)0                          // array buffer offset
  );

  //3rd attribute buffer : information about particle stage
  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, particles_stage_buffer);
  glVertexAttribPointer(
    2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
    2,                                // size : x + y + z + size => 4
    GL_FLOAT,                         // type
    GL_FALSE,                         // normalized?
    0,                                // stride
    (void*)0                          // array buffer offset
  );


  // These functions are specific to glDrawArrays*Instanced*.
  // The first parameter is the attribute buffer we're talking about.
  // The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
  // http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
  glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
  glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
  glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1


  // Draw the particules !
  // This draws many times a small triangle_strip (which looks like a quad).
  // This is equivalent to :
  // for(i in MaxParticles) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4),
  // but faster.
  glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, MaxParticles);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);

	//text display
	glVertexAttribDivisor(1, 0); //reset the stride at which vertices are read in
	char text[256];
	sprintf(text,"%.2f sec", glfwGetTime() );
	for(unsigned i = 0; i < texts.size(); i++){
		printText2D(texts[i].text.c_str(), texts[i].x, texts[i].y, texts[i].size);
	}

	// Swap buffers
	glfwSwapBuffers(window);

	//video recording
	//if(start_recording && !record) {
	if(!(glfwGetKey(window, GLFW_KEY_R ) != GLFW_PRESS )  && !record){
		record = true;
		frame_count = 0;

		cout<<cmd;
		ffmpeg = popen(cmd.c_str(), "w");
		if(ffmpeg == NULL) {
			fprintf(stderr, "Could not open video file");
		}

	} else if (record == true) {//only save even frames?
		frame_count++;

		if(frame_count%2 == 0){
			void *pixels = malloc(pixelsX*pixelsY*3);
			glReadPixels(0, 0, (int)(pixelsX), (int)(pixelsY), GL_RGB, GL_UNSIGNED_BYTE, pixels);

			if (ffmpeg)
	    	fwrite(pixels, (int)(pixelsX*pixelsY*3), 1, ffmpeg);
			free(pixels);
		}
	}

	//if we are finished recording, upload video to google drive
	if(frame_count >= frame_total) {
		frame_count = 0;
		record = false;
		pclose(ffmpeg);

		char temp[255];
		getcwd(temp, sizeof(temp));
		std::string command = "sudo python " + std::string(temp) + "/graphics/generateQR.py &";
		cout << command << endl;
		system(command.c_str());

		ret_val = true;
	}

  glfwPollEvents();
	return ret_val;
}

/*
	adds a text struct to the class member vector<Text> texts
	this will cause the text to be rendered at the next call to update_display()
*/
void GraphicsModule::add_text(string text, int x, int y, float size){
	Text new_text;
	new_text.text = text;
	new_text.x = to_opengl_world_x(x);
	new_text.y = to_opengl_world_y(y);
	new_text.size = size;

	texts.push_back(new_text);
}

/*
	removes a text struct from our vector of texts, return false if the text is
	not found. Will cause the text to be removed at the next call to update_display()
*/
bool GraphicsModule::remove_text(string text){
	bool was_found = false;

	for(unsigned i = 0; i < texts.size(); i++){
		if(texts[i].text.compare(text) == 0){
			texts.erase(texts.begin() + i);
			was_found = true;
		}
	}

	return was_found;
}

/*
	initializes the values needed to render a QR code, called in
	GraphicsModule constructor

	input -> module_dir : the location of the graphics module wrt the calling
		                    program, should end with a '/'
*/
void GraphicsModule::init_qr(string module_dir){

	//load our shaders for the qr code
	QRProgramID = LoadShaders ( (module_dir+"shaders/QR.vertexshader").c_str(),
	                            (module_dir+"shaders/QR.fragmentshader").c_str() );

	//get handlers for variables that we will pass into our vertex shaders
	QRCameraRight_worldspace_ID  = glGetUniformLocation(QRProgramID, "CameraRight_worldspace");
  QRCameraUp_worldspace_ID  = glGetUniformLocation(QRProgramID, "CameraUp_worldspace");
	QRViewProjMatrixID = glGetUniformLocation(QRProgramID, "VP");
	QRPosID = glGetUniformLocation(QRProgramID, "QRPos");
	QRSizeID = glGetUniformLocation(QRProgramID, "QRSize");

	//create texture sampler for QR code
	QRTextureID  = glGetUniformLocation(QRProgramID, "myTextureSampler");

	//VBO containing 4 vertices for the qr code
	const static GLfloat g_vertex_buffer_data[] = {
		-0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		-0.5f,  0.5f, 0.0f,
		 0.5f,  0.5f, 0.0f,
	};
	glGenBuffers(1, &qr_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, qr_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_DYNAMIC_DRAW);
}

/*
	renders the QR code using the QR texture that was uploaded during updateQR
	updateQR must be called before this function
*/
void GraphicsModule::render_qr(){
	if(!qr_enabled)
		return;

	glDisable(GL_BLEND);

	glUseProgram(QRProgramID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, QRTexture);
	glUniform1i(QRTextureID, 0);

	glUniform3f(QRCameraRight_worldspace_ID, ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
	glUniform3f(QRCameraUp_worldspace_ID, ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);

	glUniform3f(QRPosID, (float)qr_x, (float)qr_y, 1.0f);
	glUniform2f(QRSizeID, (float)qr_size, (float)qr_size);

	glUniformMatrix4fv(QRViewProjMatrixID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, qr_vertex_buffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisableVertexAttribArray(0);
}

/*
	adds a qr code found at qrcode_fp to the bottom right of the screen,
	rendered on the next call to update_display

	enabled : whether the png should be displayed or not
	qrcode_fp : must contain filename of a png if enabled is set to true
*/
void GraphicsModule::update_qr(bool enabled, const char* qrcode_fp, int x, int y, int size){
	QRTexture = loadPNG(qrcode_fp);
	if((int)QRTexture != -1){
		qr_was_enabled = true;
		if(enabled && !qr_enabled){
			qr_x = to_opengl_world_x(x);
			qr_y = to_opengl_world_y(y);
			qr_size = size;
		}
		qr_enabled = enabled;
	}else{
		qr_enabled = false;
	}

	if(!enabled)
		qr_enabled = false;
}

/*
free the resources used by the graphics module
*/
void GraphicsModule::cleanup(){
	if(!is_init)
		return;

  free(ParticlesContainer);

  delete[] g_particule_position_size_data;

  // Cleanup VBO and shader
  glDeleteBuffers(1, &particles_position_buffer);
  glDeleteBuffers(1, &particles_stage_buffer);
  glDeleteBuffers(1, &billboard_vertex_buffer);
  glDeleteProgram(programID);
  glDeleteTextures(1, &Texture);

	//clean up for QR code resources
	if(qr_was_enabled){
		glDeleteBuffers(1, &qr_vertex_buffer);
		glDeleteProgram(QRProgramID);
		glDeleteTextures(1, &QRTexture);
	}

	//clean up for text resources
	texts.clear();
	cleanupText2D();

  // Close OpenGL window and terminate GLFW
  glfwTerminate();
}

/*
used to test whether the graphics module should close it's window.
Closes on escape, or some types of failure
*/
bool GraphicsModule::should_close(){
	if(!is_init)
		return true;

	return !(glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		     glfwWindowShouldClose(window) == 0 );
}
