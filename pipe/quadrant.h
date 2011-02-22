/* 
	header for quadrant.c 
	
*/


void begin(int id, gvpipe_t pipes[], int totChildren, int totPipes, Drone drone, int gotDrone, int height, int width);
void runChild();
void child_handler(int num);
void readDrone();
void updateDrone();
void checkAndSend();
void waitingForDrone();
