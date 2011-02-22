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

int running, hasDrone;
int screenHeight, screenWidth;
gvpipe_t *myPipes;
Drone myDrone, sendDrone;

/*
	Start the child process
	
	Sets up where each pipe is going
*/
void begin(int id, gvpipe_t pipes[], int totChildren, int totPipes, Drone drone, int gotDrone, int height, int width)
{
	myID = id;
	myPipes = pipes;
	myDrone = drone;
	hasDrone = gotDrone;
	screenHeight = height;
	screenWidth = width;
	
	//setup the pipes
	//prevout = id + (id-1)
	//previn = id + (id-2)
	//nextout = 2*id
	//nextin = (2*id)+1
	
	
	if(myID == 0)
	{
		prevOut = totPipes - 1;
		prevIn = totPipes - 2;
	}
	else
	{
		prevOut = myID + (myID - 1);
		prevIn = myID + (myID - 2);
	}
	nextOut = 2 * myID;
	nextIn = (2 * myID) + 1;
	
	//close the appropriate ends
	//dont want to read from out pipes or write on in pipes
	close(myPipes[prevOut][READ]);
	close(myPipes[prevIn][WRITE]);
	close(myPipes[nextOut][READ]);
	close(myPipes[nextIn][WRITE]);
	
	runChild();
}

/*
	Wait for a new drone to enter the cell
*/
void waitingForDrone()
{
	int num, curPipe, maxDesc;
	Drone readDrone;
	fd_set waitFor;
	
	FD_ZERO(&waitFor);
	FD_SET(myPipes[prevIn][READ], &waitFor);
	FD_SET(myPipes[nextIn][READ], &waitFor);
	
	//finding value for maxDesc
	if(myPipes[prevIn][READ] > myPipes[nextIn][READ])
		maxDesc = myPipes[prevIn][READ] + 1;
	else
		maxDesc = myPipes[nextIn][READ] + 1;
		
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
	update the location of the drone and redraw its location
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
	a VERY hard coded way of handling where to send each drone
	finds which cell this process is and then checks its borders
	to see if the drone left.
	
	This is made much cleaner in the dynamic version by creating
	a struct for each cell.
*/
void checkAndSend()
{
	Drone sendDrone = myDrone;
	switch(myID)
	{
		case 0:
			if(myDrone.locY >= screenHeight / 2)
			{
				write(myPipes[prevOut][WRITE], &sendDrone, sizeof(Drone));
				hasDrone = 0;
			}
			else if(myDrone.locX >= screenWidth / 2)
			{
				write(myPipes[nextOut][WRITE], &sendDrone, sizeof(Drone));
				hasDrone = 0;
			}
			break;
		case 1:
			if(myDrone.locY >= screenHeight / 2)
			{
				write(myPipes[nextOut][WRITE], &sendDrone, sizeof(Drone));
				hasDrone = 0;
			}
			else if(myDrone.locX <= screenWidth / 2)
			{
				write(myPipes[prevOut][WRITE], &sendDrone, sizeof(Drone));
				hasDrone = 0;
			}
			break;
		case 2:
			if(myDrone.locY <= screenHeight / 2)
			{
				write(myPipes[prevOut][WRITE], &sendDrone, sizeof(Drone));
				hasDrone = 0;
			}
			else if(myDrone.locX <= screenWidth / 2)
			{
				write(myPipes[nextOut][WRITE], &sendDrone, sizeof(Drone));
				hasDrone = 0;
			}
			break;
		case 3:
			if(myDrone.locY <= screenHeight / 2)
			{
				write(myPipes[nextOut][WRITE], &sendDrone, sizeof(Drone));
				hasDrone = 0;
			}
			else if(myDrone.locX <= screenWidth / 2)
			{
				write(myPipes[prevOut][WRITE], &sendDrone, sizeof(Drone));
				hasDrone = 0;
			}
			break;
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
