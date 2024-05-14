#include <stdlib.h>

#ifndef QUEUE_H
#define QUEUE_H

typedef struct operacao operacao;
typedef struct node node;
typedef struct queue opQueue;

struct operacao
{
    int id;
    int op;
    int value;
    int *retorno;
};


struct node
{
    operacao op;
    node *next;
};

struct queue
{
    node *head;
    node *tail;
    int lenght;
};

void initQueue(opQueue **q)
{
    opQueue *new_queue = (opQueue *)malloc(sizeof(opQueue));
    new_queue->head = NULL;
    new_queue->tail = NULL;
    new_queue->lenght = 0;

    *q = new_queue;
}

void enqueue(opQueue *q, operacao op)
{
    node *new_node;
    new_node = (node *)malloc(sizeof(node));
    new_node->next = NULL;
    new_node->op = op;

    q->tail->next = new_node;
    q->tail = new_node;

    if (q->head == NULL)
    {
        q->head = new_node;
    }

    q->lenght++;
}

operacao* dequeue(opQueue *q)
{
    operacao* ret = (operacao *) malloc(sizeof(operacao));
    *ret = q->head->op;
    node *tmp_node = q->head;

    if (q->head == q->tail)
    {
        q->tail = NULL;
    }

    q->lenght--;
    q->head = q->head->next;
    free(tmp_node);

    return ret;
}

int isEmpty(opQueue *q)
{
    return (q->lenght == 0);
}

#endif