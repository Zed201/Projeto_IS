#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef QUEUE_H
#define QUEUE_H

typedef struct node node;
typedef struct queue fila;
typedef struct process process;

// struct do processo
struct process {
        char *name;
        pthread_cond_t con;
        pthread_mutex_t m_exec, m_end;
        pthread_t id;
        int flag_exec, flag_end;
        int exec_qtd;
};

// basicamente cria um processo com o nome e com a "quantidade de execucoes" determinada
process* process_create(char *nome, int qtd) {
        process *ptr = (process*) malloc(sizeof(process));
        ptr->name = nome;
        ptr->flag_end = 0;
        ptr->exec_qtd = qtd; // tamanho do loop fixo para todos basicamente

        pthread_cond_init(&ptr->con, NULL);
        pthread_mutex_init(&ptr->m_exec, NULL);
        pthread_mutex_init(&ptr->m_end, NULL);

        return ptr;
}

struct node
{
        process *data; // thread guardada no nó
        node *next;   // ponteiro para o proxima node
};

struct queue
{
        node *head;            // node mais antiga
        node *tail;            // node mais recente
        int lenght;            // quantidade de nodes na fila
        pthread_mutex_t mutex; // mutex para impedir condicao de corrida
        pthread_cond_t cond;   // condicao para acordar quem estiver esperando
};

// inicializador da fila
void initQueue(fila **q)
{
        fila *new_queue = (fila *)malloc(sizeof(fila));
        new_queue->head = NULL;
        new_queue->tail = NULL;
        new_queue->lenght = 0;

        pthread_mutex_init(&new_queue->mutex, NULL); // inicia os mutex
        pthread_cond_init(&new_queue->cond, NULL);   // inicia as condicoes
        *q = new_queue;
}

// operacao atomica para adicionar uma nova operacao na fila
void push(fila *q, process *dado)
{
        // criacao do node a ser adicionada na queue
        node *new_node;
        new_node = (node *)malloc(sizeof(node));
        new_node->next = NULL;
        new_node->data = dado;

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
        pthread_cond_signal(&q->cond);     // acorda o quem estiver dormindo na condicao (no caso o escalonado)
        pthread_mutex_unlock(&(q->mutex)); // libera o mutex
}

// operacao atomica para retirar um processo da fila
process *pop(fila *q)
{
        process *value; // processo que ira ser obtida da queue

        pthread_mutex_lock(&(q->mutex)); // trava o mutex da queue se estiver livre
        while (q->lenght == 0)
        {
                // se nao tiver nenhum node ele libera e retorna null
                pthread_mutex_unlock(&(q->mutex));
                return NULL;
        }

        // obtem o primeiro item da queue
        value = q->head->data;

        node *tmp_node = q->head;

        if (q->head == q->tail)
        {
                q->tail = NULL;
        }

        q->lenght--;
        q->head = q->head->next;
        free(tmp_node);

        pthread_mutex_unlock(&(q->mutex)); // libera o mutex da queue

        return value;
}

#endif
