#include <iostream>
#include "car.h"
#include "model.h"
#include "transform.h"
#include "camera.h"
#include <StopWatch.h>
#include <math.h>
#define NEAR_ZERO 0.05
#define WHEEL_ROTN_STEP 0.01
using namespace std;


void Car::calculateCircle()
{
  float thetaCar =	m3dDegToRad(carRotationAngle);

	// calculating new circle of rotation
	float prev_radius = radius;
	float thetaWheel = m3dDegToRad(wheelAngle);

	// preventing INF
	if(wheelAngle > - NEAR_ZERO && wheelAngle < NEAR_ZERO)	
	{
		if (prev_radius > 0)
			radius = INFINITY;
		else
			radius = -INFINITY;
	}
	else
	{
		radius = wheelBase / tan(thetaWheel);	
	}
	circleCenter.x = (prev_radius - radius) * cos(thetaCar) + circleCenter.x;
	circleCenter.z = circleCenter.z - ((prev_radius - radius) * sin(thetaCar));
}


void Car::updateAngles()
{
	mat3 rotn = Transform::rotate(wheelAngle, myCamera.up);

	calculateCircle();		
		
	// turning only the front wheels left
	for(int i = 0; i < 2; i++)
	{
		frontWheel[i].rotate = mat4(rotn);
		frontWheel[i].transform = carBody[0].transform * frontWheel[i].translate * frontWheel[i].rotate;		
	}
}
void Car::turnLeft(float dt)
{
	// limiting the rotaing of wheel and increasing wheelAngle
	if(wheelAngle < wheelTurnLimit)
	{
		wheelAngle = wheelAngle + wheelStep * dt;
		//cout << "\n\nturning left";
	}
}

void Car::turnRight(float dt)
{
	// limiting the rotaing of wheel and decreasing wheelAngle
	if(wheelAngle > -wheelTurnLimit)
	{
		wheelAngle = wheelAngle - wheelStep * dt;
		//cout << "\n\nturning right";
	}		
}


float signum(float x)
{
	if (x < 0)
		return -1;
	else
		return 1;
}

