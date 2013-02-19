#include <GL/glew.h>
#include <GL/glut.h>
#include "model.h"
#include "car.h"
#include "sceneVariables.h"
#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>



void transformvec (const GLfloat input[4], GLfloat output[4]) 
{
    GLfloat modelview[16]; // in column major order
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview); 

    for (int i = 0 ; i < 4 ; i++) {
        output[i] = 0; 
        for (int j = 0 ; j < 4 ; j++) {
            output[i] += modelview[4*j+i] * input[j]; 
        }
    }
}

void setUniform(mat4 transform, GLfloat amb[4], GLfloat diff[4], GLfloat spec[4], float shine, GLuint texture)
{
  glLoadMatrixf(&transform[0][0]);
	glUniform4fv(colorUniform.ambientcol,1,amb) ; 
	glUniform4fv(colorUniform.diffusecol,1,diff) ; 
	glUniform4fv(colorUniform.specularcol,1,spec) ; 
	glUniform1fv(colorUniform.shininesscol,1,&shine) ;		
	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );	
	glBindTexture(GL_TEXTURE_2D,texture);	
}
void plot_point(double x, double y) {
   
	glColor3f(0,0,0);
	glLineWidth(2);
	glBegin(GL_LINES);
	glColor3f(0,0,1); 
	glVertex2f(x, y);	
	glVertex2f(0, 0);
	glEnd();
}


void display() 
{
	
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE );
	glClearColor(0, 0.4, 0.6, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		
	
	glMatrixMode(GL_MODELVIEW);
	mat4 mv, transform(1.0);	
	mv = glm::lookAt(myCamera.eye, myCamera.goal, myCamera.up); 		

	glLoadMatrixf(&mv[0][0]); 
	
	glUseProgram(shader);
	
	const GLfloat light_positional[] = {0, 200, -250, 1}; 
	const GLfloat color[] =  {0.6,0.3,0,1};   

	GLfloat light[4] ;
	transformvec(light_positional, light) ;
	
	// setting light position and color
	glUniform4fv(lightposn, 1, light) ; 	
	glUniform4fv(lightcolor,1,color);
	
	// render the land
	transform = mv * land.transform;
	setUniform(transform, land.ambient, land.diffuse, land.specular, land.shininess, land.texture);
	land.batch.Draw();		

	glColor3f(1, 0, 0);

	setUniform(transform, roads.ambient, roads.diffuse, roads.specular, roads.shininess, roads.texture);
	roads.batch.Draw();
	joints.batch.Draw();


	// render the car body
	for(int i = 0; i < 2; i++)
	{ 		
		transform = mv * myCar.carBody[i].transform;
		setUniform(transform, myCar.carBody[i].ambient, myCar.carBody[i].diffuse, myCar.carBody[i].specular, myCar.carBody[i].shininess, myCar.carBody[i].texture);
		myCar.carBody[i].batch.Draw();		
	}

	// render the frontwheels
	for(int i = 0; i < 2; i++)
	{ 		
		transform = mv * myCar.frontWheel[i].transform;
		setUniform(transform, myCar.frontWheel[i].ambient, myCar.frontWheel[i].diffuse, myCar.frontWheel[i].specular, myCar.frontWheel[i].shininess, myCar.frontWheel[i].texture);
		myCar.frontWheel[i].batch.Draw();		
	}

	// render the rearwheels
	for(int i = 0; i < 2; i++)
	{ 		
		transform = mv * myCar.rearWheel[i].transform;
		setUniform(transform, myCar.rearWheel[i].ambient, myCar.rearWheel[i].diffuse, myCar.rearWheel[i].specular, myCar.rearWheel[i].shininess, myCar.rearWheel[i].texture);
		myCar.rearWheel[i].batch.Draw();		
	}


	glutSwapBuffers();
	glutPostRedisplay();
}
