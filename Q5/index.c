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
int B[I] = {13, 11, 10, 12};            // Vetor (matriz coluna) dos resultados das equações;

// Definir a barreira;
pthread_barrier_t barrier;

// Função em que as threads irão executar;
void *jacobi(void *arg);            // Método de Jacobi;

// Funções auxiliares se necessário;

int main()
{
    // Variáveis auxiliares;
    int N, i;

    // Determinar a quantidade N de threads;
    printf("Digite a quantidade de processadores (ou núcleos): ");
    scanf("%d", &N);

    // Variável das threads;
    pthread_t threads[N];           // N threads para o método de Jacobi;

    // Variáveis auxiliares;

    // Inicializar a barreira para N threads;
    pthread_barrier_init(&barrier, NULL, N);

    // Criar N threads para o método de Jacobi;
    for (i = 0; i < N; i++)
    {
        //
        pthread_create(&threads[i], NULL, jacobi, NULL); //ajustar argumento se precisar
    }

    // Aguardar a conclusão das N threads;
    for (i = 0; i < N; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // Destruir a barreira;
    pthread_barrier_destroy(&barrier);

    // Imprimir a solução do sistema;
    printf("Solução do sistema:\n");
    for (i = 0; i < I; i++)
    {
        printf("x%d = %d\n", i + 1, B[i]); //não será B[i], mas sim a solução do sistema;
    }

    // Encerrar o programa;
    pthread_exit(NULL);
    return 0;                       // Não deve ser executado, mas é necessário por boas práticas e para evitar warnings;
}

// Método de Jacobi;
void *jacobi(void *arg)             // Não pode criar thread aqui dentro;
{

}