#ifndef UTILITIES_H
#define UTILITIES_H

#define MAX_RANDOM 500

#include <stdio.h>
#include <stdlib.h>

int multimerge(int *start[], const int lengths[], const int Number, int newArray[], const int newArrayLength);

int compare_ints(const void *a, const void *b);

void printArray(int myid, char *arrayName, int array[], int start, int length);

void printArray1(int myid, char *arrayName, int array[], int length);

void printArrayAtOnce(int myid, char *msg, int array[], int length);

#endif
