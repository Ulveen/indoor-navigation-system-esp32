#include <bits/stdc++.h>
using namespace std;

#define sizeI 7
#define sizeJ 7

struct entity {
    int i;
    int j;
};
entity e{0,0};

char currState = 'V';

char maze[sizeI][sizeJ] = {
    {'.', '#', '#', '.', '.', '.', '#'},
    {'.', '#', '#', '.', '#', '.', '#'},
    {'.', '#', '#', '.', '#', '.', '.'},
    {'.', '.', '.', '.', '#', '.', '.'},
    {'.', '.', '#', '.', '#', '#', '.'},
    {'.', '.', '#', '.', '.', '#', '.'},
    {'.', '.', '#', '.', '.', '#', '.'}
};

int weights[sizeI][sizeJ] = {0};

void clearScreen() {
    for (int i=0;i<42;i++) {
        puts("");
    }
}

void printDebug(const bool map) {
    for (int i = 0; i < sizeI; i++) {
        for (int j = 0; j < sizeJ; j++) {
            if (map) {
                if (i == e.i && j == e.j) {
                    cout << currState << " ";

                }else {
                    cout << maze[i][j] << " ";
                }
            }
            else {
                if (weights[i][j] == INT_MAX) {
                    cout << setw(3) << "##" << " ";
                }else {
                    cout << setw(3) << weights[i][j] << " ";
                }
            }
        }
        cout << endl;
    }
}

int manhattanDistance(const int i, const int j, const int targetI, const int targetJ) {
    return abs((targetI - i) + (targetJ - j));
}

void initWeights() {
    for (int i=0;i<sizeI;i++) {
        for (int j=0;j<sizeJ;j++) {
            weights[i][j] = manhattanDistance(i,j, sizeI-1, sizeJ-1);
        }
    }
}

vector<vector<int>> dir = {{0,1,2},{1,0,0},{0,-1,3},{-1,0,1}};
// kanan, bawah, kiri, atas
vector<char> states = {'V','^','>','<'};

bool validateCoordinate(const int &i, const int &j) {

    if (i < 0 || j < 0 || i >= sizeI || j >= sizeJ) return false;
    if (maze[i][j] == '#') {
        return false;
    }

    return true;
}

void floodFill(int i, int j, vector<vector<bool>> &vis) {
    queue<pair<int,int>> q;
    q.push({i, j});
    weights[i][j] = 0;

    while (!q.empty()) {
        auto [i, j] = q.front();
        q.pop();
        vis[i][j] = true;

        for (auto &d : dir) {
            int ni = i + d[0];
            int nj = j + d[1];

            // check boundary and wall
            if (ni < 0 || nj < 0 || ni >= sizeI || nj >= sizeJ) continue;
            if (!vis[ni][nj]) {
                if (weights[ni][nj] != INT_MAX) {
                    weights[ni][nj] = weights[i][j] + 1;
                    q.push({ni, nj});
                }else {
                    weights[ni][nj] = INT_MAX;
                }
            }
        }
    }
}

void moveEntity() {
    bool moved = false;

    while (e.i != sizeI-1 || e.j != sizeJ-1) {
        moved = false;
        clearScreen();
        printDebug(true);
        puts("");
        printDebug(false);
        getchar();

        for (auto &d : dir) {
            int newI = e.i + d[0], newJ = e.j + d[1];
            if (!moved && validateCoordinate(newI,newJ) && weights[e.i][e.j] > weights[newI][newJ]) {
                e.i += d[0];
                e.j += d[1];
                moved=true;
                currState = states[d[2]];
            }
            if (newI >= 0 && newJ>=0 && newI < sizeI && newJ < sizeJ && maze[newI][newJ] == '#') {
                weights[newI][newJ] = INT_MAX;
            }
        }

        if (!moved) {
            vector<vector<bool>> vis(sizeI,vector<bool>(sizeJ,false));
            floodFill(sizeI-1, sizeJ-1, vis);
        }


    }

    clearScreen();
    printDebug(true);
}

int main() {

    vector<vector<bool>> vis(sizeI,vector<bool>(sizeJ,false));
    floodFill(sizeI-1, sizeJ-1, vis);
    moveEntity();

    return 0;
}
