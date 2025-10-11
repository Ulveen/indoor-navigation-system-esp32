#include <iostream>
#include <queue>
#include <windows.h>
#include <vector>

#define W 15
#define H 15

#define WALL '#'
#define START 'S'
#define END 'E'
#define VISITED '.'
#define IN_QUEUE '*'

#define INVALID -1

const int MOVE_X[] = {0, 1, 0, -1};
const int MOVE_Y[] = {-1, 0, 1, 0};
const char CURR[] = "^>v<";

int direction = 0;

int startX = 13;
int startY = 13;

int endX = 1;
int endY = 1;

using namespace std;

struct Tile
{
    int y;
    int x;
    int distance;

    Tile *prev;

    Tile(int y, int x, int distance, Tile *prev1) : y(y), x(x), distance(distance), prev(prev1) {}
};

Tile *currTile;

struct TilePriority
{
    bool operator()(const Tile *a, const Tile *b) const
    {
        return a->distance > b->distance;
    }
};

char map[H][W + 1] = {
    "   ##########  ",
    " E #        #  ",
    "   ######## #  ",
    "          # #  ",
    "          # #  ",
    "          # #  ",
    "          # #  ",
    "          # #  ",
    "          # #  ",
    "          # #  ",
    "          # #  ",
    "          # ###",
    "          #    ",
    "          #  S ",
    "               ",
};

bool visited[H][W] = {false};

int distances[H][W];

int calcDistance(int y1, int x1, int y2, int x2)
{
    return abs(y1 - y2) + abs(x1 - x2);
}

int getDirection(int fromY, int fromX, int targetY, int targetX)
{
    if (targetY < fromY)
    {
        return 0;
    }
    if (targetX > fromX)
    {
        return 1;
    }
    if (targetY > fromY)
    {
        return 2;
    }
    return 3;
}

void init()
{
    map[startX][startY] = START;
    map[endX][endY] = END;

    for (int i = 0; i < H; i++)
    {
        for (int j = 0; j < W; j++)
        {
            distances[i][j] = calcDistance(endY, endX, i, j);
        }
    }
}

void showDistances()
{
    for (int i = 0; i < H; i++)
    {
        for (int j = 0; j < W; j++)
        {
            printf("%2d ", distances[i][j]);
        }
        puts("");
    }
}

void showMap()
{
    for (int i = 0; i < H; i++)
    {
        for (int j = 0; j < W; j++)
        {
            if (i == currTile->y && j == currTile->x)
            {
                printf("%c", CURR[direction]);
            }
            else
            {
                printf("%c", map[i][j]);
            }
        }
        puts("");
    }
}

bool isOutOfBounds(int y, int x)
{
    return y < 0 || x < 0 || y > H - 1 || x > W - 1;
}

void handleTurn(Tile *target, Tile *from)
{
    int newDirection = getDirection(currTile->y, currTile->x, from->y, from->x);
    if (newDirection != direction)
    {
        printf("Turning...\n");
        direction = newDirection;
        Sleep(500);
    }
}

bool backtrack(Tile *target)
{
    printf("\n\n");
    showDistances();
    printf("Backtracking...\n");
    printf("Target y: %d x: %d\n", target->y, target->x);
    printf("Curr y: %d x: %d\n", currTile->y, currTile->x);
    Sleep(500);

    while (currTile)
    {
        int distance = calcDistance(target->y, target->x, currTile->y, currTile->x);
        if (distance <= 1)
        {
            return true;
        }

        Tile *prev = currTile->prev;
        handleTurn(currTile, prev);
        currTile = prev;

        system("cls");
        showMap();
        Sleep(1000);
    }
    return false;
}

int getDistance(int newY, int newX)
{
    if (isOutOfBounds(newY, newX) || visited[newY][newX] || map[newY][newX] == WALL)
    {
        return INVALID;
    }

    return distances[newY][newX];
}

void solve()
{
    priority_queue<Tile *, vector<Tile *>, TilePriority> pq;
    currTile = new Tile(startY, startX, distances[startY][startX], nullptr);
    pq.push(currTile);
    visited[startY][startY] = true;

    bool found = false;

    while (!pq.empty() && !found)
    {
        Tile *top = pq.top();
        pq.pop();

        if (calcDistance(currTile->y, currTile->x, top->y, top->x) > 1)
        {
            bool success = backtrack(top);
            if (success)
            {
                printf("Backtrack success\n");
                Sleep(1000);
            }
            else
            {
                printf("Backtrack failed\n");
                Sleep(1000);
                break;
            }
        }

        handleTurn(currTile, top);

        map[currTile->y][currTile->x] = VISITED;

        top->prev = currTile;
        currTile = top;

        if (currTile->y == endY && currTile->x == endX)
        {
            found = true;
            break;
        }

        for (int i = 0; i < 4; i++)
        {
            int newX = top->x + MOVE_X[i];
            int newY = top->y + MOVE_Y[i];

            int distance = getDistance(newY, newX);
            distances[newY][newX] = distance;

            if (distance == INVALID)
            {
                continue;
            }

            if (map[newY][newX] != END)
            {
                map[newY][newX] = IN_QUEUE;
            }

            visited[newY][newX] = true;

            pq.push(new Tile(newY, newX, distance, nullptr));
        }

        system("cls");
        showMap();
        Sleep(500);
    }
}

int main()
{
    init();
    solve();
}
