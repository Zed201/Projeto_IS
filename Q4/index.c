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
        // verificar primeiro o 3x3 que ele iniciou
        int aux_3x3 = 0;
        for (int i = crd.largura; i < crd.largura + 3; i++) {
                for (int j = crd.altura; j < crd.altura + 3; j++) {
                        // fazer verificar a condiçao( se ja for invalido ele para)
                        if (condicao != 1) {
                                pthread_exit(NULL);
                        }
                        int bit_3x3 = 1 << crd.sudoku[i][j];
                        if(aux_3x3 & bit_3x3){
                                // invalido
                                // mudar variavel de condiçao, com mutex

                                pthread_exit(NULL);
                        }
                        aux_3x3 |= bit_3x3;
                        // verifica a linha do elemento
                        int aux_linha = 0;
                        for (int _j = 0; _j < 9; _j++) {
                                if (condicao != 1) {
                                        pthread_exit(NULL);
                                }
                                int bit_linha = 1 << crd.sudoku[i][_j];
                                if(aux_linha & bit_linha){
                                        // invalido
                                }
                                aux_linha |= bit_linha;
                        }
                        // verificar a coluna
                        int aux_coluna = 0;
                        for (int _i = 0; _i < 9; _i++) {
                                if (condicao != 1) {
                                        pthread_exit(NULL);
                                }
                                int bit_coluna = 1 << crd.sudoku[_i][j];
                                if(aux_coluna & bit_coluna){
                                        // invalido
                                }
                                aux_coluna |= bit_coluna;
                        }
                }

        } 
        pthread_exit(NULL);
}

struct Cords* Ccords(int** sudoku, int l, int a){
        struct Cords* ptr = malloc(sizeof(struct Cords));
        ptr->largura = l;
        ptr->altura = a;
        ptr->sudoku = sudoku;
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

        // fazer leitura dos arquivos(no caso pegar como scanf passando por > os dados)
        // for (int i = 0; i < TNUM; i++) {
        //         pthread_create(&ids_threads[i], NULL, /*funcao*/, /*Posicao de começar, fazer for duplo*/)
        // }
        // for (int i = 0; i < TNUM; i++){
        //         pthread_join(ids_threads[i], NULL)
        // }
        // if(condição == 1){
        //         printf("Sudoku Válido\n");
        // } else {
        //         printf("Sudoku Inválido\n");
        // }

        verificar_grade(Ccords(matrix, 1, 2));
        for (int i = 0; i < 9; i++) {
                free(matrix[i]);
        }
        free(matrix);

        return 0;
}
