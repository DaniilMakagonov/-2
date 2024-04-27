#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <time.h>
#include <fcntl.h>

#define M 10
#define N 10
#define V 0.01

int garden[M][N];
sem_t semaphores[M][N];
double v1 = V, v2 = V;

struct GardenerParams {
    int id;
    int startX;
    int startY;
    int speed;
};

void* gardener(void* arg) {
    struct GardenerParams* params = (struct GardenerParams*)arg;
    int x = params->startX;
    int y = params->startY;

    while (1) {
        sem_wait(&semaphores[x][y]);
        if (garden[x][y] == 0) {
            printf("Садовник %d обработал участок (%d, %d)\n", params->id, x, y);
            garden[x][y] = params->id;
            sleep(params->speed);
        }
        sem_post(&semaphores[x][y]);
        sleep(V);

        if (params->id == 1) {
            ++y;
            if (y == N) {
                y = 0;
                ++x;
                if (x == M) break;
            }
        } else {
            --x;
            if (x < 0) {
                x = M;
                --y;
                if (y < 0) break;
            }
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("input must be in format\nfirst_gardener_speed second_gardener_speed");
        return 1;
    }
    v1 *= strtol(argv[1], NULL, 10);
    v2 *= strtol(argv[2], NULL, 10);

    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            garden[i][j] = 0;
            sem_init(&semaphores[i][j], 0, 1);
        }
    }

    pid_t pid1, pid2;
    struct GardenerParams params1 = {1, 0, 0, v1};
    struct GardenerParams params2 = {2, M - 1, N - 1, v2};

    if ((pid1 = fork()) == 0) {
        gardener(&params1);
        exit(0);
    } else if ((pid2 = fork()) == 0) {
        gardener(&params2);
        exit(0);
    } else {
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
    }

    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            sem_destroy(&semaphores[i][j]);
        }
    }

    return 0;
}
