#ifndef TEXTURE_HPP
#define TEXTURE_HPP

// Load a .BMP file using our custom loader
GLuint loadBMP_custom(const char * imagepath);

GLuint loadPNG(const char* filename);

GLuint loadDDS(const char* filename);

#endif
