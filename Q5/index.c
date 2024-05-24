#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/***************************************************************************************************************/
//Pode alterar os valores de I, P e do SEL para outros testes (lembrar de mudar as matrizes quando necessário);//
/***************************************************************************************************************/

// Parâmetros de teste;
#define I 3                         // Número de equações;
#define P 10                        // Número de iterações de refino;

                /////////////////////////////
                //   O sistema exemplo é:  //
                //  3w + 7x + 5y + z = 13  //
                //   w +  x + 2y + 3z= 11  //
                //  2w + 5x + 6y + 7z= 10  //
                //  4w + 2x + 5y + 2z= 12  //
                /////////////////////////////

// Matrizes exemplo para o médoto de Jacobi;
int A[I][I] = {                     // Matriz dos coeficientes das equações;
    {3, 7, 5, 1},
    {1, 1, 2, 3},
    {2, 5, 6, 7},
    {4, 2, 5, 2}
};
int B[I] = {13, 11, 10, 12};        // Vetor (matriz coluna) dos resultados das equações;
int X[I] = {1, 1, 1, 1};            // Vetor (matriz coluna) dos resultados iniciais das equações (1 por padrão);

// Definir a barreira;
pthread_barrier_t barrier;

// Função em que as threads irão executar;
void *jacobi(void *arg);            // Método de Jacobi;

// Funções auxiliares se necessário;
//

int main()
{
    // Variáveis auxiliares;
    int N, i, j;

    // Determinar a quantidade N de threads;
    printf("Digite a quantidade de processadores (ou núcleos): ");
    scanf("%d", &N);

    // Variável das threads;
    pthread_t threads[N];           // N threads para o método de Jacobi;

    // Variável de argumentos das threads;
    int *ids[N];                     // Identificadores das threads;

    // Inicializar a barreira para N threads;
    pthread_barrier_init(&barrier, NULL, N);

    // Criar N threads para o método de Jacobi;
    for (i = 0; i < N; i++)
    {
        ids[i] = (int *)malloc(sizeof(int));
        *ids[i] = i;    // acredito que seja melhor definir o começo dentro da função, para podermos ter o i atual tbm;
        pthread_create(&threads[i], NULL, jacobi, (void *)ids[i]);
    }

    // Aguardar a conclusão das N threads;
    for (i = 0; i < N; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // Destruir a barreira;
    pthread_barrier_destroy(&barrier);

    // Desalocar os argumentos das threads;
    for (i = 0; i < N; i++)
    {
        free(ids[i]);
    }

    // Imprimir sistema de equações;
    printf("Sistema de equações:\n");
    for (i = 0; i < I; i++)
    {
        for (j = 0; j < I; j++)
        {
            printf("%dX[%d] ", A[i][j], j + 1);
            if (j < I - 1)
            {
                printf("+ ");
            }
            else 
            {
                printf("= %d\n", B[i]);
            }
        }
    }

    // Imprimir a solução do sistema;
    printf("Solução do sistema:\n");
    for (i = 0; i < I; i++)
    {
        printf("X[%d] = %d\n", i + 1, X[i]);
    }

    // Encerrar o programa;
    pthread_exit(NULL);
    return 0;                       // Não deve ser executado, mas é necessário por boas práticas e para evitar warnings;
}

// Método de Jacobi;
void *jacobi(void *arg)
{
    // com o i definir o começo da iteração;
    // definir o final e ajustar para o ultimo i (i==N-1);
    // loop de refinos;
        // equações de Jacobi (essa equação tem outro loop, provavelmente baseado em I);
            // atualizar o valor de cada X[i];
        // barreira;
    // fim do loop de refinos;
    // encerrar a thread;
}