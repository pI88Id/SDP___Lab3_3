#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <zconf.h>
#include <string.h>

struct Parameters {
    int *v;
    int left;
    int right;
    int threshold;
};
struct Parameters Parameters;

void* quicksort_t (void* parameters);
void quicksort_r(int *v, int left, int right);
void swap(int* i, int* j);
struct Parameters setParameters (int *v, int left, int right, int threshold);


int main(int argc, char* argv[]) {
    struct Parameters parameters;
    struct stat st;
    int bFile, i=0, length=0;
    pthread_t tid;
    char* tmp;

    //int v[10] = {1, 43, 65, 2, 8, 90, 6, 2, 12, 76};
    int *v;
    char* vec,*ve;

    if (argc != 3){
        fprintf(stderr, "ERROR - %s <path> <threshold>\n", argv[0]);
        return -1;
    }
    if ((bFile = open(argv[1], O_RDONLY, 0)) == -1){
        fprintf(stderr, "ERROR - Impossible open file\n");
        return -1;
    }

    stat(argv[1], &st);

    if ((vec = mmap(NULL, st.st_size, PROT_WRITE, MAP_PRIVATE, bFile, 0)) == MAP_FAILED) {
        fprintf(stderr, "ERROR - Opening mmap file\n");
        return -4;
    }

    v = (int*)malloc(st.st_size);
    i = 0;
    while (i < st.st_size) {
        if (vec[i] != ' ') {
            v[length] += atoi(&vec[i]);
        }else length++;
        i++;
    }
    length++;

    close(bFile);

    printf("Vettore da ordinare:\n\t");
    for (int i = 0;i<length; i++)
        printf("%d ", v[i]);
    printf("\n");

    parameters = setParameters(v, 0, 9, (int) atoi(argv[2]));

    if (pthread_create(&tid, NULL, (void*)quicksort_t, (void*) &parameters)) return -10;
    if (pthread_join(tid, NULL)) return -10;

    printf("Vettore ordinato:\n\t");
    for (int i = 0;i<length; i++){
        printf("%d ", v[i]);
    }

    munmap(vec, st.st_size);

    return 0;
}

void* quicksort_t (void* param) {
    int i, j, x, pivot;
    pthread_t tid1, tid2;
    struct Parameters *parameters, param1, param2;

    parameters = (struct Parameters*)param;

    pivot = (parameters->right+parameters->left)/2;

    x = parameters->v[pivot];
    i = parameters->left - 1;
    j = parameters->right + 1;
    while (i < j) {
        while (parameters->v[--j] > x);
        while (parameters->v[++i] < x);
        if (i < j) swap (&parameters->v[i], &(parameters->v[j]));
    }

    if (parameters->right-parameters->left >= parameters->threshold){
        param1 = setParameters(parameters->v, parameters->left, pivot-1, parameters->threshold);
        param2 = setParameters(parameters->v, pivot+1, parameters->right, parameters->threshold);

        if (pthread_create(&tid1, NULL, (void*)quicksort_t, (void*) &param1)) pthread_exit((void*)-2);
        if (pthread_create(&tid2, NULL, (void*)quicksort_t, (void*) &param2)) pthread_exit((void*)-2);

        if (pthread_join(tid1, NULL)) pthread_exit((void*)-3);
        if (pthread_join(tid2, NULL)) pthread_exit((void*)-3);

    }else quicksort_r(parameters->v, parameters->left, parameters->right);

    pthread_exit(NULL);
}

void swap(int* i, int* j){
    int tmp;
    tmp = *i;
    *i = *j;
    *j = tmp;
}

struct Parameters setParameters (int *v, int left, int right, int threshold){
    struct Parameters p;

    p.v = v;
    p.left = left;
    p.right = right;
    p.threshold = threshold;

    return p;
}

void quicksort_r(int *v, int left, int right) {
    int i, j, x,pivot, tmp;
    if (left >= right){
        return;
    }
    pivot = (right+left)/2;

    x = v[pivot];
    i = left - 1;
    j = right + 1;
    while (i < j) {
        while (v[--j] > x);
        while (v[++i] < x);
        if (i < j){
            tmp = v[i];
            v[i] = v[j];
            v[j] = tmp;
        }
    }

    quicksort_r(v, left, pivot-1);
    quicksort_r(v, pivot+1, right);
}
