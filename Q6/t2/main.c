#include "queue_safe.h"
#include "cpu_safe.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define N 15 // number of CPUs
#define MAX_PROCESS_REQUEST 500

queue* lista_pronto;
cpu_container* cpus;
int ended = 1;

void* escalonador(void* args) {
    int runned_processes = 0;
    while (runned_processes < MAX_PROCESS_REQUEST) {
        //printf("Runned processes: %d\n", runned_processes);
        process * next = next_wait(lista_pronto);
        if (next == NULL) {
            continue;
        }

        is_full_wait(cpus);
        
        int last_available = 0;
        for (int i = 0; i < cpus->total_cpus; i++) {
            if (check_cpu_available(cpus, i)) {
                last_available = i;
            }
        }

        run_process(cpus, next, last_available);
        runned_processes++;

    }
    ended = 0;
}

void *generic_func(void *args){
    // a propria thread tem acesso à sua estrutura de processo
    process* p_data = (process* ) args;
    //printf("Processo de nome %s iniciado\n", p_data->name);

    pthread_mutex_lock(&p_data->m_state);
    while (p_data->state == BLOCKED) {
        pthread_cond_wait(&p_data->c_state, &p_data->m_state);
    }
    pthread_mutex_unlock(&p_data->m_state);

    clock_t start_time = clock();
    clock_t end_time = clock();
    clock_t run_time = (double)(end_time - start_time) * 1000.0 / CLOCKS_PER_SEC;
    
    int value = 0;
    while (run_time <= p_data->quantum){
        value += (rand() % 5) * (-1 ? (rand() % 2) == 0 : 1);          // simulating any computation

        end_time = clock();
        run_time = (double)(end_time - start_time) * 1000.0 / CLOCKS_PER_SEC;
    }

    //printf("Processo de nome %s finalizado, valor final: %d\n", p_data->name, value);

    pthread_mutex_lock(&p_data->m_state);
    p_data->state = FINISHED;
    pthread_cond_signal(p_data->c_escalonador);
    //printf("Exiting process %s\n", p_data->name);
    pthread_mutex_unlock(&p_data->m_state);

    pthread_exit(NULL);
}

void *user(void* args){
    for (int i = 0; i < MAX_PROCESS_REQUEST; i++) {
        float sleep_between_req = (float) (rand() % 5) * 100.0;    // 500-100ms
        usleep(sleep_between_req);
        
        char* name = (char*)malloc(sizeof(char) * 50);
        sprintf(name, "process_%d", i);
        process* new_process = process_create(&generic_func, name, 100);
        
        request(lista_pronto, new_process);    //renomear para request
    }

    pthread_exit(NULL);
}

void debugger() {
    while (ended) {
        printf("----------\n");

        usleep(1000);
        for (int i = 0; cpus && i < cpus->total_cpus; i++) {
            char name[100];
            get_process_name(cpus, i, name);
            printf("CPU %d: %s\n", i, name);
        }
    }
}

int main() {
    lista_pronto = queue_create();
    cpus = cpu_container_create(N);

    clock_t start_time = clock();

    pthread_t escalonador_t;
    pthread_t user_t;
    pthread_t debugger_t;
    printf("Rodando\n");
    pthread_create(&escalonador_t, NULL, &escalonador, NULL);
    pthread_create(&user_t, NULL, &user, NULL);
    pthread_create(&debugger_t, NULL, &debugger, NULL);

    pthread_join(escalonador_t, NULL);
    pthread_join(user_t, NULL);
    pthread_join(debugger_t, NULL);
    
    clock_t end_time = clock();
    double run_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Total run time: %lf\n", run_time);
}