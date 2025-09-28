#include <iostream>
#include <vector>
#include <queue>
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

    Tile(int y, int x, int distance) : y(y), x(x), distance(distance) {}
};

struct TilePriority
{
    bool operator()(const Tile &a, const Tile &b) const
    {
        return a.distance > b.distance;
    }
};

int world[H][W] = {0};
int distances[H][W] = {0};

int currDirection, currX, currY;

void initWorld()
{
    world[START_Y][START_X] = START;
    world[END_Y][END_X] = END;

    currDirection = FORWARD;
    currX = START_X;
    currY = START_Y;
}

void initDistance()
{
    for (int i = 0; i < H; i++)
    {
        for (int j = 0; j < W; j++)
        {
            distances[i][j] = abs(END_Y - i) + abs(END_X - j);
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
    return y < 0 || x < 0 || y > H - 2 || x > W - 2;
}

void solve()
{
    bool found = false;

    priority_queue<Tile, vector<Tile>, TilePriority> pq;
    pq.push(Tile(START_X, START_Y, distances[START_X][START_Y]));
    while (!found && !pq.empty())
    {
        Tile curr = pq.top();
        pq.pop();

        printf("%d %d\n", curr.y, curr.x);

        for (int i = 0; i < 4; i++)
        {
            int newX = curr.x + MOVE_X[i];
            int newY = curr.y + MOVE_Y[i];

            if (isOutOfBounds(newY, newX))
            {
                continue;
            }

            int type = world[newY][newX];

            if (type == END)
            {
                found = true;
                break;
            }

            if (type == VISITED)
            {
                continue;
            }

            if (type == WALL)
            {
                distances[newY][newX] = INVALID;
            }

            int newDistance = distances[newY][newX];

            if (newDistance == INVALID)
            {
                continue;
            }

            world[newY][newX] = VISITED;
            pq.push(Tile(newY, newX, newDistance));
        }
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
    fflush(stdin);

    solve();

    scanf("[^\n]");
    fflush(stdin);
}