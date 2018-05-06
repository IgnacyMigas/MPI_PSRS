#include "utilities.h"

int multimerge(int *start[], const int lengths[], const int Number, int newArray[], const int newArrayLength){
    int index, i, j, min;
    int pivots[Number];
    for(i=0; i<Number; i++){
        pivots[i] = 0;
    }
    for(i=0; i<newArrayLength; ++i){
        min = MAX_RANDOM;
        index = -1;
        for(j=0; j<Number; ++j){
            if(pivots[j]<lengths[j] && min>start[j][pivots[j]]){
                min = start[j][pivots[j]];
                index = j;
            }
        }
        if(index != -1){
            newArray[i] = min;
            ++pivots[index];
        }
        else{
            printf("ERROR: cannot find right element!");
        }
    }
}}

int compare_ints(const void *a, const void *b)
{
    int arg1 = *(const int*)a;
    int arg2 = *(const int*)b;
 
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

void printArray(int myid, char *arrayName, int array[], int length)
{
    int i;
	for(i=0; i<length; i++)
	{
		printf("%d: %s[%d] = %d\n", myid, arrayName, i, array[i]);
	}
}
