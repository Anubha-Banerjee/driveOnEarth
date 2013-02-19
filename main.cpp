#include <iostream>
#include <string>
#include <GL/glew.h>
#include <GL/glut.h>
#include "connection.h"
#include "camera.h"
#include "model.h"
#include "map.h"
#include "car.h"
#include "sceneVariables.h"

#define WIDTH 1000
#define HEIGHT 1000

Model land, roads, joints;
Camera myCamera;
Car myCar;
Map map;

GLuint test;
GLint shader;
GLuint lightposn; 
GLuint lightcolor; 
uniform colorUniform;

void display();

void reshape(int width,int height){
  
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(60,width/(float)height,0.1,1000);
	glViewport(0,0,width,height);
}


void initializeLand()
{
	float amb[4] = {1, 1, 1, 1};
	float diff[4] = {0.5, 0.5, 0.5, 1};
	float spec[4] = {0, 0, 0, 1};
	float shine = 200;

	// the scale of the land
	mat4 transf(1);

	land.initializeModelFromObjectFile("bigLand.obj", "brown.tga", amb, diff, spec, shine, transf);	
}

void setup()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glDisable(GL_CULL_FACE);


	// create the land object
	initializeLand();

	// Initialize shaders
	shader=gltLoadShaderPairWithAttributes("ver.vp","frag.fp",3,GLT_ATTRIBUTE_VERTEX,"vertex",GLT_ATTRIBUTE_NORMAL,"normal",GLT_ATTRIBUTE_TEXTURE0,"texture");			

	lightposn = glGetUniformLocation(shader,"lightposn") ;       
    lightcolor = glGetUniformLocation(shader,"lightcolor") ;
    colorUniform.ambientcol = glGetUniformLocation(shader,"ambient") ;       
    colorUniform.diffusecol = glGetUniformLocation(shader,"diffuse") ;       
    colorUniform.specularcol = glGetUniformLocation(shader,"specular") ;  
    colorUniform.shininesscol = glGetUniformLocation(shader,"shininess") ; 


	FILE *fp = fopen ("d://data.txt", "r+");
	glClearColor(0.603922, 0.803922, 0.196078, 1);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(-WIDTH/2, WIDTH/2, -HEIGHT/2, HEIGHT/2);
															
	// take in input coordinates
	map.inputCoordinates();

	// create the query to retrive data from server 
	std::string query = createQuery(map.map_latitude, map.map_longitude);

	// connect and retrive the response data which will be saved to the disk on a file
	// connect_and_save(query, fp);

	map.calculateShift(map.map_latitude, map.map_longitude);

	// store the nodes and ways in array data structure
	map.store_nodes_in_array(fp);
	map.store_ways_in_array(fp);

	// store the rectangle coordintes in rectangle data structure
	map.store_rect_coords_in_array();

	// store the rectanlges in the batch which we will draw, from rectangle data structure
	//store_rects_in_batch();

	map.store_rects_in_float_array();

	float amb[4] = {1, 1, 1, 1};
	float diff[4] = {0.5, 0.5, 0.5, 1};
	float spec[4] = {0, 0, 0, 1};
	float shine = 200;
	mat4 transf(1.0);

	roads.initializeModelFromFloatArray(map.rectArray, 6, "t1.tga", amb, diff, spec, shine, transf);
	joints.initializeModelFromFloatArray(map.jointArray, 12, "t1.tga", amb, diff, spec, shine, transf);
	myCar.initializeCar();

	// intialise camera
	myCamera.eye = vec3(0, 500,10);
	myCamera.up = vec3(0,1,0);
	myCamera.goal = vec3(0,0,0);
}

void loopStep()
{
	myCar.updatePosition();
	display();
}

void keyPressed (unsigned char key, int x, int y)
{
	if(key == 27)
	{
		exit(0);
	}	
}

void specialUp(int key, int x, int y) {
	switch(key) {
	case 100: //left
		myCar.turningLeft = false;
		break;
	case 101: //up
		myCar.accelerating = false;
		break;
	case 102: //right
		myCar.turningRight = false;
        break;
	case 103: //down	
	    break;
	}	
}

void specialDown(int key, int x, int y) {	

	switch(key) {
	case 100: //left
		myCar.turningRight = false;
		myCar.turningLeft = true;
		break;
	case 101: //up
		myCar.accelerating = true;
		break;
	case 102: //right
		myCar.turningRight = true;
		myCar.turningLeft = false;
        break;
	case 103: //down	
		break;
	}	
}

void main(int argc,char **argv)
{
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowPosition(0,0);

	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("3d map land");	
	//glutFullScreen();

	glutDisplayFunc(loopStep);
	glutReshapeFunc(reshape);

	
	glutKeyboardFunc(keyPressed);
	glutSpecialFunc(specialDown);
    glutSpecialUpFunc(specialUp);
	

	glewInit();
	setup();
	glutMainLoop();
}
