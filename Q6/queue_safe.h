#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#ifndef QUEUE_H
#define QUEUE_H

typedef struct node node;
typedef struct queue opQueue;
typedef struct pro pro;
// tentar ver de colocar status
enum status
{
    ready,
    executing,
    blocked
};

struct pro{
        char *Nome_Processo;
        pthread_cond_t con;
        pthread_mutex_t m1, m2;
        int flag_exe, flag_end, _status;
        // colocar o pthread_t aqui tbm
};

// TODO: Testar para ver se essa funcao funciona
pro* process_create(char *nome){
        struct pro *ptr = (struct pro*) malloc(sizeof(struct pro));
        ptr->Nome_Processo = nome;
        // ver onde coloca o destroy, talvez na hora da struct tiver finalizado a função
        pthread_cond_init(&ptr->con, NULL);
        pthread_mutex_init(&ptr->m1, NULL);
        pthread_mutex_init(&ptr->m2, NULL);
        return ptr;
}

struct node
{
    pro *data; // operacao guardada no nó
    node *next;   // ponteiro para a proxima operacao
};

struct queue
{
    node *head;            // operacao mais antiga
    node *tail;            // operacao mais recente
    int lenght;            // quantidade de operacoes na fila
    pthread_mutex_t mutex; // mutex para impedir condicao de corrida
    pthread_cond_t cond;   // condicao para acordar quem estiver esperando a fila possuir requisicoes (banco)
};

// inicializador da fila
void initQueue(opQueue **q)
{
    opQueue *new_queue = (opQueue *)malloc(sizeof(opQueue));
    new_queue->head = NULL;
    new_queue->tail = NULL;
    new_queue->lenght = 0;

    pthread_mutex_init(&new_queue->mutex, NULL); // inicia os mutex
    pthread_cond_init(&new_queue->cond, NULL);   // inicia as condicoes
    *q = new_queue;
}

// operacao atomica para adicionar uma nova operacao na fila
void push(opQueue *q, pro *dado)
{
    // criacao da requisicao a ser adicionada na queue
    node *new_node;
    new_node = (node *)malloc(sizeof(node));
    new_node->next = NULL;
    new_node->data = dado;

    // new_node->data = (pro *)malloc(sizeof(pro));
    
    // strcpy(new_node->data->name, name);
    // new_node->proc->status = ready;
    // pthread_cond_init(&new_node->proc->cond, NULL);

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

// operacao atomica para receber uma operacao da fila
thread *pop(opQueue *q)
{
    thread *ret; // operacao que ira ser obtida da queue

    pthread_mutex_lock(&(q->mutex)); // trava o mutex da queue se estiver livre
    while (q->lenght == 0)
    {
        pthread_cond_wait(&q->cond, &q->mutex); // dorme se a queue estiver vazia
    }

    // obtem o primeiro item da queue
    ret = q->head;

    node *tmp_node = q->head;

    if (q->head == q->tail)
    {
        q->tail = NULL;
    }

    q->lenght--;
    q->head = q->head->next;
    free(tmp_node);

    pthread_mutex_unlock(&(q->mutex)); // libera o mutex da queue

    return ret;
}

#endif
