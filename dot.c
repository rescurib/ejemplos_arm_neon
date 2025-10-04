#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define ARRAY_SIZE 4096
#define ITERATIONS 1000
float time_diff(struct timespec start, struct timespec stop);

int main(char arg, char** argv){
    int i,j;
    
    float* array_a = (float*) aligned_alloc(16,ARRAY_SIZE*sizeof(float));
    float* array_b = (float*) aligned_alloc(16,ARRAY_SIZE*sizeof(float));
    float  result = 0;

    struct timespec start,stop;
    double exec_time;

    srand(172);

    for(i=0;i<ARRAY_SIZE;i++)
    {
        array_a[i] = (float)rand()/RAND_MAX;
        array_b[i] = (float)rand()/RAND_MAX;
    }

    for(i=0;i<ITERATIONS;i++)
    {
        result = 0;
        clock_gettime(CLOCK_MONOTONIC,&start);
        for(j=0;j<ARRAY_SIZE;j++)
        {
            result += array_a[j]*array_b[j];
        }
        clock_gettime(CLOCK_MONOTONIC,&stop);
        exec_time += time_diff(start,stop);
    }

    printf("Resultado: %.4f \n", result);
    printf("Tiempo de ejecución: %.4f nanosegundos\n",exec_time / ITERATIONS);

    free(array_a);
    free(array_b);

    return 0;
}

float time_diff(struct timespec start, struct timespec stop){
    return (stop.tv_sec -start.tv_sec)*1e9 + (stop.tv_nsec - start.tv_nsec);
}
