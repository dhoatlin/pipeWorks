/*

	Pipeworks is a program that bounces a drone around inside a box.

	The trick is there are four quadrants to the box, each controlled
	by its own child process.
	
	drone.c
	
	drone handles all the movements and locations of the object bouncing
	of walls.
	
	AUTHOR: Dave Hoatlin
	DATE: 2/18/11
*/

#include "drone.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

Drone globalDrone;

/*
	Create a drone.
	
	The x and y parameters are the maximum height and width of the drawing area
*/
Drone createDrone(int xMin, int xMax, int yMin, int yMax)
{
	Drone drone;
	drone = getRandDir(drone);
	drone.locX = randomize(xMin+1, xMax-1);
	drone.locY = randomize(yMin+1, yMax-1);
	return drone;
}

/*
	seed the random function
*/
void seedDroneRand()
{
	srand48(time(NULL));
}

/* 
	generate a random number between a and b
*/
int randomize (int a, int b) {
    return (int) (a + drand48 () * (b - a));
}

/*
	generate a random number to be used for the direction of the drone
*/
Drone getRandDir(Drone drone)
{
    if (randomize (0, 11) < 5) {
    	drone.dirR = 1;
    	drone.dirC = randomize (-4, -1);
    }
    else {
    	drone.dirR = randomize (-4, -1);
    	drone.dirC = 1;
    }
    return drone;
}

/*
	move the drone a certain distance specified by the parameters
*/
Drone moveD(int x, int y, Drone drone)
{
	drone.locX += x;
	drone.locY += y;
	return drone;
}

/*
	inverts the Y direction of the drone
	used when the drone bounces off a wall
*/
Drone invertDirR(Drone drone)
{
	drone.dirR *= -1;
	return drone;
}

/*
	inverts the X direction of the drone
	used when the drone bounces off a wall
*/
Drone invertDirC(Drone drone)
{
	drone.dirC *= -1;
	return drone;
}

