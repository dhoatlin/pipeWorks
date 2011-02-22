/*

	Pipeworks is a program that bounces a drone around inside a box.

	The trick is there are four quadrants to the box, each controlled
	by its own child process.
	
	overseer.c
	
	overseer starts everything.
	It forks all the children needed for the program
	It creates all the pipes the children will use
	It spawns a drone on the screen
	
	Author: Dave Hoatlin
	Date: 2/18/11

*/

#include "overseer.h"
#include "drone.h"
#include "draw.h"
#include "quadrant.h"
#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

const int NUM_PIPE = 8;
const int NUM_CHILD = 4;

int screenHeight;
int screenWidth;
gvpipe_t *copper;

pid_t pids[4];

/*
	Initialize the program
	
	Starts ncurses and draws the initial screen
	Creates all the pipes
	Forks all the children
*/
int main()
{
	initCurses();
	drawScreen();
	setupSigs();

	screenHeight = getHeight();
	screenWidth = getWidth();
	int i;
	
	copper = (gvpipe_t *) calloc (NUM_PIPE, sizeof(gvpipe_t));
	
	seedDroneRand();
	Drone drone = createDrone(screenWidth, screenHeight);

	int giveDrone = findQuad(drone.locX, drone.locY);
	
	//creating all pipes
	for (i = 0; i < NUM_PIPE; i++)
        pipe (copper[i]);
        
    //create children
    for (i = 0; i < NUM_CHILD; i++) {
    	pids[i] = fork ();
        if (pids[i] == 0) {
        	if(i == giveDrone)
            	begin(i, copper, NUM_CHILD, NUM_PIPE, drone, 1, screenHeight, screenWidth);
            else
            	begin(i, copper, NUM_CHILD, NUM_PIPE, drone, 0, screenHeight, screenWidth);
            exit (0);
        }
    }
    
	waitForChildren();
	
	free(copper);
	return;
}

/*
	Wait for all the child processes to complete before closing
*/
void waitForChildren()
{
	int i;
	for(i = 0; i < NUM_CHILD; i++)
	{
		wait();
	}
}

/*
	Find which quadrant the drone belongs in
*/
int findQuad(int x, int y)
{
	int giveDrone;
	if(x < screenWidth / 2 && y < screenHeight / 2)
	{
		giveDrone = 0;
	}
	else if(x > screenWidth / 2 && y < screenHeight / 2)
	{
		giveDrone = 1;
	}
	else if(x < screenWidth / 2 && y > screenHeight / 2)
	{
		giveDrone = 2;
	}
	else
	{
		giveDrone = 3;
	}
	return giveDrone;
}

/*
	setup the sig handler
*/
void setupSigs()
{
	struct sigaction act;
	act.sa_handler = sig_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGUSR1, &act, NULL);
}

/*
	Define behavior for signals
*/
void sig_handler(int num)
{
	switch(num)
	{
		case SIGUSR1:
			killTheKids();
			break;
	}
}

/*
	end all the child processes
*/
void killTheKids()
{
	int i;
	for(i = 0; i < 4; i++)
	{
		kill(pids[i], SIGINT);
	}
}

