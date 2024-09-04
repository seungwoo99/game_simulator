/* Compile method
    g++ main.cpp simulator.cpp -o simulator -Wall
*/

//simulator.cpp
//CS470
//Simon Choi
/* Honor code: "I pledge that I have neither given nor received help from anyone 
other than the instructor/TA for all program components included here!" */

//header files
#include "simulator.h"
#include <iostream>
#include <sstream>
#include <random>
#include <pthread.h>
#include <fstream>
#include <unistd.h>

using namespace std;

//global variables
pthread_mutex_t myMutex;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int mapRow;
int mapCol;
int A_Players; //players in Team A
int B_Players; //players in Team B
bool stopThreads = false;
bool gameover = false;
int result = -1;
int ATerritory;
int BTerritory;

Simulator::Simulator(int argc, char* argv[]){ //constructor
    if(argc < 5){
        cout << "Need more arguments.\n";
        cout << "FORMAT: ./simulator [A Players] [B Players] [Rows] [Columns]\n";
        gameStatus = 0;
    }else if(argc == 5){
        for(int i=1; i<argc; i++){
            istringstream ss(argv[i]);
            int num;
            if(!(ss >> num)){
                cerr << "Invalid number for input: " << argv[i] << '\n';
            }
            userInput.push_back(num);
        }
        A_Players = userInput.at(0);
        B_Players = userInput.at(1);
        mapRow = userInput.at(2);
        mapCol = userInput.at(3);
        int totalPlayers = A_Players + B_Players;
        int totalLocation = mapRow * mapCol;
        if(totalPlayers > totalLocation){
            cerr << "There are too many players than the map size.\n";
            gameStatus = 0;
        }else if(totalPlayers == totalLocation){
            cout << "Result: DRAW\n";
            gameStatus = 0;
        }else if(A_Players <= 0 || B_Players <=0 || mapRow <= 0 || mapCol <= 0){
            cout << "Arguments can not be less than or equal to 0.\n";
            gameStatus = 0;
        }else{
            //create a map
            map = new int*[mapRow];
            for(int i=0; i<mapRow; i++){
                map[i] = new int[mapCol];
            }
            for(int i=0; i<mapRow; i++){
                for(int j=0; j<mapCol; j++){
                    map[i][j] = 0;
                }
            }
            ATerritory = A_Players;
            BTerritory = B_Players;
        }
    }else{
        cout << "Too many arguments.\n";
        cout << "FORMAT: ./simulator [A Players] [B Players] [Rows] [Columns]\n";
        gameStatus = 0;
    }
}
Simulator::~Simulator(){ //destructor
}
void Simulator::placePlayer(){
    //place team A players
    for(int i=0; i<A_Players; i++){
        int x = genRandNum(0, mapRow-1);
        int y = genRandNum(0, mapCol-1);
        if(map[x][y] == 0){
            map[x][y] = 1;
        }else {
            i--;
        }
    }
    //place team B players
    for(int i=0; i<B_Players; i++){
        int x = genRandNum(0, mapRow-1);
        int y = genRandNum(0, mapCol-1);
        if(map[x][y] == 0){
            map[x][y] = 2;
        }else {
            i--;
        }
    }
    //store map array in binary file
    ofstream file("map.bin", ios::out | ios::binary);        
    //write the array to the binary file
    size_t size = mapRow * mapCol * sizeof(int);
    file.write((char*)map, size);
    file.close();

    cout << "<Orginal Map>\n";
    for(int i=0; i<mapRow; i++){
        for(int j=0; j<mapCol; j++){
            if(map[i][j] == 0){
                cout << "0" << " | ";
            }else if(map[i][j] == 1){
                cout << "A" << " | ";
            }else if(map[i][j] == 2){
                cout << "B" << " | ";
            }
        }
        cout << endl;
    }
    cout << endl;
}
int Simulator::genRandNum(int min, int max){
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> randNum(min, max);
    int num = randNum(gen);

    return num; //return generated random number
}
/*
map element values
0: unoccupied territory
1: A players location
2: B players location
3: A territory
4: B territory
*/
void* Simulator::Missile(void* player){
    while(!stopThreads){
        int playerID = (int)(long)(player);
        //generate random numbers for coordinate
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> randRow(0, mapRow-1);
        uniform_int_distribution<> randCol(0, mapCol-1);
        int randX = randRow(gen);
        int randY = randCol(gen);
    
        pthread_mutex_lock(&myMutex);
        cout << "----------------------------------------------------\n";
        //read binary file that map array is stored
        ifstream ex_file("map.bin", ios::binary);
        int **readMap = new int*[mapRow];
        for(int i=0; i<mapRow; i++){
            readMap[i] = new int[mapCol];
        }
        if(ex_file.is_open()){
            size_t size = mapRow * mapCol * sizeof(int);
            ex_file.read((char*) readMap, size);
            ex_file.close();
        }else{
            printf("Fail to open binary file.\n");
            return (void*)0;
        }

        if(playerID <= A_Players){ //if thread function is called from Team A
            printf("Player%d from team A sends a missile to location (%d,%d).\n", playerID, randX, randY);
        }else if(playerID > A_Players){ //if thread function is called from Team B
            printf("Player%d from team B sends a missile to location (%d,%d).\n", playerID-A_Players, randX, randY);
        }

        switch (readMap[randX][randY]){
            case 0: //unoccupies location
                if(playerID <= A_Players){
                    readMap[randX][randY] = 3; //Team A occupies the location
                    printf("Player%d from team A occupies new location (%d,%d).\n", playerID, randX, randY);
                    checkNeighbors(readMap, randX, randY, playerID);
                    ++ATerritory;
                }else if(playerID > A_Players){
                    readMap[randX][randY] = 4; //Team B occupies the location
                    printf("Player%d from team B occupies new location (%d,%d).\n", playerID-A_Players, randX, randY);
                    checkNeighbors(readMap, randX, randY, playerID);
                    ++BTerritory;
                }
                break;
            case 1: //location where a player from Team A is standing
                printf("This location (%d,%d) is where A team player is located.\n", randX, randY);
                break;
            case 2: //location where a player from Team B is standing
                printf("This location (%d,%d) is where B team player is located.\n", randX, randY);
                break;
            case 3: //Team A's territory
                if(playerID <= A_Players){ //Team A
                    readMap[randX][randY] = 0; //release the territory
                    printf("Player%d from team A releases his territory (%d,%d).\n", playerID, randX, randY);
                    --ATerritory;
                }else if(playerID > A_Players){ //Team B
                    readMap[randX][randY] = 4;  //Team B occupies the location
                    printf("Player%d from team B occupies territory (%d,%d) of team A.\n", playerID-A_Players, randX, randY);
                    checkNeighbors(readMap, randX, randY, playerID);
                    ++BTerritory;
                    --ATerritory;
                }
                break;
            case 4: //Team B's territory
                if(playerID <= A_Players){ //Team A
                    readMap[randX][randY] = 3; //Team B occupies the location
                    printf("Player%d from team A occupies territory (%d,%d) of team B.\n", playerID, randX, randY);
                    checkNeighbors(readMap, randX, randY, playerID);
                    ++ATerritory;
                    --BTerritory;
                }else if(playerID > A_Players){ //Team B
                    readMap[randX][randY] = 0; //release the territory
                    printf("Player%d from team B releases his territory (%d,%d).\n", playerID-A_Players, randX, randY);
                    --BTerritory;
                }
                break;
        }

        //store readMap array in binary file
        ofstream file("map.bin", ios::out | ios::binary);        
        //write the array to the binary file
        size_t size = mapRow * mapCol * sizeof(int);
        file.write((char*)readMap, size);
        file.close();

        //sleep for random number
        uniform_int_distribution<> randDelay(1, 3);
        int delay = randDelay(gen);
        printf("Need preparation for next missile for %d seconds.\n", delay);
        //current status of the map
        printf("A territory: %d, B territory: %d\n", ATerritory, BTerritory);

        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&myMutex);
        sleep(delay);
    }
    return (void*)0;
}
bool Simulator::checkStatus(){
    if((ATerritory == BTerritory) && (ATerritory+BTerritory == mapRow*mapCol)){
        //game is draw
        gameover = true;
        result = 0;
        return gameover;
    }else if((BTerritory == B_Players) && (ATerritory == mapRow*mapCol-BTerritory)){
        //Team A wins
        gameover = true;
        result = 1;
        return gameover;
    }else if((ATerritory == A_Players) && (BTerritory == mapRow*mapCol-ATerritory)){
        //Team B wins
        gameover = true;
        result = 2;
        return gameover;
    }
    return false;
}
void* Simulator::checkMap(void* arg){
    pthread_mutex_lock(&myMutex);
    while(!checkStatus()){
        pthread_cond_wait(&cond, &myMutex);
    }
    stopThreads = true;
    if(result == 0){
        cout << "RESULT: Draw.\n";  
    }else if(result == 1){
        cout << "RESULT: Team A wins.\n";
    }else if(result == 2){
        cout << "RESULT: Team B wins.\n";
    }
    pthread_mutex_unlock(&myMutex);

    return (void*)0;
}
void Simulator::startSimul(){
    pthread_t Thread_A[A_Players];
    pthread_t Thread_B[B_Players];
    pthread_t Supervisor;
    //create supervisor thread
    pthread_create(&Supervisor, NULL, checkMap, NULL);
    //create Team A players' threads
    for(int threadA=0; threadA<A_Players; threadA++){
        pthread_create(&Thread_A[threadA], NULL, Missile, (void*)(long)(threadA+1));
        usleep(200);
    }
    //create Team B players' threads
    for(int threadB=A_Players; threadB<A_Players+B_Players; threadB++){
        pthread_create(&Thread_B[threadB], NULL, Missile, (void*)(long)(threadB+1));
        usleep(200);
    }

    //join Team A threads
    for(int i=0; i<A_Players; i++){
        pthread_join(Thread_A[i], NULL);
    }
    //join Team B threads
    for(int i=0; i<B_Players; i++){
        pthread_join(Thread_B[i], NULL);
    }
    //join supervisor thread
    pthread_join(Supervisor, NULL);
}
void Simulator::checkNeighbors(int** map, int x, int y, int playerID){
    int countA = 0;
    int countB = 0;
    //count territories of Team A and Team B
    for(int i=x-1; i<=x+1; i++){
        for(int j=y-1; j<=y+1; j++){
            if((0<=i && i<mapRow) && (0<=j && j<mapCol)){
                if(map[i][j] == 1 || map[i][j] == 3){
                    ++countA;
                }else if(map[i][j] == 2 || map[i][j] == 4){
                    ++countB;
                }
            }
        }
    }
    //if Team A occupies the territory and has more territories around it
    if(countA > countB && playerID <= A_Players){
        for(int i=x-1; i<=x+1; i++){
            for(int j=y-1; j<=y+1; j++){
                if((0<=i && i<mapRow) && (0<=j && j<mapCol)){
                    if(map[i][j] == 4){
                        map[i][j] = 3;
                        ++ATerritory;
                        --BTerritory;
                        printf("Location (%d, %d) is changed to Team A's territory.\n", i, j);
                    }
                }
            }
        }
    //if Team B occupies the territory and has more territories around it
    }else if(countA < countB && playerID > A_Players){
        for(int i=x-1; i<=x+1; i++){
            for(int j=y-1; j<=y+1; j++){
                if((0<=i && i<mapRow) && (0<=j && j<mapCol)){
                    if(map[i][j] == 3){
                        map[i][j] = 4;
                        --ATerritory;
                        ++BTerritory;
                        printf("Location (%d, %d) is changed to Team B's territory.\n", i, j);
                    }
                }
            }
        }
    }
}
void Simulator::printMap(){
    //read binary file that map array is stored
    ifstream ex_file("map.bin", ios::binary);
    int **result = new int*[mapRow];
    for(int i=0; i<mapRow; i++){
        result[i] = new int[mapCol];
    }
    if(ex_file.is_open()){
        size_t size = mapRow * mapCol * sizeof(int);
        ex_file.read((char*) result, size);
        ex_file.close();
    }
    for(int i=0; i<mapRow; i++){
        for(int j=0; j<mapCol; j++){
            if(result[i][j] == 1){
                cout << "A" << " | ";
            }else if(result[i][j] == 2){
                cout << "B" << " | ";
            }else if(result[i][j] == 3){
                cout << "a" << " | ";
            }else if(result[i][j] == 4){
                cout << "b" << " | ";
            }
        }
        cout << endl;
    }
    cout << "---------------------GAME OVER----------------------\n";
    
}