/*
	header for drone.c
*/

typedef struct
{
	int dirR;
	int dirC;
	int locX;
	int locY;
} Drone;

Drone createDrone(int x, int y);
void seedDroneRand();
Drone getRandDir(Drone drone);
int randomize(int a, int b);
int getRandLoc(int bound);
Drone moveD(int x, int y, Drone drone);
Drone invertDirR(Drone drone);
Drone invertDirC(Drone drone);

Drone loadDrone();
void saveDrone(Drone saveDrone);
