#include <glm/glm.hpp>
#include <StopWatch.h>
#include "model.h"
#define INFINITY 10000
#define USER_ACCEL 5000
static const float wheelTurnLimit = 60.0f;

static const float constFriction = 0.01;
static const float proportionateFriction = 0.0005;
static const float wheelStep = 100;


typedef glm::mat3 mat3 ;
typedef glm::mat4 mat4 ; 
typedef glm::vec3 vec3 ; 
typedef glm::vec4 vec4 ; 

class Car
{
  public:
	Model carBody[2];
	Model frontWheel[2];
	Model rearWheel[2];


	// TODO: make all of these local variables if possible

	float wheelAngle;
	float carRotationAngle;		
	float wheelRevolveAngle;
	float wheelBase;

	float linearVelocity;
	float acceleration;
	vec3 carCentre;
		
	float radius;
	vec4 circleCenter;
	CStopWatch timer;

	void calculateCircle();
	void updateAngles();

	void updatePosition();
	
	void turnLeft(float dt);
	void turnRight(float dt);


	bool turningLeft;
	bool turningRight;
	bool accelerating;

	void initializeCar();
	
	Car::Car()
	{	
		carRotationAngle = 0;
		wheelAngle = 0;
		wheelRevolveAngle = 0;
		radius = INFINITY;
		circleCenter = vec4(-radius, 0, 0, 1);
		acceleration = 0;
		linearVelocity = 0;
		wheelBase = 20;
	}
};
extern Car myCar;
