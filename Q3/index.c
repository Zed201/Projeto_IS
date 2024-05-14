#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "queue.h"

#define N_CLIENTES 5
#define MAX_OPERACOES 250

enum operat {saque, deposito, consulta};
enum status {ok, fail, waiting};

typedef struct operacao {
    int id;
    int op;
    int value;
    int* retorno;   
} operacao;

typedef struct resultado {
    int status;
    int value;
} resultado;

// fila para registrar as operacoes e o mutex para assegurar a regiao critica
opQueue* queue;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mu_queue = PTHREAD_MUTEX_INITIALIZER;

// condição para avisar o cliente que a operação solicitada foi realizada
pthread_cond_t cond_clientes[N_CLIENTES];
// array para guardar o resultado da operação dos clientes
resultado *result_clientes[N_CLIENTES];
pthread_mutex_t *mu_result[N_CLIENTES];

void setResult(int idCliente, resultado r);

void *banco(void *ptr) {
    int contas[N_CLIENTES];
    
    for (int i = 0; i < N_CLIENTES; i++) {
        contas[i] = 0;
    }

    for (int i = 0; i < MAX_OPERACOES; i++) {
        pthread_mutex_lock(&mu_queue);
        while (isEmpty(queue)) {
            pthread_cond_wait(&empty, &mu_queue);
        }

        operacao* req;
        req = dequeue(queue);

        // pensando se preciso de um mutex
        resultado* res = result_clientes[req->id];

        if (req->op == consulta) {
            res->status = ok;
            res->value = contas[req->id];
        }
        else if (req->value < 0) {
            res->status = fail;
            res->value = -1;
        }
        else if (req->op == deposito)
        {
            contas[req->id] += req->value;

            res->status = ok;
            res->value = -1;
        }
        else if (req->op == saque) {
            if (contas[req->id] - req->value >= 0) {
                contas[req->id] -= req->value;

                res->status = ok;
                res->value = -1;
            }
            else {
                res->status = fail;
                res->value = -1;
            }
        }

        pthread_cond_signal(&cond_clientes[req->id]);

        if (req != NULL)
            free(req);
        pthread_mutex_unlock(&mu_queue);
    }

}

void *cliente(void *id) {

}

int main() {
    pthread_t banco_t;
    pthread_t clientes_t[N_CLIENTES];

    initQueue(&queue);

    for (int i = 0; i < N_CLIENTES; i++) {
        result_clientes[i] = (resultado *) malloc(sizeof(resultado));
    }

    for (int i = 0; i < N_CLIENTES; i++) {
        pthread_cond_init(&cond_clientes[i], NULL);
    }

    pthread_create(&banco_t, NULL, &banco, NULL);

    for (int i = 0; i < N_CLIENTES; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&clientes_t[i], NULL, &cliente, (void *) id);
    }

    for (int i = 0; i < N_CLIENTES; i++) {
        pthread_join(clientes_t[i], NULL);
    }
    pthread_join(banco_t, NULL);
    
}
