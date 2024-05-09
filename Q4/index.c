#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

pthread_mutex_t mutex_cond = PTHREAD_MUTEX_INITIALIZER; // Mutex so para modificar essa condicao
int condicao = 1; // Valido é 1 e invalido é 0

// Struct para passar informações para cada Thread
struct Cords{
        int altura, largura;
        int** sudoku;
};

void *verificar_grade(void *dados) {
        struct Cords crd = *((struct Cords*) dados);
        // cada thread vai ficar responsável por um dos 9 quadrantes 3x3
        int aux_3x3 = 0;
        for (int i = crd.largura; i < crd.largura + 3; i++) {
                for (int j = crd.altura; j < crd.altura + 3; j++) {

                        if (!condicao) { // se ja for invalido ele só sai de cara
                                pthread_exit(NULL);
                        }
                        /*
                         * Basicamente tô usando bitwise para verificar a existencia de numeros repetidos, evitando mais for encadeado
                         * esse left_shift ele desloca o 1 o numero de casas do digito correspondente
                         * logo teriamos coisas como 100 para o numero 2, 1000 para o 3...
                         *
                         * O aux_3x3 basicamente é um acumulador e funciona como se fosse um 'vetor', de bits apenas,
                         * em que cada bit representa o numero correspondete a sua posicao, logo para verificar a existencia 
                         * de algum numero nesse 'array' eu basicamente posso fazer um and bit a bit, se ele resultar 
                         * em algo diferente de zero, o numero ja esta nesse 'vetor' e o sudoku e invalido, 
                         * se nao ele depois faz um or bit a bit para 'adicionar' esse 1 ao 'vetor' na posicao certa
                         *  Ex:
                         *  1011000010 -> {9, 1, 6, 7}
                         *  0100000000 -> {8}
                         *  O 8 nao ta no vetor entao nao entra no if, pois o and bit a bit vai dar 0, ai or bit a bit
                         *  vai adicionar ele no 'vetor'
                         *  1111000010 -> {1, 6, 7, 8, 9}
                         *
                         * */
                        int bit_3x3 = 1 << crd.sudoku[i][j];

                        if(aux_3x3 & bit_3x3){
                                // Se entrar nesse if ele e invalido
                                pthread_mutex_lock(&mutex_cond);
                                condicao = 0;
                                pthread_mutex_unlock(&mutex_cond);

                                pthread_exit(NULL);
                        }

                        aux_3x3 |= bit_3x3;

                        // verifica a linha do elemento
                        // a logca é a mesma do bitwise, ele basicamente so tá com um aux diferente para não interferir
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
                        // Mesma coisa dos outros dois
                        int aux_coluna = 0;
                        for (int _i = 0; _i < 9; _i++) {
                                if (!condicao) {
                                        pthread_exit(NULL);
                                }
                                int bit_coluna = 1 << crd.sudoku[_i][j];
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
        // se tudo for valido naquele setor de 3x3, 
        // seja linha, coluna ou entre os proprio elementos do 3x3 ele so termina
        pthread_exit(NULL);
}

// funcao só para faciliar a criacao das coordenadas
struct Cords Ccords(int** sudoku, int l, int a){
        struct Cords ptr;
        ptr.largura = l;
        ptr.altura = a;
        ptr.sudoku = sudoku;
        return ptr;
};

int main(){
        // Matriz de "Threads" no 'mesmo formato' das grades para facilitar
        pthread_t ids_threads[3][3];
        // leitura da matriz, usando o < como entrada do executavel
        int** matrix = (int **)malloc(sizeof(int*) * 9);
        for (int i = 0; i < 9; i++) {
                matrix[i] = (int *)malloc(sizeof(int) * 9);

                for (int j = 0; j < 9; j++) {
                        scanf("%d ", &matrix[i][j]);
                }
        }
        // O 'vetor' com as coordenadas(tentei instanciar elas direto na chamada do pthread_create mas ele da seg fault)
        struct Cords **cordenadas = (struct Cords **)malloc(sizeof(struct Cords*) * 3);
        
        // inicializacao das coordenadas e das threads logo em seguida
        for (int i = 0; i < 3; i++) {
                cordenadas[i] = (struct Cords*) malloc(sizeof(struct Cords) * 3);

                for (int j = 0; j < 3; j++) {
                        cordenadas[i][j] = Ccords(matrix, 3 * i, 3 * j);
                        pthread_create(&ids_threads[i][j], NULL, verificar_grade, (void *)&cordenadas[i][j]);
                }
        }
        // Join das threads
        for (int i = 0; i < 3; i++){
                for (int j = 0; j < 3; j++) {
                        pthread_join(ids_threads[i][j], NULL);
                }
        }
        // verifica a condicao e da a resposta equivalente
        if(condicao == 1){
                printf("Sudoku Válido\n");
        } else {
                printf("Sudoku Inválido\n");
        }
        
        // limpa tudo
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
