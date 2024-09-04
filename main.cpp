/* Compile method
    g++ main.cpp simulator.cpp -o simulator -Wall
*/

//main.cpp
//CS470
//Simon Choi
/* Honor code: "I pledge that I have neither given nor received help from anyone 
other than the instructor/TA for all program components included here!" */

//header files
#include "simulator.h"

//main function
int main(int argc, char* argv[]){
    //read command line argument
    Simulator simulator = Simulator(argc, argv);
    if(simulator.gameStatus == 0){
        return 0;
    }

    //Position players in A and B randomly on the map
    simulator.placePlayer();

    //start simulating
    simulator.startSimul();

    //print the map after game over
    simulator.printMap();

    return 0;


}
