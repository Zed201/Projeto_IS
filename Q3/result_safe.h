#include <pthread.h>
#include <stdlib.h>

#ifndef RESULT_H
#define RESULT_H

typedef struct resultado resultado;
typedef struct cliente cliente;

struct resultado
{
    int status;
    int value;
};

struct cliente
{
    int id;
    resultado result;
    pthread_mutex_t mu_result;
    pthread_cond_t cond_result;
};

// operacao atomica para
void initCliente(cliente **cl, int id)
{
    cliente *new_client = (cliente *)malloc(sizeof(cliente));
    new_client->id = id;
    new_client->result.status = -1;
    new_client->result.value = -1;

    pthread_mutex_init(&new_client->mu_result, NULL);
    pthread_cond_init(&new_client->cond_result, NULL);
    *cl = new_client;
}

void sendResult(cliente *cl)
{
}

#endif