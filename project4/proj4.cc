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

int speed,turn,targetX, targetY, endTargetX, endTargetY, originX, originY;
float normalX1, normalY1;
int normalX2, normalY2;
int nextGridPosX, nextGridPosY;
//the normal of the gridMap is 1000x10000
int normalX = 1000;
int normalY = 1000;

double distanceToObstacle;
PlayerClient * rob;
Position2dProxy * pos2d;
LaserProxy * las2d;
bool atTarget = false,toLeft = false, toRight = false, atOrigin = false;

#define SCALE_MAP 2
#define GRID_ROWS 1000 
#define GRID_COLS 1000
float gridMap[GRID_ROWS][GRID_COLS];
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
void checkFinalPos(float x, float y)
{
  if(x == targetX && y == targetY){
    atTarget = true;
  }
}

float normalize(float value, float min, float max)
{
    return abs((value-min) / (max-min));
}
void getNormal(int x, int y){
    float xPercent = normalize(x,0,normalX);
    float yPercent = normalize(y,0,normalY);
    float minX = -20;
    float maxX = 20;
    float minY = -9;
    float maxY = 9;
    normalX1 = (int) (xPercent*(abs(maxX - minX)) + minX);
    normalY1 = (int) (yPercent*(abs(maxY - minY)) + minY);
}
void invNormal (float x, float y)
{
    float xPercent = normalize(x, -20, 20);
    float yPercent = normalize(y, -9, 9);
    float minX = 0;
    float maxX = 1000;
    float minY = 0;
    float maxY = 1000;
    normalX2 = (int) (xPercent*(abs(maxX - minX)) + minX);
    normalY2 = (int) (yPercent*(abs(maxY - minY)) + minY);
}
void inputMap(int output) 
{
    int i, j, m, n;
    char inputLine1[80], nextChar;
    int width, height, maxVal;

    ifstream inFile("hospital_section.pnm");

    /* Initialize map to 0's, meaning all free space */

    for (m=0; m<GRID_ROWS; m++)
        for (n=0; n<GRID_COLS; n++)   
            gridMap[m][n] = 0.0;

    /* Read past first line */
    inFile.getline(inputLine1,80);

    /* Read in width, height, maxVal */
    inFile >> width >> height >> maxVal;
    cout << "Width = " << width << ", Height = " << height << endl;

    /* Read in map; */
    for (i=0; i<height; i++)    
        for (j=0; j<width; j++) {
	  inFile >> nextChar;
	  if (!nextChar){
	    gridMap[i/SCALE_MAP][j/SCALE_MAP] = 1.0;
	    for(int k=1;k<2;k++){
		gridMap[(i+k)/SCALE_MAP][(j+k)/SCALE_MAP] = 1.0;
		for(int l=0;l<3;l++){
		    gridMap[(i+k+l)/SCALE_MAP][(j+k+l)/SCALE_MAP] = 1.0;
		    gridMap[(i+k-l)/SCALE_MAP][(j+k-l)/SCALE_MAP] = 1.0;
		}
		gridMap[(i-k)/SCALE_MAP][(j+k)/SCALE_MAP] = 1.0;
		gridMap[(i+k)/SCALE_MAP][(j-k)/SCALE_MAP] = 1.0;
		gridMap[(i-k)/SCALE_MAP][(j-k)/SCALE_MAP] = 1.0;
		    }
	  }
	}
    cout << "Map input complete.\n";

    if (output)  {
      ofstream outFile("scaled_hospital_section.pnm");
      outFile << inputLine1 << endl;
      outFile << width/SCALE_MAP << " " << height/SCALE_MAP << endl
	      << maxVal << endl;

      for (i=0; i<height/SCALE_MAP; i++)
	for (j=0; j<width/SCALE_MAP; j++) {
	  if (gridMap[i][j] == 1.0)
	    outFile << (char) 0;
	  else
	    outFile << (char) -1;
	}
       cout << "Scaled map output to file.\n";
    }
}
/**
void avoid_obstacle(double distanceToObst, double dist, double ang)
{
    
}
void Act(double direction, double distance)
{
    rob->Read();
    
}
**/
bool isInRange(int x, int y){
  if(x > 0 && x < 999 && y > 0 && y < 999){
	return true;
  }else{
	return false;
  }
}
void wave(int x, int y, int count)
{
  //first check if at origin
  if(x == originX && y == originY){
	atOrigin = true;
  }
  gridMap[x][y] = count;
  bool l = false;
  bool r = false;
  bool u = false;
  bool d = false;
    if(isInRange(x-1,y) && (gridMap[x-1][y] == 0 || gridMap[x-1][y] > 999)){
	l = true;
	gridMap[x-1][y] = count;
    }
    if(isInRange(x+1,y) && (gridMap[x-1][y] == 0 || gridMap[x+1][y] > 999)){
	r = true;
	gridMap[x+1][y] = count;
    }
    if(isInRange(x,y-1) && (gridMap[x][y-1] == 0 || gridMap[x][y-1] > 999)){
	u = true;
	gridMap[x][y-1] = count;
    }
    if(isInRange(x,y+1) && (gridMap[x][y+1] == 0 || gridMap[x][y+1] > 999)){
	d = true;
	gridMap[x][y+1] = count;
    }

    count++;
    if(l){
	wave(x-1,y,count);
    }
    if(r){
	wave(x+1,y,count);
    }
    if(u){
	wave(x,y-1,count);
    }
    if(d){
	wave(x,y+1,count);
    }
	//...
}
void Wavefront(int x, int y, int x1, int y1)
{
  atOrigin = false;
  int count = 2;
  //read in current robot x y position
  std::cout << "testing" << std::endl;
  //originX and originY are where I am in terms of the gridMap
  originX = x;
  originY = y;
  //target waypoint is x1, y1
  //do this using the gridMap[][]
  do{
    //start from goal
    //gridMap[x][y]

    wave(x,y,2);
  }while(!atOrigin);
}
void getNextGridPos(int x, int y)
{
  //get values around this point
  int left = x-1;
  int right = x+1;
  int up = y-1;
  int down = y+1;
  int num = gridMap[x][y] + 1;
  int newX = x, newY = y;
  //find a value of which gridMap[][] = gridmap[x][y]+1
  if(gridMap[left][y] == num){
    newX = left;
  }else if(gridMap[right][y] == num){
    newX = right;
  }else if(gridMap[x][up] == num){
    newY = up;
  }else if(gridMap[x][down] == num){
    newY = down;
  }

  nextGridPosX = newX;
  nextGridPosY = newY;
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

      rob->Read();
      //get my position
      originX = pos2d->GetXPos();
      originY = pos2d->GetYPos();
      //check if were at the final pos
      //checkFinalPos(originX, originY);
      //Wavefront(x,y);
      //start mapping, we already have the wavefront and normalization
      //gridMap[] has the values to go to next.
      //first let's find out where to go to next. reverse normalize my value
	std::cout << "originX: " << originX << " originY: " << originY << std::endl;
      //to the grid value
      invNormal(x,y);
      int goalX = normalX2;
      int goalY = normalY2;
      std::cout << "goalX: " << goalX << " goalY: " << goalY << std::endl;
      invNormal(originX,originY);
      int myPosX = normalX2;
      int myPosY = normalY2;
      std::cout << "myPosX: " << myPosX << " myPosY: " << myPosY << std::endl;
      //this is the value in the gridMap that should be 2, the "origin" on the gridMap
      std::cout << normalX2 << " " << normalY2 << std::endl;
      Wavefront(myPosX, myPosY, goalX, goalY);
      //wavefront planning finished
      //Now we can start the process of moving
      //we are at position gridMap[normalX2][normalX2], the first time, this is 2

    do{
      std::cout << gridMap[normalX2][normalY2] << std::endl;
      //we need to find the next value from this value by searching at this layer 
      //and looking for the value+1
      getNextGridPos(normalX2, normalY2);
      rob->Read();
      //newGridPos values hold the new gridPos I'm looking for
      //we need to normalize the grid values to plot values
      getNormal(nextGridPosX, nextGridPosY);
      //new normals in normalX1 and normalX2
      std::cout << "nextGridPosX :" << nextGridPosX << " nextGridPosY:" << nextGridPosY << std::endl;
      pos2d->GoTo(normalX1, normalY1, 0);
      //now we have to redo the normalX2 and normalY2
      invNormal(pos2d->GetXPos(),pos2d->GetYPos());
      checkFinalPos(pos2d->GetXPos(),pos2d->GetYPos());
      
    }while(!atTarget);
    atTarget = false;

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
    targetX = waypoints[i];
    targetY = waypoints[i+1];
    Pilot(waypoints[i],waypoints[i+1]);
  }
}

//set up
int main(int argc, char **argv)
{
  inputMap(1);
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
