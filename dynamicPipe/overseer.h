/*
	header for overseer.c
*/

typedef int (gvpipe_t) [2];


int findQuad(int x, int y);
void waitForChildren();
void setupSigs();
void sig_handler(int num);
void killTheKids();

