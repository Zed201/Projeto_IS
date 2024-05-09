#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#define TNUM 9

pthread_mutex_t mutex_cond = PTHREAD_MUTEX_INITIALIZER;
int condicao = 1; // Valido é 1 e invalido é 0

struct Cords{
        int altura, largura;
        int** sudoku;
};

void *verificar_grade(void *dados) {
        struct Cords crd = *((struct Cords*) dados);
        // printf("Verificando %d-%d\n", crd.largura, crd.altura);
        // verificar primeiro o 3x3 que ele iniciou
        int aux_3x3 = 0;
        for (int i = crd.largura; i < crd.largura + 3; i++) {
                for (int j = crd.altura; j < crd.altura + 3; j++) {
                        // fazer verificar a condiçao( se ja for invalido ele para)
                        // printf("Grade_ver %d-%d\n", i,j);
                        if (!condicao) {
                                pthread_exit(NULL);
                                // return NULL;
                        }
                        int bit_3x3 = 1 << crd.sudoku[i][j];
                        // printf("Verificando elemento %d[%d][%d]\n", crd.sudoku[i][j], i, j);
                        if(aux_3x3 & bit_3x3){
                                // invalido
                                // mudar variavel de condiçao, com mutex
                                pthread_mutex_lock(&mutex_cond);
                                condicao = 0;
                                pthread_mutex_unlock(&mutex_cond);

                                pthread_exit(NULL);
                        }
                        aux_3x3 |= bit_3x3;
                        // verifica a linha do elemento
                        int aux_linha = 0;
                        for (int _j = 0; _j < 9; _j++) {
                                if (!condicao) {
                                        pthread_exit(NULL);
                                        // return NULL;
                                }
                                int bit_linha = 1 << crd.sudoku[i][_j];
                                // printf("Linha ver %d-%d\n", i, _j);
                                if(aux_linha & bit_linha){
                                        // invalido
                                        pthread_mutex_lock(&mutex_cond);
                                        condicao = 0;
                                        pthread_mutex_unlock(&mutex_cond);

                                        pthread_exit(NULL);
                                }
                                aux_linha |= bit_linha;
                        }
                        // verificar a coluna
                        int aux_coluna = 0;
                        for (int _i = 0; _i < 9; _i++) {
                                if (!condicao) {
                                        pthread_exit(NULL);
                                        // return NULL;
                                }
                                int bit_coluna = 1 << crd.sudoku[_i][j];
                                // printf("Coluna Ver %d-%d\n", _i, j);
                                if(aux_coluna & bit_coluna){
                                        // invalido
                                        pthread_mutex_lock(&mutex_cond);
                                        condicao = 0;
                                        pthread_mutex_unlock(&mutex_cond);

                                        pthread_exit(NULL);
                                }
                                aux_coluna |= bit_coluna;
                        }
                }

        } 
        printf("Terminando\n");
        pthread_exit(NULL);
}

struct Cords Ccords(int** sudoku, int l, int a){
        struct Cords ptr;
        ptr.largura = l;
        ptr.altura = a;
        ptr.sudoku = sudoku;
        return ptr;
};

int main(){
        pthread_t ids_threads[TNUM];
        // parte de ler a matriz pelo scanf  
        int** matrix = (int **)malloc(sizeof(int*) * 9);
        for (int i = 0; i < 9; i++) {
                matrix[i] = (int *)malloc(sizeof(int) * 9);
                for (int j = 0; j < 9; j++) {
                        scanf("%d ", &matrix[i][j]);
                }
        }
        struct Cords *cordenadas = malloc(sizeof(struct Cords) * 9);
        // for (int i = 0; i < 3; i++) {
        //         for (int j = 0; j < 3; j++) {
        //                 cordenadas[i]  Ccords(matrix, 3 * i, 3 * j));
        //                 // verificar_grade(Ccords(matrix, i * 3, j * 3));
        //         }
        // }

        cordenadas[0] = Ccords(matrix, 3 * 0, 3 * 0);
        cordenadas[1] = Ccords(matrix, 3 * 1, 3 * 0);
        cordenadas[2] = Ccords(matrix, 3 * 2, 3 * 0);
        cordenadas[3] = Ccords(matrix, 3 * 0, 3 * 1);
        cordenadas[4] = Ccords(matrix, 3 * 1, 3 * 1);
        cordenadas[5] = Ccords(matrix, 3 * 2, 3 * 1);
        cordenadas[6] = Ccords(matrix, 3 * 0, 3 * 2);
        cordenadas[7] = Ccords(matrix, 3 * 1, 3 * 2);
        cordenadas[8] = Ccords(matrix, 3 * 2, 3 * 2);
        for (int i = 0; i < 9; i++) {
             pthread_create(&ids_threads[i], NULL, verificar_grade, (void *)&cordenadas[i]);
        }
        for (int i = 0; i < TNUM; i++){
                printf("Thread %d terminou\n", i);
                pthread_join(ids_threads[i], NULL);
}
        // tava dando seg fault pois eu tava colocando o ponteiro direto no create da thread
        // printf("Threads Terminaram\n");
        if(condicao == 1){
                printf("Sudoku Válido\n");
        } else {
                printf("Sudoku Inválido\n");
        }

        for (int i = 0; i < 9; i++) {
                free(matrix[i]);
        }
        free(matrix);

        return 0;
}
