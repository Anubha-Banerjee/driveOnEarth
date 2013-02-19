#ifndef MODEL_H
#define MODEL_H

#include <glm/glm.hpp>
#include <GLTools.h>

typedef glm::mat3 mat3 ;
typedef glm::mat4 mat4 ; 
typedef glm::vec3 vec3 ; 
typedef glm::vec4 vec4 ; 

class Model {
public:
  GLBatch batch;
	float size ;
	GLfloat ambient[4]; 
	GLfloat diffuse[4];
	GLfloat specular[4]; 
	GLfloat shininess;
    mat4 transform ;
	mat4 translate;
	mat4 rotate;
	mat4 scale;
	vec4 position, positionPrev;
	GLuint texture;

	// fills the batch and initialize the colors, transforms
	void initializeModelFromObjectFile(char *objectFile, char * textureFile, float amb[4], float diff[4], float spec[4], float shine, mat4 transf);
	void initializeModelFromFloatArray(GLfloat arr[1000000][3], int numberOfTriangles,  char * textureFile, float amb[4], float diff[4], float spec[4], float shine, mat4 transf);
} ;

extern Model land, roads, joints;
#endif
