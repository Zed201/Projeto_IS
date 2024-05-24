#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue_safe.h"
#include "result_safe.h"

#define N_CLIENTES 5
#define MAX_OPERACOES 30

enum operat {
    saque,
    deposito,
    consulta
};

typedef struct bancoInfo {
    int numClientes;
    int numContas;
} bancoInfo;

typedef struct clientInfo {
    int myId;
    int accountId;
} clientInfo;

// fila de operacoes requisitadas para o banco -> interface
opQueue *queue;

// interface para comunicação com os clientes
cliente *clientes_com[N_CLIENTES];

// banco
void *banco_th(void *args) {
    bancoInfo * info = (bancoInfo *)args;
    
    float valores[info->numContas]; // array com os valores depositado de cada cliente
    for (int i = 0; i < info->numContas; i++)
    {
        valores[i] = 0.0f;
    }

    for (int i = 0; i < info->numClientes * MAX_OPERACOES; i++) {
        operacao next; // guarda a proxima operacao a ser processada
        resultado res; // guarda o resultado da operacao
        strcpy(res.msg, "");

        next = getOp_wait(queue); // dorme enquanto espera uma operacao da fila

        // processa a operacao solicitada
        if (next.op == consulta) {
            res.status = ok;
            sprintf(res.msg, "Cliente %d (%d): Seu saldo é R$%.2f.\n", next.accountId, next.clientId, valores[next.accountId]);
        }
        else if (next.value < 0) {
            res.status = fail;
            sprintf(res.msg, "Cliente %d (%d): Saque ou deposito negativos são invalidos.\n", next.accountId, next.clientId);
        }
        else if (next.op == deposito) {
            valores[next.accountId] += next.value;

            res.status = ok;
            sprintf(res.msg, "Cliente %d (%d): Deposito de R$%.2f realizado com sucesso, seu novo saldo é R$%.2f.\n", 
                                next.accountId, next.clientId, next.value, valores[next.accountId]);
        }
        else if (next.op == saque) {
            if (valores[next.accountId] - next.value >= 0) {
                valores[next.accountId] -= next.value;

                res.status = ok;
                sprintf(res.msg, "Cliente %d (%d): Saque de R$%.2f realizado com sucesso, seu novo saldo é R$%.2f.\n", 
                                next.accountId, next.clientId, next.value, valores[next.accountId]);
            }
            else {
                res.status = fail;
                sprintf(res.msg, "Cliente %d (%d): Saque de R$%.2f invalido, seu saldo atual é R$%.2f.\n", 
                                next.accountId, next.clientId, next.value, valores[next.accountId]);
            }
        }

        sendResult(clientes_com[next.clientId], res); // envia o resultado para o cliente
    }

    pthread_exit(NULL);
}

// cliente
void *cliente_th(void *args) {
    clientInfo *myInfo = (clientInfo *)args;

    for (int i = 0; i < MAX_OPERACOES; i++) {
        operacao myOperation;                       // variavel onde sera guardada a operacao a ser realizada
        resultado res;                              // variavel onde sera guardado o resultado da operacao
        myOperation.clientId = myInfo->myId;        // id do cliente
        myOperation.accountId = myInfo->accountId;  // id da conta
        myOperation.op = i % 3;                     // operacaoes a serem realizadas
        myOperation.value = i * 3;                  //

        // trava o mutex do cliente, isso evita o caso em que a operacao é enviada e o
        // banco processa e retorna antes do resultado ser colocado como waiting
        // o waiting é necessário para distinguir operacoes passadas de recentes
        setWaitingResult(clientes_com[myInfo->myId]);
        sendOp(queue, myOperation); // envia a operacao para o banco
        res = getWaitingResult(
            clientes_com[myInfo->myId]); // dorme enquanto espera o resultado e libera o mutex do cliente ao fim

        // print do resultado obtido
        printf("%s", res.msg);
    }

    free(myInfo);
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

    // instancia o banco
    bancoInfo *b_info = (bancoInfo *)malloc(sizeof(bancoInfo));
    b_info->numClientes = 5;
    b_info->numContas = 5;

    pthread_create(&banco, NULL, &banco_th, (void *)b_info);

    // instancia os clientes
    for (int i = 0; i < N_CLIENTES; i++) {
        clientInfo *info = (clientInfo *)malloc(sizeof(clientInfo));
        info->myId = i;
        info->accountId = i;
        pthread_create(&(clientes[i]), NULL, &cliente_th, (void *)info);
    }

    // espera todas as threads finalizarem
    pthread_join(banco, NULL);
    for (int i = 0; i < N_CLIENTES; i++) {
        pthread_join(clientes[i], NULL);
    }
}