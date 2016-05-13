#include <libplayerc++/playerc++.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <cmath>
#include "args2.h"
#include <string>
#define PI 3.14159265359
#define RAYS 32
using namespace std;
using namespace PlayerCc;
int speed,turn;
PlayerClient * rob;
Position2dProxy * pos2d;
bool atTarget = false,toLeft = false, toRight = false;
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
    rob = &robot;
    pos2d = &pp;

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
    //std::cout << "distance is :" << distance << std::endl;
    //std::cout << "angle is" << angle << std::endl;
    if(angle < pp.GetYaw()*180/PI)
    {
      toLeft = true;
    }
    else
    {
      toLeft = false;
    }
    Act(angle* PI/180, distance);

    }while(!atTarget);

  }
  catch (PlayerCc::PlayerError & e)
  {
    std::cerr << e << std::endl;
  }
}
void Navi(double waypoints[],int size)
{
  for(int i=0;i<size;i+=2)
  { 
    //std::cout << "Processing " << waypoints[i] << "," << waypoints[i+1] <<std::endl;
    Pilot(waypoints[i],waypoints[i+1]);
    atTarget = false;
  }
}

//set up
int main(int argc, char **argv)
{
  parse_args(argc,argv);
  string line;
  ifstream myfile(argv[1]);
  double data[100];
  int i=0, count = 0;
  
  if(myfile.is_open())
  {
    myfile >> data[i] >> data[i+1];

        i+=2;
    while( getline(myfile,line) )
    {
      myfile >> data[i] >> data[i+1];

      i+=2;
      count = i;
    }
    myfile.close();
  }
  else
  {
    std::cout << "Could not open File" << std::endl;
  }
  count-=2;
  //put array of values into a new array
  //successfully read in text file
  //now we can use it
  double*nums = NULL;
  nums = new double[count];
  for(int i=0;i<count;i++){
    nums[i] = data[i];
  }
  //pass array to navigator
  Navi(nums, count);
  return -1;

}
