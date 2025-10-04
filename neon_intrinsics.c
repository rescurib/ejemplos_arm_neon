#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <arm_neon.h>

#define ARRAY_SIZE 2048
#define ITERATIONS 50

void mult_and_acc(const float* a, const float* b, float* result, int size) 
{
    *result = 0;
    for(int i = 0; i<size; i++)
    {
        *result += a[i]*b[i];
    }
}

void neon_mult_and_acc(const float* a, const float* b, float* result, int size)
{
    float32x4_t va, vb;
    float32x4_t vacc = vdupq_n_f32(0.0f);
    int i;
    for(i = 0; i<(size-4); i += 4)
    {
        va     = vld1q_f32(&a[i]);
        vb     = vld1q_f32(&b[i]);
        vacc   = vmlaq_f32(vacc,va,vb);
    }

    float32x2_t sum_vectors = vadd_f32(vget_low_f32(vacc),vget_high_f32(vacc));
    *result = vget_lane_f32(sum_vectors,0) + vget_lane_f32(sum_vectors,1);

    // Elementos restantes
    for(; i<size; i++)
    {
        *result += a[i]*b[i];
    }
}

float get_time_diff(struct timespec start, struct timespec end){
    return (end.tv_sec - start.tv_sec)*1e3 + (end.tv_nsec - start.tv_nsec)/1e6;
}

void main(int argc, char** argv){
    int i;
    float* array_a       = (float*)aligned_alloc(16,ARRAY_SIZE * sizeof(float));
    float* array_b       = (float*)aligned_alloc(16,ARRAY_SIZE * sizeof(float));
    float scalar_result = 0.0f;
    float neon_result   = 0.0f;
    
    struct timespec start, end;

    srand(172);
    for(i = 0; i < ARRAY_SIZE; i++)
    {
        array_a[i] = (float)rand()/RAND_MAX;
        array_b[i] = (float)rand()/RAND_MAX;
    }

        double scalar_time = 0.0;
        double neon_time = 0.0;
        struct timespec t0, t1;

        // Scalar timing: measure each iteration separately and accumulate
        for(i=0; i < ITERATIONS; i++)
        {
                clock_gettime(CLOCK_MONOTONIC, &t0);
                mult_and_acc(array_a, array_b, &scalar_result, ARRAY_SIZE);
                clock_gettime(CLOCK_MONOTONIC, &t1);
                scalar_time += get_time_diff(t0, t1);
        }
        printf("Resultado escalar: %.4f \n", scalar_result);
        printf("Tiempo de ejecución escalar: %.4f microsegundos \n", scalar_time / ITERATIONS);

    clock_gettime(CLOCK_MONOTONIC,&start);
    for(i=0; i < ITERATIONS; i++)
    {
      neon_mult_and_acc(array_a,array_b,&neon_result, ARRAY_SIZE);
    }
    clock_gettime(CLOCK_MONOTONIC,&end);
    double neon_time = get_time_diff(start,end);
    printf("Resultado escalar: %.4f \n",neon_result);
    printf("Tiempo de ejecución SIMD: %.f microsegundos \n",neon_time);
    
    printf("Porcentaje de mejora: %.2f%% \n", (float)(scalar_time/neon_time)*100);
}
