#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#define TNUM 9

pthread_mutex_t mutex_cond = PTHREAD_MUTEX_INITIALIZER;
int condição = 1; // Valido é 1 e invalido é 0

struct Cords{
        int altura, largura;
        int** sudoku;
};

void *verificar_grade(void *dados) {
        struct Cords crd = *((struct Cords*) dados);
        printf("%d-%d", crd.altura, crd.largura);
        printf("Entrou");
        for (int i = 0; i < 9; i++) {
                for (int j = 0; j < 9; j++) {
                        printf("%d ", crd.sudoku[i][j]);
                }
                printf("\n");
        }
        //pthread_exit(NULL);
        return NULL;
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
        
        // void *p = verificar_grade(Ccords(valid_, 1, 2));
        return 0;
}
