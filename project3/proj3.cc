#include <libplayerc++/playerc++.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include <cmath>
#include "args2.h"
#include <string>
#define PI 3.14159265359
#define RAYS 32
using namespace std;
using namespace PlayerCc;

struct vec{
	double magnitude;
	double direction;
};
vec operator+(vec a, vec b){
    vec accumVec;
    accumVec.magnitude = (a.magnitude + b.magnitude)/2;
    accumVec.direction = a.direction + b.direction;
    return accumVec;
}
//D is the distance we want to be the buffer
//d is the current distance
//after reading in each obstacle, we want to
//sum the vectors, and move

vec repulsive(double angle, double d, double D){
	vec outputVector;
	if(d<=D){
		outputVector.direction = angle-180;
		outputVector.magnitude = (D-d)/D;
	}else{
		outputVector.direction = 0.0;
		outputVector.magnitude = 0.0;
	}
	return outputVector;
}

int speed,turn;
double distanceToObstacle;
vec myDirection;
vec myVec;
PlayerClient * rob;
Position2dProxy * pos2d;
LaserProxy * las2d;
bool atTarget = false,toLeft = false, toRight = false;
void avoid_obstacle(double distanceToObst, double dist, double ang)
{
    //turn the distance and angle we have into our current vector heading
    vec myVec;
    myVec.magnitude = dist;
    myVec.direction = ang;
    //Now we need the vector for the obstacle
    
      //now we have distance(magnitude) but not direction.
      //to get direction, 
      double angleOfObstacle = -1;
      //obstacle is between -90 and 0
      //but reading from the getRange takes it from right to left in terms of the robot's vision
      for(double min = 0;min<=180;min++)
      {
	  double rangeValue = las2d->GetRange(min);
	  //std::cout << "Min: " << min << ", This Range Value: " << rangeValue << std::endl;
	  if(rangeValue == distanceToObst){
	    //found the correct value
	    std::cout << "correct value found:" << min << std::endl;
	    angleOfObstacle = min;
	  }
	}
	vec obstacleVec = repulsive(angleOfObstacle,1, distanceToObst);
	vec SumVector = obstacleVec + myVec;
	std::cout << "SumVector Magnitude: " << SumVector.magnitude << ", SumVector Direction: " << SumVector.direction << std::endl;
	pos2d->SetSpeed(SumVector.magnitude, SumVector.direction);

}
void Act(double direction, double distance)
{
    rob->Read();
    if(!toLeft)
    {
      if(distance <5) 
      {
	pos2d->SetSpeed(5,10);

      }
      else
      {
        pos2d->SetSpeed(10,10);
	
      }
    }
    else{
      if(distance <5)
      {
	pos2d->SetSpeed(5,-10);
      }
      else
      {
	pos2d->SetSpeed(10,-10);
      }
    }
}
void Pilot(double x, double y)
{
  
  //recieved x and y coordinates to go to
  //make the robot to get coordinates from for our current position
  try
  {
    //initialized robot and 2d position
    PlayerClient robot(gHostname, gPort);
    Position2dProxy pp(&robot, gIndex);
    LaserProxy lp(&robot, gIndex);
    rob = &robot;
    pos2d = &pp;
    las2d = &lp;

    pp.SetMotorEnable (true);
    //x and y pos of current state
    double myPosX;
    double myPosY;
    do
    {

      robot.Read();
          //x and y pos of current state
      myPosX = pp.GetXPos();
      myPosY = pp.GetYPos();
      //std::cout << robot << std::endl;
      //std::cout << "My pos is: " << myPosX << "," << myPosY << std::endl;
      //std::cout << "WayPoint is: " << x << "," << y << std::endl;
      double xDist = x - myPosX;
      double yDist = y - myPosY;
      //std::cout << "distance is " << xDist << " by " << yDist << std::endl;
      //yaw is current angle we are facing
      //std::cout << "Yaw is " << pp.GetYaw()*180/PI << std::endl;
      double distance = sqrt(pow(xDist,2) + pow(yDist,2));
      //angle is angle in respect to waypoint
      double angle = atan2(yDist,xDist)* 180/PI;
      if(distance < 1.0)
      {
        //stop when close enough
        atTarget = true;
        //read final position
	std::cout << "Final position of WayPoint (" << x << "," << y << ") is " << "(" << myPosX << "," << myPosY << ")" << std::endl;
      }

      bool left = true;
      //min left and min right of field of vision
      double minR = las2d->GetMinRight();
      double minL = las2d->GetMinLeft();
      //compare left to right
      if(minR < minL){
	bool left = false;
	distanceToObstacle = minR;
      }else{
	distanceToObstacle = minL;
      }

    if(angle < pp.GetYaw()*180/PI)
    {
      toLeft = true;
    }
    else
    {
      toLeft = false;
    }
      if(distanceToObstacle < 2){
	//engage obstacle avoid
	avoid_obstacle(distanceToObstacle,distance,angle);
        
      }else{
          Act(distance, angle);
      }
    //std::cout << "distance is :" << distance << std::endl;
    //std::cout << "angle is" << angle << std::endl;

    }while(!atTarget);

  }
  catch (PlayerCc::PlayerError & e)
  {
    std::cerr << e << std::endl;
  }
}

//set up
int main(int argc, char **argv)
{
  int x;
  int y;
  parse_args(argc,argv);
  string line;
  ifstream myfile(argv[1]);
  //arg[0] is ./proj
  //arg[1] is first coordinate
  //arg[2] is the second coordinate
  x = atoi(argv[1]);
  y = atoi(argv[2]);
  std::cout << argv[1] << " " << argv[2] << std::endl;

  //pass array to navigator
  Pilot(x,y);
  return -1;

}
