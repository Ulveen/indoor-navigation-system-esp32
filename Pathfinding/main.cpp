#include <iostream>
#include <vector>
#include <queue>
#include <windows.h>

#define W 15
#define H 15

#define START_X 13
#define START_Y 13
#define END_X 1
#define END_Y 1

#define EMPTY 0
#define WALL 1
#define START 2
#define END 3
#define VISITED 4
#define IN_QUEUE 5
#define BACKTRACKED 6

#define INVALID -1
#define FORWARD 0
#define RIGHT 1
#define BACKWARD 2
#define LEFT 3

const int MOVE_X[] = {0, 1, 0, -1};
const int MOVE_Y[] = {-1, 0, 1, 0};

using namespace std;

struct Tile
{
    int y;
    int x;
    int distance;

    Tile *prev;

    Tile(int y, int x, int distance, Tile *prev1) : y(y), x(x), distance(distance), prev(prev1) {}
};

struct TilePriority
{
    bool operator()(const Tile *a, const Tile *b) const
    {
        return a->distance > b->distance;
    }
};

int world[H][W] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1}};
int distances[H][W] = {0};

int calcDistance(int y1, int x1, int y2, int x2)
{
    return abs(y1 - y2) + abs(x1 - x2);
}

int currDirection;
Tile *currTile;

void initWorld()
{
    world[START_Y][START_X] = START;
    world[END_Y][END_X] = END;
}

void initDistance()
{
    for (int i = 0; i < H; i++)
    {
        for (int j = 0; j < W; j++)
        {
            distances[i][j] = calcDistance(END_Y, END_X, i, j);
        }
    }
}

void showWorld()
{
    for (int i = 0; i < H; i++)
    {
        for (int j = 0; j < W; j++)
        {
            printf("%2d ", world[i][j]);
        }
        printf("\n");
    }
}

void showMap()
{
    for (int i = 0; i < H; i++)
    {
        for (int j = 0; j < W; j++)
        {
            printf("%2d ", distances[i][j]);
        }
        printf("\n");
    }
}

bool isOutOfBounds(int y, int x)
{
    return y < 0 || x < 0 || y > H - 1 || x > W - 1;
}

bool backtrack(Tile *target)
{
    printf("\n\n");
    showMap();
    printf("Backtracking...\n");
    printf("Target y: %d x: %d\n", target->y, target->x);
    printf("Curr y: %d x: %d\n", currTile->y, currTile->x);
    scanf("[^\n]");
    getchar();

    while (currTile)
    {
        world[currTile->y][currTile->x] = BACKTRACKED;
        system("cls");
        printf("\n");
        showWorld();
        Sleep(500);
        int distance = calcDistance(target->y, target->x, currTile->y, currTile->x);
        if (distance <= 1)
        {
            return true;
        }
        currTile = currTile->prev;
    }
    return false;
}

void solve()
{
    bool found = false;

    priority_queue<Tile *, vector<Tile *>, TilePriority> pq;
    currTile = new Tile(START_Y, START_X, distances[START_Y][START_X], nullptr);
    pq.push(currTile);

    while (!found && !pq.empty())
    {
        Tile *top = pq.top();

        top->prev = currTile;
        int distance = calcDistance(top->y, top->x, currTile->y, currTile->x);

        if (distance > 1)
        {
            bool success = backtrack(top);
            if (!success)
            {
                printf("Backtrack failed\n");
                break;
            }
            else
            {
                printf("Backtrack success\n");
            }
            scanf("[^\n]");
            getchar();
        }

        if (top->y == END_Y && top->x == END_X)
        {
            found = true;
            break;
        }

        currTile = top;

        world[top->y][top->x] = VISITED;
        pq.pop();

        for (int i = 0; i < 4; i++)
        {
            int newX = top->x + MOVE_X[i];
            int newY = top->y + MOVE_Y[i];

            if (isOutOfBounds(newY, newX))
            {
                continue;
            }

            int type = world[newY][newX];

            if (type == WALL)
            {
                distances[newY][newX] = INVALID;
            }

            int newDistance = distances[newY][newX];

            if (newDistance == INVALID || type == VISITED || type == IN_QUEUE || type == BACKTRACKED)
            {
                continue;
            }

            world[newY][newX] = IN_QUEUE;
            pq.push(new Tile(newY, newX, newDistance, nullptr));
        }

        system("cls");
        showWorld();
        Sleep(500);
    }

    if (found)
    {
        printf("\n\nFOUND!\n\n");
    }
    else
    {
        printf("\n\nNOT FOUND\n\n");
    }

    showMap();
    printf("\n\n");
    showWorld();
}

int main()
{
    system("cls");
    printf("Start: %d, End: %d\n", START, END);

    initWorld();
    printf("World: \n");
    showWorld();

    printf("\n");

    printf("Map: \n");
    initDistance();
    showMap();

    scanf("[^\n]");
    getchar();

    system("cls");
    solve();

    scanf("[^\n]");
    getchar();
}