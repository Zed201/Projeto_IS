#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

pthread_mutex_t mutex_cond = PTHREAD_MUTEX_INITIALIZER;
int condicao = 1; // Valido é 1 e invalido é 0

struct Cords{
        int altura, largura;
        int** sudoku;
};

void *verificar_grade(void *dados) {
        struct Cords crd = *((struct Cords*) dados);
        int aux_3x3 = 0;
        for (int i = crd.largura; i < crd.largura + 3; i++) {
                for (int j = crd.altura; j < crd.altura + 3; j++) {
                        if (!condicao) {
                                pthread_exit(NULL);
                        }
                        int bit_3x3 = 1 << crd.sudoku[i][j];
                        if(aux_3x3 & bit_3x3){
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
                                }
                                int bit_linha = 1 << crd.sudoku[i][_j];
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
        pthread_t ids_threads[3][3];
        // parte de ler a matriz pelo scanf  
        int** matrix = (int **)malloc(sizeof(int*) * 9);
        for (int i = 0; i < 9; i++) {
                matrix[i] = (int *)malloc(sizeof(int) * 9);
                for (int j = 0; j < 9; j++) {
                        scanf("%d ", &matrix[i][j]);
                }
        }
        struct Cords **cordenadas = (struct Cords **)malloc(sizeof(struct Cords*) * 3);


        for (int i = 0; i < 3; i++) {
                cordenadas[i] = (struct Cords*) malloc(sizeof(struct Cords) * 3);
                for (int j = 0; j < 3; j++) {
                        cordenadas[i][j] = Ccords(matrix, 3 * i, 3 * j);
                        pthread_create(&ids_threads[i][j], NULL, verificar_grade, (void *)&cordenadas[i][j]);
                }
        }
        for (int i = 0; i < 3; i++){
                for (int j = 0; j < 3; j++) {
                        pthread_join(ids_threads[i][j], NULL);
                }
        }
        if(condicao == 1){
                printf("Sudoku Válido\n");
        } else {
                printf("Sudoku Inválido\n");
        }

        for (int i = 0; i < 9; i++) {
                free(matrix[i]);
        }
        free(matrix);
        for (int i = 0; i < 3; i++) {
                free(cordenadas[i]);
        }
        free(cordenadas);

        return 0;
}
