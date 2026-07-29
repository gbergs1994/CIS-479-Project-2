// CIS 479 Project 2 
// Gunnar Bergstrom 

#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <limits>

using namespace std;

const int ROWS = 5;
const int COLS = 6;
const double P_HIT_OBS = 0.95; // Correct obstacle guessing
const double P_FALSE_OBS = 0.15; // Incorrect obstacle reporting 
// Forward, left, and right movement probability 
const double P_FORWARD = 0.70; 
const double P_LEFT = 0.20;
const double P_RIGHT = 0.10;
enum Action {NORTH, EAST, SOUTH, WEST};

// Maze, 0 =  open space, 1 = obstacle
int maze[ROWS][COLS]={
{0,0,0,0,0,0},
{0,1,1,1,1,0},
{0,1,0,0,1,0},
{0,1,0,0,0,0},
{0,0,0,0,0,0}
};

double belief[ROWS][COLS];
double tempBelief[ROWS][COLS];

// Checks for valid maze setup 
bool valid(int r,int c) {
    if(r<0||r>=ROWS||c<0||c>=COLS)
        return false;
    return maze[r][c]==0;
}

// Checks for obstactle 
bool obstacle(int r,int c) {
    return !valid(r,c);
}

// Equal probability for open spaces
void initialize() {
    int count=0;

    for(int r=0;r<ROWS;r++)
        for(int c=0;c<COLS;c++)
            if(valid(r,c))
                count++;

    double p=100.0/count;

    for(int r=0;r<ROWS;r++)
        for(int c=0;c<COLS;c++)
            belief[r][c]=valid(r,c)?p:0;
}

// Prints the belief of each cell in the maze
void printBelief(string title) {
    cout<<endl<<title<<endl;

    cout<<fixed<<setprecision(2);

    for(int r=0;r<ROWS;r++) {
        for(int c=0;c<COLS;c++) {
            if(!valid(r,c)) // when obstacle sensed
                cout<<" ## ";
            else
                cout<<setw(6)<<belief[r][c];
        }
        cout<<endl;
    }
}

// Converts direction to row and column location
void directionDelta(int dir,int &dr,int &dc) {
    dr=dc=0;

    switch(dir) {
        case NORTH: dr=-1; break;
        case SOUTH: dr=1; break;
        case EAST: dc=1; break;
        case WEST: dc=-1; break;
    }
}

// Returns the direction to the left of movement
int leftDir(int d) {
    if(d==NORTH) return WEST;
    if(d==WEST) return SOUTH;
    if(d==SOUTH) return EAST;
    return NORTH;
}

// Returns the direction to the right of movement
int rightDir(int d) {
    if(d==NORTH) return EAST;
    if(d==EAST) return SOUTH;
    if(d==SOUTH) return WEST;
    return NORTH;
}

// Moves the probability of a cell to its neighbor
void moveContribution(int r,int c,int dir,double prob) {
    int dr,dc;
    directionDelta(dir,dr,dc);

    int nr=r+dr;
    int nc=c+dc;

    if(valid(nr,nc))
        tempBelief[nr][nc]+=belief[r][c]*prob;
    else
        tempBelief[r][c]+=belief[r][c]*prob;
}

// Updates the probability of each cell 
void prediction(Action action) {
    for(int r=0;r<ROWS;r++)
        for(int c=0;c<COLS;c++)
            tempBelief[r][c]=0;

    for(int r=0;r<ROWS;r++) {
        for(int c=0;c<COLS;c++) {
            if(!valid(r,c))
                continue;

            moveContribution(r,c,action,P_FORWARD);
            moveContribution(r,c,leftDir(action),P_LEFT);
            moveContribution(r,c,rightDir(action),P_RIGHT);
        }
    }

    for(int r=0;r<ROWS;r++)
        for(int c=0;c<COLS;c++)
            belief[r][c]=tempBelief[r][c];
}

// Returns probability of a given cell 
double observationProbability(int r,int c,const vector<int>& obs) {
    bool realObs[4];

    // West North East South
    realObs[0]=obstacle(r,c-1);
    realObs[1]=obstacle(r-1,c);
    realObs[2]=obstacle(r,c+1);
    realObs[3]=obstacle(r+1,c);

    double p=1.0;

    for(int i=0;i<4;i++) {
        if(realObs[i]) {
            if(obs[i]==1)
                p*=P_HIT_OBS;
            else
                p*=1-P_HIT_OBS;
        }
        else {
            if(obs[i]==1)
                p*=P_FALSE_OBS;
            else
                p*=1-P_FALSE_OBS;
        }
    }

    return p;
}

// Updates the probability of each cell 
void filtering(const vector<int>& obs) {
    double total=0;

    for(int r=0;r<ROWS;r++) {
        for(int c=0;c<COLS;c++) {
            if(!valid(r,c))
                continue;

            belief[r][c]*=observationProbability(r,c,obs);
            total+=belief[r][c];
        }
    }

    for(int r=0;r<ROWS;r++)
        for(int c=0;c<COLS;c++)
            if(valid(r,c)) //If valid, normalize 
                belief[r][c]=belief[r][c]/total*100.0;
}

// Waits for Enter to continue
void waitForEnter() {
    cout << "\nPress Enter to continue...";
    cin.get();
}

// Main 
int main() {
    initialize();
    printBelief("Initial Location Probabilities");
    waitForEnter();

    filtering({0,0,0,1});
    printBelief("Filtering after Evidence [W, N, E, S] [0,0,0,1]");
    waitForEnter();

    prediction(NORTH);
    printBelief("Prediction after Action N");
    waitForEnter();

    filtering({1,0,0,0});
    printBelief("Filtering after Evidence [W, N, E, S] [1,0,0,0]");
    waitForEnter();

    prediction(NORTH);
    printBelief("Prediction after Action N");
    waitForEnter();

    filtering({1,1,0,0});
    printBelief("Filtering after Evidence [W, N, E, S] [1,1,0,0]");
    waitForEnter();

    prediction(EAST);
    printBelief("Prediction after Action E");
    waitForEnter();

    filtering({0,1,1,0});
    printBelief("Filtering after Evidence [W, N, E, S] [0,1,1,0]");
    waitForEnter();

    return 0;
}