void Car::updatePosition()
{
	// prev_wheelAngle note :
	// every frame there is an effort to make the wheelAngle to 0, now when after non zero 
	// value the wheelAngle becomes 0, whether to increase carRotationAngle or decrease it 
	// depends on the value of wheelAngle before it becomes 0, if it was negative then then
	// carRotationAngle has to be decreased as anticlockwise on right circle, therefore
	// prevWheelAngle stores the value of wheelAngle before it becomes 0.

	static float prev_wheelAngle = 1;

	// the time elapsed since last frame
	float time = timer.GetElapsedSeconds();
	timer.Reset();	// reset the timer so that in next frame we get time elapsed


	// set constant acceleration if the car is accelerating (user has pressed the up key)
	float acceleration;
	if (accelerating)
		acceleration = USER_ACCEL;
	else
		acceleration = 0;

	if (turningLeft == false && turningRight == false) 
	{
		const float alpha = 0.01;
		float d_alpha = alpha * time * linearVelocity;

		wheelAngle = wheelAngle * (1-d_alpha);
				
		cout << "\n\n\n" << wheelAngle; 
	}
	else
	{
		if (turningLeft)
			turnLeft(time);
		if (turningRight)
			turnRight(time);
	}

	// prev_wheelAngle is the last angle of wheel before its gradually reduced to 0
	if(wheelAngle != 0)
		prev_wheelAngle = wheelAngle;

	// need to update variuous angles based on wheel angle
	updateAngles();	
	
	linearVelocity = linearVelocity + (acceleration * time);


	float decreaseInVelocity = (constFriction + proportionateFriction * linearVelocity * linearVelocity) * time;

	if (decreaseInVelocity > linearVelocity) {
		linearVelocity = 0;
	}
	else {
		linearVelocity = (abs(linearVelocity) - decreaseInVelocity) * signum(linearVelocity);
	}

	float angularVelocity = linearVelocity / abs(radius);
		
	mat3 rotn;
	// moving forward while turning left

	float angleStep = 0;
	angleStep = (angularVelocity * time);

	if(wheelAngle > 0)
	{	
		carRotationAngle = carRotationAngle + angleStep;
		rotn = Transform::rotate(angleStep, myCamera.up);
	}

	// moving forward while turning right
	if(wheelAngle < 0)
	{				
		carRotationAngle = carRotationAngle - angleStep;
		rotn = Transform::rotate(-angleStep, myCamera.up);
	}

	// wheelAngle is 0, then to decide whether to add or substract angleStep we have to see what the previous wheelAngle was, so according to that angle we can see which direction its going
	if(wheelAngle == 0)
	{
		if(prev_wheelAngle > 0)
		{
			carRotationAngle = carRotationAngle + angleStep;
			rotn = Transform::rotate(angleStep, myCamera.up);
		}
		if(prev_wheelAngle < 0)
		{
			carRotationAngle = carRotationAngle - angleStep;
			rotn = Transform::rotate(-angleStep, myCamera.up);
		}
	}
		
	
	// cout << "radius: " << radius << " thetaWheel " << wheelAngle << " angle: " << carRotationAngle << " anglestep: " << angleStep << "\n";	


	// traslating origin to change circleCenter as car rotates about the circle of rotation 	
	vec4 positionNew;
	positionNew = carBody[0].position - circleCenter;
	positionNew = mat4(rotn) * positionNew;
	
	carBody[0].position = positionNew + circleCenter;

	// compute rotn of the car
	mat3 rotnCar = Transform::rotate(carRotationAngle, myCamera.up);	


	// rotaring the wheel as they roll on land
	mat3 rotnWheel = Transform::rotate(wheelRevolveAngle, vec3(1, 0, 0));

	
	// TODO : change this so that this is linked with speed of car
	// faster the car moves, faster the wheels rolls/ revolves 
	wheelRevolveAngle = wheelRevolveAngle - linearVelocity * WHEEL_ROTN_STEP;
	

	// calculating tranlation of the car, by seeing its position
	mat4 transl = Transform::translate(carBody[0].position.x, carBody[0].position.y, carBody[0].position.z);	

	// actually affecting transform of the car
	carBody[0].transform =  transl * mat4(rotnCar);	
	carBody[1].transform =  transl * mat4(rotnCar);	

	// transform the 4 wheels
	frontWheel[0].transform =  carBody[0].transform * frontWheel[0].translate * frontWheel[0].rotate * mat4(rotnWheel);
	frontWheel[1].transform = carBody[0].transform * frontWheel[1].translate * frontWheel[1].rotate  * mat4(rotnWheel) ;

	rearWheel[0].transform = carBody[0].transform * rearWheel[0].translate * mat4(rotnWheel);	
	rearWheel[1].transform = carBody[0].transform * rearWheel[1].translate * mat4(rotnWheel);	

	// rotate the third person camera like you rotated the car
    myCamera.rotateCamera(circleCenter, rotn, carCentre);
}


void Car::initializeCar()
{
	float amb[4] = {1, 1, 1, 1};
	float diff[4] = {0.5, 0.5, 0.5, 1};
	float spec[4] = {1, 1, 1, 1};
	float shine = 120;
	mat4 transf(1.0);
	myCar.carBody[0].initializeModelFromObjectFile("carExt.obj", "rose.tga", amb, diff, spec, shine, transf);	

	transf = Transform::translate(4, 2 , -6.3);
	myCar.frontWheel[0].initializeModelFromObjectFile("wheel.obj", "t1.tga", amb, diff, spec, shine, transf);

	transf = Transform::translate(-4, 2 , -6.3);
	myCar.frontWheel[1].initializeModelFromObjectFile("wheel.obj", "t1.tga", amb, diff, spec, shine, transf);

	transf = Transform::translate(4, 2 , 7);	
	
	myCar.rearWheel[0].initializeModelFromObjectFile("wheel.obj", "t1.tga", amb, diff, spec, shine, transf);

	transf = Transform::translate(-4, 2 , 7);
	myCar.rearWheel[1].initializeModelFromObjectFile("wheel.obj", "t1.tga", amb, diff, spec, shine, transf);
	
	amb[0] = 0.3;
	amb[1] = 0.1;
	amb[2] = 0.2;
	myCar.carBody[1].initializeModelFromObjectFile("carInt.obj", "sofa.tga", amb, diff, spec, shine, transf);
}
