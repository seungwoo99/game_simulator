//simulator.h
//CS470
//Simon Choi
/* Honor code: "I pledge that I have neither given nor received help from anyone 
other than the instructor/TA for all program components included here!" */

#ifndef simulator_h
#define simulator_h

//header files
#include <vector>
using namespace std;

class Simulator{
public:
    Simulator(int argc, char* argv[]); //constructor
    ~Simulator(); //destructor
    void placePlayer();//place players on the map randomly
    int genRandNum(int min, int max); //generate random number
    static void* Missile(void* player); //player thread function
    static bool checkStatus(); //check game status who wins or loses
    static void* checkMap(void* arg); //supervisor thread function
    void startSimul(); //start simulating the game
    static void checkNeighbors(int** map, int x, int y, int playerID); //check nearby locations

    void printMap();

    int gameStatus;
    vector<int> userInput; //index 0:team A players, 1: team B players, 2: map rows, 3: map columns
    int **map;
}; //end class

#endif /* simulator_h */