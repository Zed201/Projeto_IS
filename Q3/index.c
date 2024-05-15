#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue_safe.h"
#include "result_safe.h"

#define N_CLIENTES 5
#define MAX_OPERACOES 30

enum operat
{
    saque,
    deposito,
    consulta
};

// fila de operacoes requisitadas para o banco -> interface
opQueue *queue;

// interface para comunicação com os clientes
cliente *clientes_com[N_CLIENTES];

// banco
void *banco_th(void *args) {
    int valores[N_CLIENTES]; // array com os valores depositado de cada cliente
    for (int i = 0; i < N_CLIENTES; i++)
    {
        valores[i] = 0;
    }

    for (int i = 0; i < N_CLIENTES * MAX_OPERACOES; i++) {
        operacao next; // variavel onde sera guardada a proxima operacao a ser processada
        resultado res; // variavel onde serea guardada o resutlado da operacao

        next = getOp_wait(queue); // dorme enquanto espera uma operacao da fila - atomico

        // processa a operacao solicitada
        if (next.op == consulta) {
            res.status = ok;
            res.value = valores[next.id];
        }
        else if (next.value < 0) {
            res.status = fail;
            res.value = off;
        }
        else if (next.op == deposito) {
            valores[next.id] += next.value;

            res.status = ok;
            res.value = off;
        }
        else if (next.op == saque) {
            if (valores[next.id] - next.value >= 0) {
                valores[next.id] -= next.value;

                res.status = ok;
                res.value = off;
            }
            else {
                res.status = fail;
                res.value = off;
            }
        }

        sendResult(clientes_com[next.id], res); // envia o resultado para o cliente - atomico
    }

    pthread_exit(NULL);
}

// cliente
void *cliente_th(void *args) {
    int *myID = (int *)args;

    for (int i = 0; i < MAX_OPERACOES; i++) {
        operacao myOperation;      // variavel onde sera guardada a operacao a ser realizada
        resultado res;             // variavel onde sera guardado o resultado da operacao
        myOperation.id = *myID;    // id do cliente
        myOperation.op = i % 3;    // operacaoes a serem realizadas
        myOperation.value = i * 3; //

        // trava o mutex do cliente, isso evita o caso em que a operacao é enviada e o
        // banco processa e retorna o resultado antes do waiting ser colocado no resultado
        // o waiting é necessário para distinguir operacoes passadas de recetes
        setWaitingResult(clientes_com[*myID]);
        sendOp(queue, myOperation); // envia a operacao para o banco
        res = getWaitingResult(
            clientes_com[*myID]); // dorme enquanto espera o resultado e libera o mutex do cliente ao fim

        // processa o resultado obtido
        if (myOperation.op != consulta) {
            if (res.status == ok)
            {
                printf("Cliente %d: Operação realizada com sucesso!\n", *myID + 1);
            }
            else if (res.status == fail) // caso de tentar sacar sem saldo suficiente
            {
                printf("Cliente %d: Algo incorreto foi solicitado\n", *myID + 1);
            }
            else
            {
                printf("Cliente %d: Problema\n", *myID + 1);
            }
        }
        else {
            printf("Cliente %d: Valor guardado: %d\n", *myID + 1, res.value); // consulta de saldo
        }
    }

    free(myID);
    pthread_exit(NULL);
}

int main()
{
    initQueue(&queue);
    for (int i = 0; i < N_CLIENTES; i++) {
        initCliente(&clientes_com[i], i);
    }

    pthread_t banco;
    pthread_t clientes[N_CLIENTES];

    pthread_create(&banco, NULL, &banco_th, NULL);
    for (int i = 0; i < N_CLIENTES; i++) {
        int *id = (int *)malloc(sizeof(int));
        *id = i;
        pthread_create(&(clientes[i]), NULL, &cliente_th, (void *)id);
    }

    // espera todas as threads finalizarem
    pthread_join(banco, NULL);
    for (int i = 0; i < N_CLIENTES; i++) {
        pthread_join(clientes[i], NULL);
    }
}