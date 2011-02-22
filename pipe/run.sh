#
#Build and run pipeworks
#
clear
rm overseer.o
rm drone.o
rm draw.o
rm quadrant.o
rm overseer
gcc -c overseer.c drone.c draw.c quadrant.c
gcc -o overseer -lncurses overseer.o drone.o draw.o quadrant.o
./overseer
