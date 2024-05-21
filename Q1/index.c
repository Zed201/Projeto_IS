#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define x 4  // Número de threads

// Variáveis globais
int contador = 0;
pthread_mutex_t mutex;

// Estrutura para passar argumentos
typedef struct {
    const char *arquivo;
    const char *palavra;
} Args;

void *busca(void *args);

int main(int argc, char *argv[]) {

    const char *word = "brasil";

    // Inicializa o mutex
    pthread_mutex_init(&mutex, NULL);

    // Criando as threads
    pthread_t thread[x];
    Args threadArgs[x];

    for (int i = 0; i < x; i++) {
        threadArgs[i].arquivo = argv[i + 1];
        threadArgs[i].palavra = word;

        if (pthread_create(&thread[i], NULL, busca, &threadArgs[i]) != 0) {
            printf("Erro ao criar a thread\n");
            return 1;
        }
    }

    for (int j = 0; j < x; j++) {
        pthread_join(thread[j], NULL);
    }

    // Destrói o mutex
    pthread_mutex_destroy(&mutex);

    printf("Total de ocorrências: %d\n", contador);

    return 0;
}

// Função executada por cada thread
void *busca(void *args) {
    Args *threadArgs = (Args *)args;
    FILE *file;
    const char *arquivo = threadArgs->arquivo;
    const char *palavra = threadArgs->palavra;
    int count = 0;
    int tam = 1000;
    char texto[tam];

    file = fopen(arquivo, "r");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        pthread_exit(NULL);
    }

    while (fgets(texto, tam, file)) {
        char *linha = texto; // Aponta para o início do texto
        while ((linha = strstr(linha, palavra)) != NULL) {
            count++;
            linha += strlen(palavra); // Atualiza a posição do ponteiro
        }
    }
    fclose(file);

    // Atualizando contador global
    pthread_mutex_lock(&mutex);
    contador += count;
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}
