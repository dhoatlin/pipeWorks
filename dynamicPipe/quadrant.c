/*
	Pipeworks is a program that bounces a drone around inside a box.

	The trick is there are four quadrants to the box, each controlled
	by its own child process.
	
	quadrant.c
	
	Runs as a child process.
	This handles everything inside a specific quadrant.
	Reads and writes from pipes when the drone comes in and out of a quadrant
*/

#include "overseer.h"
#include "drone.h"
#include "quadrant.h"
#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>

const int READ = 0;
const int WRITE = 1;

int previous, next, myID;
int running = 1;
int prevOut, prevIn, nextOut, nextIn;

int hasDrone;
int screenHeight, screenWidth;
gvpipe_t *myPipes;
Drone myDrone, sendDrone;
Cell myCell;

/*
	Start the child process
	
	Sets up where each pipe is going
*/
void begin(int id, gvpipe_t pipes[], int totChildren, int totPipes, Cell cell, int gotDrone, int height, int width)
{
	myID = id;
	myPipes = pipes;
	myCell = cell;
	hasDrone = gotDrone;
	screenHeight = height;
	screenWidth = width;

	if(hasDrone)
	{
		myDrone = createDrone(myCell.westBound, myCell.eastBound, myCell.southBound, myCell.northBound);
	}
	//updateText(myDrone.locX, myDrone.locY, getpid());
	drawDrone(myDrone.locX, myDrone.locY);
	runChild();
}

/*
	If this cell does not currently contain a drone wait until 
	a drone has moved to this cell
*/
void waitingForDrone()
{
	int num, curPipe;
	int maxDesc = -1;
	Drone readDrone;
	fd_set waitFor;
	
	FD_ZERO(&waitFor);
	FD_SET(myPipes[myCell.northIn][READ], &waitFor);
	FD_SET(myPipes[myCell.eastIn][READ], &waitFor);
	FD_SET(myPipes[myCell.southIn][READ], &waitFor);
	FD_SET(myPipes[myCell.westIn][READ], &waitFor);
	
	maxDesc = myPipes[myCell.northIn][READ];
	if(myPipes[myCell.eastIn][READ] > maxDesc)
		maxDesc = myPipes[myCell.eastIn][READ];
	if(myPipes[myCell.southIn][READ] > maxDesc)
		maxDesc = myPipes[myCell.southIn][READ];
	if(myPipes[myCell.westIn][READ] > maxDesc)
		maxDesc = myPipes[myCell.westIn][READ];
	
	//waiting for a pipe to have something to read
	num = select(maxDesc, &waitFor, NULL, NULL, NULL);
			
	if(num > 0)
	{
		for(curPipe = 0; curPipe <= maxDesc; curPipe++)
		{
			//find pipe with data and read it
			if(FD_ISSET(curPipe, &waitFor))
			{
				//childChange(getpid());
				read(curPipe, &readDrone, sizeof(Drone));
				hasDrone = 1;
			
				myDrone = readDrone;
			}
		}
	}
}
/*
	Update the drones location and then draw its new position
	
	Also checks if the drone's direction needs to be inverted
*/
void updateDrone()
{
	myDrone = moveD(myDrone.dirC, myDrone.dirR, myDrone);
	drawDrone(myDrone.locX, myDrone.locY);
	updateText(myDrone.locX, myDrone.locY, getpid());
	
	//check if invert needed
	if (myDrone.locY <= 2 || myDrone.locY >= screenHeight - 1)
	{
		myDrone = invertDirR(myDrone);
	}
    if (myDrone.locX <= 1 || myDrone.locX >= screenWidth - 1)
    {
    	myDrone = invertDirC(myDrone);
    }
}

/*
	Check if a drone has left the cells boundries
	
	If it has send it to the next cell
*/
void checkAndSend()
{
	Drone sendDrone = myDrone;
	if(myDrone.locY < myCell.northBound && myDrone.locY > 0)
	{
		write(myPipes[myCell.northOut][WRITE], &sendDrone, sizeof(Drone));
		hasDrone = 0;
	}
	else if(myDrone.locY > myCell.southBound && myDrone.locY < screenHeight - 1)
	{
		write(myPipes[myCell.southOut][WRITE], &sendDrone, sizeof(Drone));
		hasDrone = 0;
	}
	else if(myDrone.locX < myCell.westBound && myDrone.locX > 0)
	{
		write(myPipes[myCell.westOut][WRITE], &sendDrone, sizeof(Drone));
		hasDrone = 0;
	}
	else if(myDrone.locX > myCell.eastBound && myDrone.locX < screenWidth - 1)
	{
		write(myPipes[myCell.eastOut][WRITE], &sendDrone, sizeof(Drone));
		hasDrone = 0;
	}
}
/*
	Handles the movement of the drone inside a quadrant.
	
	If the drone is not in this quadrant, block until some input comes from the
	pipe.
	
	If the drone leaves this quadrant, write the drone struct to the next
	quadrant
*/
void runChild()
{	
	int maxDesc;
	while(running)
	{		
		//wait for input if no drone present
		if(!hasDrone)
		{
			waitingForDrone();
		}
		
		//otherwise update the location and check if it needs to be passed
		else if(hasDrone)
		{
			updateDrone();
			checkAndSend();
		}
		sleep(1);
	}
}
