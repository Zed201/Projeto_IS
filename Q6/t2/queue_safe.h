#include <pthread.h>
#include <stdlib.h>

#ifndef QUEUE_H
#define QUEUE_H

#define EXECUTING 0
#define READY 1
#define FINISHED 2
#define BLOCKED 3

typedef struct node node;
typedef struct queue queue;
typedef struct process process;

// tentar ver de colocar status

struct process
{
    char *name;
    void *(*func)(void *);
    pthread_t* exec_thread;
    pthread_cond_t c_state;
    pthread_mutex_t m_state;
    pthread_cond_t* c_escalonador;
    int state;
    int quantum;
};

struct node
{
    process *pc; // process guardada no nó
    node *next;  // ponteiro para a proxima process
};

struct queue
{
    node *head;            // process mais antiga
    node *tail;            // process mais recente
    int lenght;            // quantidade de operacoes na fila
    pthread_mutex_t mutex; // mutex para impedir condicao de corrida
    pthread_cond_t cond;   // condicao para acordar quem estiver esperando a fila possuir requisicoes (banco)
};

#include "cpu_safe.h"

// inicializador do processo
process *process_create(void * (*func)(void *) ,char *nome, int quantum)
{
    struct process *ptr = (struct process *)malloc(sizeof(struct process));
    ptr->name = nome;
    ptr->state = 0;
    ptr->quantum = 10; // inicia com o padrao de 10 e vai aumentando se ele nao terminar
    ptr->func = func;
    ptr->exec_thread = malloc(sizeof(pthread_t));
    ptr->c_escalonador = NULL;

    pthread_cond_init(&ptr->c_state, NULL);
    pthread_mutex_init(&ptr->m_state, NULL);

    return ptr;
}

void process_destroy(process *pc)
{
    pthread_cond_destroy(&pc->c_state);
    pthread_mutex_destroy(&pc->m_state);
    free(pc->name);
    //free(pc->exec_thread);
    //free(pc);
}

// inicializador da fila
queue *queue_create()
{
    queue *new_queue = (queue *)malloc(sizeof(queue));
    new_queue->head = NULL;
    new_queue->tail = NULL;
    new_queue->lenght = 0;

    pthread_mutex_init(&new_queue->mutex, NULL); // inicia os mutex
    pthread_cond_init(&new_queue->cond, NULL);   // inicia as condicoes

    return new_queue;
}

// process atomica para adicionar uma nova process na fila
void request(queue *q, process *pc)
{
    // criacao da requisicao a ser adicionada na queue  
    node *new_node;
    new_node = (node *)malloc(sizeof(node));

    new_node->pc = pc;
    new_node->next = NULL;

    pthread_mutex_lock(&(q->mutex)); // lock necessario a partir do momento que se deseja alterar a queue q
    if (q->tail != NULL)
    {
        q->tail->next = new_node;
    }
    q->tail = new_node;

    if (q->head == NULL)
    {
        q->head = new_node;
    }

    q->lenght++;
    pthread_cond_signal(&q->cond);     // acorda o quem estiver dormindo na condicao (geramente o banco)
    pthread_mutex_unlock(&(q->mutex)); // libera o mutex
}

// process atomica para receber uma process da fila
process *next_wait(queue *q)
{
    process *value;                         // process que ira ser obtida da queue

    pthread_mutex_lock(&(q->mutex));        // trava o mutex da queue se estiver livre
    while (q->lenght == 0)
    {
        pthread_cond_wait(&q->cond, &q->mutex); // dorme se a queue estiver vazia
    }

    node *tmp_node = q->head;               // obtem o primeiro item da queue

    value = tmp_node->pc;

    if (q->head == q->tail)
    {
        q->tail = NULL;
    }

    q->lenght--;
    q->head = q->head->next;
    free(tmp_node);

    pthread_mutex_unlock(&(q->mutex));      // libera o mutex da queue

    return value;
}

void clean_queue(queue *q)
{
    while (q->lenght > 0)
    {
        process *pc = next_wait(q);
        process_destroy(pc);
    }
    free(q);
}

#endif