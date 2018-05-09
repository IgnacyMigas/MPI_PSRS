#include "utilities.h"

int multimerge(int *starts[], const int lengths[], const int Number, int newArray[], const int newArrayLength) {
    int index, i, j, min;
    int pivots[Number];
    for (i = 0; i < Number; i++) {
        pivots[i] = 0;
    }
    for (i = 0; i < newArrayLength; ++i) {
        min = RAND_MAX;
        index = -1;
        for (j = 0; j < Number; ++j) {
            if (pivots[j] < lengths[j] && min > starts[j][pivots[j]]) {
                min = starts[j][pivots[j]];
                index = j;
            }
        }
        if (index != -1) {
            newArray[i] = min;
            ++pivots[index];
        } else {
            return -1;
        }
    }
    return 0;
}

int compare_ints(const void *a, const void *b) {
    int arg1 = *(const int *) a;
    int arg2 = *(const int *) b;

    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

void printArrayAtOnce(int myid, char *msg, int array[], int length) {
    char tmp_data[length * 8];
    int tmp_index = 0;
    for (int i = 0; i < length; i++) {
        tmp_index += sprintf(&tmp_data[tmp_index], "%d ", array[i]);
    }
    printf("[process-%d] %s %s\n", myid, msg, tmp_data);
}