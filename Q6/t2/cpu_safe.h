#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#ifndef CPU_SAFE_H
#define CPU_SAFE_H

typedef struct cpu cpu;
typedef struct cpu_container cpu_container;

/*
    Estrutura que armazena os processos que estão sendo executados em
    cada CPU, utilizando um ponteiro para o processo que será executada
    e um ponteiro para os dados do processo.

    Não é necessário utilizar mutex para proteger cada CPU ou o CPU container
    pois existe um único escalonador que irá alocar os processos.

    A estrutura também possui um mutex e uma condição para sinalizar quando
    um processo terminou de ser executado.
*/


typedef struct cpu
{
    process *process;
};

struct cpu_container
{
    cpu *cpus;
    int used_cpus;
    int total_cpus;
    int finished_index;
    pthread_mutex_t m_finished;
    pthread_cond_t c_finished;
};

#include "queue_safe.h"

cpu_container *cpu_container_create(int total_cpus)
{
    cpu_container *container = (cpu_container *)malloc(sizeof(cpu_container));
    container->cpus = (cpu *)malloc(sizeof(cpu) * total_cpus);
    container->used_cpus = 0;
    container->total_cpus = total_cpus;
    pthread_mutex_init(&container->m_finished, NULL);
    pthread_cond_init(&container->c_finished, NULL);
    return container;
}

void get_process_name(cpu_container *container, int index, char *name) {
    pthread_mutex_lock(&container->m_finished);
    if (container->cpus[index].process == NULL) {
        strcpy(name, "NULL");
        pthread_mutex_unlock(&container->m_finished);
        return;
    }

    pthread_mutex_lock(&container->cpus[index].process->m_state);

    if (container->cpus[index].process && container->cpus[index].process->state == FINISHED) {
        strcpy(name, "NULL");
    }
    else if (container->cpus[index].process) {
        strcpy(name, container->cpus[index].process->name);
    }

    pthread_mutex_unlock(&container->cpus[index].process->m_state);
    pthread_mutex_unlock(&container->m_finished);
}

int check_cpu_available(cpu_container *container, int index)
{
    pthread_mutex_lock(&container->m_finished);
    // verifica se a posição não tem nenhum processo alocado
    if (container->cpus[index].process == NULL)
    {
        pthread_mutex_unlock(&container->m_finished);
        return 1;
    }

    // verifica se o processo alocado já terminou
    pthread_mutex_lock(&container->cpus[index].process->m_state);
    if (container->cpus[index].process->state == FINISHED)
    {
        pthread_mutex_unlock(&container->cpus[index].process->m_state);
        pthread_mutex_unlock(&container->m_finished);
        process_destroy(container->cpus[index].process);
        container->cpus[index].process = NULL;
        container->used_cpus--;

        return 1;
    }

    pthread_mutex_unlock(&container->cpus[index].process->m_state);
    pthread_mutex_unlock(&container->m_finished);
    return 0;
}

void run_process(cpu_container *container, process *pc, int index)
{
    if (check_cpu_available(container, index) == 0)
    {
        return;
    }

    container->cpus[index].process = pc;
    container->cpus[index].process->c_escalonador = &container->c_finished;
    container->cpus[index].process->state = EXECUTING;

    container->used_cpus++;
    pthread_create(container->cpus[index].process->exec_thread, NULL, pc->func, pc);
}

void is_full_wait(cpu_container *container)
{
    if (container->used_cpus == container->total_cpus)
    {
        pthread_mutex_lock(&container->m_finished);
        pthread_cond_wait(&container->c_finished, &container->m_finished);
        pthread_mutex_unlock(&container->m_finished);
    }
}

#endif