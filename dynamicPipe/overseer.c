/*

	Pipeworks is a program that bounces a drone around inside a box.

	The trick is there are any number of quadrants to the box, each controlled
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
	Main method
	
	initializes everything needed to run the program
*/
int main(int argc, char *argv[])
{
	int rows, cols, totPipes, totChild, i, j, pipeCount, heightSegment, widthSegment;
	if(argc < 3)
	{
		printf("usage: %s #-of-rows #-of-cols\n", argv[0]);
		exit(-1);
	}
	rows = atoi(argv[1]);
	cols = atoi(argv[2]);
	if(rows < 1 || cols < 1)
	{
		printf("parameters must be an int greater than 0\n");
		exit(-1);
	}

	totPipes = calcTotPipes(rows, cols);
	totChild = rows * cols;
	Cell cells[rows][cols];

	initCurses();
	drawScreen(rows, cols);
	setupSigs();

	screenHeight = getHeight();
	screenWidth = getWidth();
	heightSegment = screenHeight / rows;
	widthSegment = screenWidth / cols;
	
	//setting up the array of cells and setting their boundries
	for(i = 0; i < rows; i++)
	{
		for(j = 0; j < cols; j++)
		{
			Cell cell;
			cell.northBound = i * heightSegment;
			cell.southBound = (i+1) * heightSegment;
			
			cell.westBound = j * widthSegment;
			cell.eastBound = (j+1) * widthSegment;
			
			//setting up pipe ids
			
			//north
			if(i == 0) //top row
			{
				//cell.northOut = NULL;
				//cell.northIn = NULL;
			}
			//north cell will always be set up
			//we can just grab its values
			else
			{
				cell.northOut = cells[i-1][j].southIn;
				cell.northIn = cells[i-1][j].southOut;
			}
			
			//east
			if(j == (cols - 1)) //right most row
			{
				//cell.eastOut = NULL;
				//cell.eastIn = NULL;
			}
			else
			{
				cell.eastOut = pipeCount;
				cell.eastIn = pipeCount + 1;
				pipeCount += 2;
			}
			
			//south
			if(i == (rows-1)) //bottom row
			{
				//cell.southOut = NULL;
				//cell.southIn = NULL;
			}
			else
			{
				cell.southOut = pipeCount;
				cell.southIn = pipeCount + 1;
				pipeCount += 2;
			}
			//west
			if(j == 0)//left most row
			{
				//cell.westOut = NULL;
				//cell.westIn = NULL;
			}
			else
			{
				cell.westOut = cells[i][j-1].eastIn;
				cell.westIn = cells[i][j-1].eastOut;
			}
			
			cells[i][j] = cell;
		}
	}
	
	copper = (gvpipe_t *) calloc (totPipes, sizeof(gvpipe_t));
	
	seedDroneRand();
	//probably want to do this in the cell that gets the drone
	//Drone drone = createDrone(screenWidth, screenHeight);

	int giveDrone = randomize(0, totChild - 1);
	
	//creating all pipes
	for (i = 0; i < totPipes; i++)
        pipe (copper[i]);
        
    //create children
    //going to want to pass a Cell struct here...
    int forkCount = 0;
    for(i = 0; i < rows; i++)
    {
    	for(j = 0; j < cols; j++)
    	{
    		pids[forkCount] = fork();
    		if(pids[forkCount] == 0)
    		{
    			if(forkCount == giveDrone) //hack to send out the drone
    				begin(i, copper, totChild, totPipes, cells[i][j], 1, screenHeight, screenWidth);
           		else
            		begin(i, copper, totChild, totPipes, cells[i][j], 0, screenHeight, screenWidth);
    		}
    	}
    }
    
	waitForChildren();
	
	free(copper);
	return;
}

/*
	Calculate the total number of pipes needed for a grid size r by c
*/
int calcTotPipes(int r, int c)
{
	int rowPipes, colPipes, totalPipes;

	rowPipes = r * (2 * (c - 1));
	colPipes = c * (2 * (r + 1));
	
	totalPipes = rowPipes + colPipes;
	
	return totalPipes;
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
	Sets up the signal handler
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
	Defines the behavior of possible signals
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
	Terminate all the child processes the parent has spawned
*/
void killTheKids()
{
	int i;
	for(i = 0; i < 4; i++)
	{
		kill(pids[i], SIGINT);
	}
}

