/*
	header file for draw.c
*/

void initCurses();
void drawScreen();
void drawDrone(int x, int y);
int getHeight();
int getWidth();
void childChange(int id);
void updateText(int x, int y, int id);
