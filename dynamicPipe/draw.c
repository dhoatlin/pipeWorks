/*
	Pipeworks is a program that bounces a drone around inside a box.

	The trick is there are four quadrants to the box, each controlled
	by its own child process.
	
	draw.c
	
	Draw handles everything with the interface.
	It initializes the screen with a border around the edges
	It also updates the location of the drone when told to
*/

#include <curses.h>
#include <stdlib.h>
#include <unistd.h>

int width, height;
int oldx = -1;
int oldy = -1;

/*
	initialize curses so we can draw the application
*/
void initCurses()
{
	initscr();
	cbreak();
	nodelay(stdscr, 1);
	noecho();
	curs_set(0);
}

/*
	Drawing the screen without the drone.
	This includes borders and some help text
*/
void drawScreen(int rows, int cols)
{
	int i, widthInc, heightInc;
	getmaxyx (stdscr, height, width);
	widthInc = width/cols;
	heightInc = height/rows;
	
	for(i = 0; i < width; i++)
    {
    	mvprintw(0, i, "+");
    	mvprintw(i+1, 0, "+");
    	mvprintw(height - 1, i, "+");
    	mvprintw(i+1, width - 1, "+");
    }
    refresh();
}

/*
	return the height of the screen
*/
int getHeight()
{
	return height;
}

/*
	return the width of the screen
*/
int getWidth()
{
	return width;
}

/*
	draws the drone and deletes its old location. If the drone was at an edge,
	redraw that edge
*/
void drawDrone(int x, int y)
{
	if(oldx > -1 && oldy > -1)
	{
		if(oldx == 0 || oldx == width || oldy == 0 || oldy == height) 
			mvprintw(oldy, oldx, "+");
		else 
			mvprintw(oldy, oldx, " ");
	}
	
	mvprintw(y,x,"X");
	oldx = x;
	oldy = y;
	refresh();
}

/*
	print out when a process switches.
	
	used for debugging
*/
void childChange(int id)
{
	mvprintw(0, 0, "process %d in control", id);
	refresh();
}

/*
	print out the location of the drone, and the controlling process.
*/
void updateText(int x, int y, int id)
{
	mvprintw(0,0,"Drawn at %d, %d -- %d", x, y, id);
	refresh();
}
