#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#ifndef RESULT_H
#define RESULT_H

// status para o retorno da operacao do cliente
enum status
{
    ok,
    fail,
    waiting,
    off = -1
};

typedef struct resultado resultado;
typedef struct cliente cliente;

// struct para representar o resultado de uma operacao
struct resultado {
    int status; // status final da operacao
    char msg[150];  // valor que pode ser retornado devido ao tipo da operacao
};

// struct que representa a interface de comunicacao com o cliente
struct cliente {
    int id;                     // id do cliente
    resultado result;           // resultado da ultima operacao
    pthread_mutex_t mu_result;  // mutex para a regiao critica -> result
    pthread_cond_t cond_result; // condicao para acordar quem estiver esperando o result
};

// inicia a struct do cliente
void initCliente(cliente **cl, int id) {
    cliente *new_client = (cliente *)malloc(sizeof(cliente));
    new_client->id = id;
    new_client->result.status = off;
    strcpy(new_client->result.msg, "");

    pthread_mutex_init(&new_client->mu_result, NULL);  // inicia os mutex
    pthread_cond_init(&new_client->cond_result, NULL); // inicia as condicoes
    *cl = new_client;
}

// operacao atomica para enviar o resultado de uma operacao para o cliente
void sendResult(cliente *cl, resultado r) {
    pthread_mutex_lock(&cl->mu_result); // trava o mutex do cliente para acessar o resultado
    cl->result.status = r.status;       // define o status e valor de retorno
    strcpy(cl->result.msg, r.msg);

    pthread_cond_signal(&cl->cond_result); // envia o sinal para o cliente de que o resultado foi retornado
    pthread_mutex_unlock(&cl->mu_result);  // libera o mutex
}

// operacao atomica para bloquear o mutex do cliente e marcar o resultado como em espera
void setWaitingResult(cliente *cl) {
    pthread_mutex_lock(&cl->mu_result); // trava o mutex do cliente para realizar as alteracoes
    cl->result.status = waiting;        // marca o resultado como em espera
    strcpy(cl->result.msg, "");
}

// operacao atomica para dormir enquanto espera o sinal de que a operacao foi realizada
resultado getWaitingResult(cliente *cl) {
    // assume-se que o setWaitingResult foi utilizado anteriormente

    resultado tmp_res;
    while (cl->result.status == waiting) // verifica se o resultado ainda esta em espera
    {
        pthread_cond_wait(&cl->cond_result, &cl->mu_result); // dorme enquanto aguarda o resultado
    }

    tmp_res.status = cl->result.status; // recebe o resultado da operacao
    strcpy(tmp_res.msg, cl->result.msg);

    pthread_mutex_unlock(&cl->mu_result); // libera o mutex do cliente

    return tmp_res;
}

#endif