#include <pthread.h>
#include <stdlib.h>

#ifndef QUEUE_H
#define QUEUE_H

typedef struct operacao operacao;
typedef struct node node;
typedef struct queue opQueue;

// struct que representa um solicitação
struct operacao {
    int id;    // id do cliente
    int op;    // operacao que sera realizada
    int value; // valor caso seja deposito
};

struct node {
    operacao op; // operacao guardada no nó
    node *next;  // ponteiro para a proxima operacao
};

struct queue {
    node *head;            // operacao mais antiga
    node *tail;            // operacao mais recente
    int lenght;            // quantidade de operacoes na fila
    pthread_mutex_t mutex; // mutex para impedir condicao de corrida
    pthread_cond_t cond;   // condicao para acordar quem estiver esperando a fila possuir requisicoes
};

// inicializador da fila
void initQueue(opQueue **q) {
    opQueue *new_queue = (opQueue *)malloc(sizeof(opQueue));
    new_queue->head = NULL;
    new_queue->tail = NULL;
    new_queue->lenght = 0;

    pthread_mutex_init(&new_queue->mutex, NULL); // inicia os mutex
    pthread_cond_init(&new_queue->cond, NULL);   // inicia as condicoes
    *q = new_queue;
}

// operacao atomica para adicionar uma nova operacao na fila
void sendOp(opQueue *q, operacao op) {
    // criacao da requisicao a ser adicionada na queue
    node *new_node;
    new_node = (node *)malloc(sizeof(node));
    new_node->next = NULL;
    new_node->op.id = op.id;
    new_node->op.op = op.op;
    new_node->op.value = op.value;

    pthread_mutex_lock(&(q->mutex)); // lock necessario a partir do momento que se deseja alterar a queue q
    if (q->tail != NULL) {
        q->tail->next = new_node;
    }
    q->tail = new_node;

    if (q->head == NULL) {
        q->head = new_node;
    }

    q->lenght++;
    pthread_cond_signal(&q->cond);     // acorda o quem estiver dormindo na condicao (geramente o banco)
    pthread_mutex_unlock(&(q->mutex)); // libera o mutex
}

// operacao atomica para receber uma operacao da fila
operacao getOp_wait(opQueue *q) {
    operacao ret; // operacao que ira ser obtida da queue

    pthread_mutex_lock(&(q->mutex)); // trava o mutex da queue se estiver livre
    while (q->lenght == 0) {
        pthread_cond_wait(&q->cond, &q->mutex); // dorme se a queue estiver vazia
    }

    // obtem o primeiro item da queue
    ret.id = q->head->op.id;
    ret.op = q->head->op.op;
    ret.value = q->head->op.value;
    node *tmp_node = q->head;

    if (q->head == q->tail) {
        q->tail = NULL;
    }

    q->lenght--;
    q->head = q->head->next;
    free(tmp_node);

    pthread_mutex_unlock(&(q->mutex)); // libera o mutex da queue

    return ret;
}

#endif