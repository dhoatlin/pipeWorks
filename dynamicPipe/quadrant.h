/* 
	header for quadrant.c 
	
*/
typedef struct
{
	int northIn;
	int northOut;
	int eastIn;
	int eastOut;
	int southIn;
	int southOut;
	int westIn;
	int westOut;
	
	int northBound;
	int eastBound;
	int southBound;
	int westBound;
} Cell;

void begin(int id, gvpipe_t pipes[], int totChildren, int totPipes, Cell cell, int gotDrone, int height, int width);
void runChild();
void child_handler(int num);
void readDrone();
void updateDrone();
void checkAndSend();
void waitingForDrone();
