#ifndef UTILITIES_H
#define UTILITIES_H

#define MAX_RANDOM 500

#include <stdio.h>
#include <stdlib.h>

int multimerge(int *start[], const int lengths[], const int Number, int newArray[], const int newArrayLength);

int compare_ints(const void *a, const void *b);

void printArrayAtOnce(int myid, char *msg, int array[], int length);

void printArrayAtOnceStart(int myid, char *msg, int array[], int start, int length);

#endif
