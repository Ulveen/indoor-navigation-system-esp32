#include <stdio.h>

int map[15][15];

int main()
{

    for (int i = 0; i < 15; i++)
    {
        for (int j = 0; j < 15; j++)
        {
            printf("%d ", map[i][j]);
        }
        printf("\n");
    }
}