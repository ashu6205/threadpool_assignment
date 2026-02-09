#include <stdio.h>
#include <stdlib.h>

void leak(int i) {
    int *arr = malloc(sizeof(int) * 50);
    arr[0] = i;

    if (i % 10000 == 0) {
        printf("Allocated %d blocks\n", i);
    }
}

int main() {
    for (int i = 1; i <= 100000; i++) {
        leak(i);
    }
    getchar(); // keep process alive
    return 0;
}
