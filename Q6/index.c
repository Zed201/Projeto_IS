#include "queue_safe.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

// a principio 1 para testar
#define N 1
#define quatum_ms 1 // quantum fixo, talvez mudar para um diferente para cada processo
fila* lista_pronto;

void *escalonador_func(void* args){
        pro* temp_pro;
        int q = quatum_ms;
        // ver alguma forma de para quando nao tiver mais processos na fila
       while(1){
                temp_pro = pop(lista_pronto); // se a lista
                printf("Executando processo %s por %d ms", temp_pro->Nome_Processo, q);
                pthread_mutex_lock(&temp_pro->m1);
                temp_pro->exec_qtd = 0;
                pthread_mutex_unlock(&temp_pro->m1);

                pthread_cond_signal(&temp_pro->con);
                usleep(1000 * quatum_ms);

                pthread_mutex_lock(&temp_pro->m1);
                temp_pro->exec_qtd = 1;
                pthread_mutex_unlock(&temp_pro->m1);               
                
                pthread_mutex_lock(&temp_pro->m1);
                if(temp_pro->flag_end){
                        printf("Processo %s finalizado\n", temp_pro->Nome_Processo);
                } else {
                       push(lista_pronto, temp_pro);
                        printf("Processo não terminou no tempo do quantum\n Esperando 2s\n");
                        sleep(2);
                }
                        pthread_mutex_unlock(&temp_pro->m1);               
        }
        pthread_exit(NULL);
}

void *user(void* agrs){
        pthread_exit(NULL);
}

void *generic_func(void *args){
        // a propria thread tem acesso à sua estrutura de processo
        pro* p_data = (pro* ) args;
        pthread_cond_wait(&p_data->con, &p_data->m1);
        printf("Processo de nome %s iniciado\n", p_data->Nome_Processo);
        for (int i = 0; i < p_data->exec_qtd; i++) {
                if(p_data->flag_exe){
                        pthread_cond_wait(&p_data->con, &p_data->m1);
                        pthread_mutex_unlock(&p_data->m1);
                }

                printf("%s-%d\n", p_data->Nome_Processo, i);
                fflush(stdout);
        }

        pthread_mutex_lock(&p_data->m1);
        p_data->flag_end = 0;
        pthread_mutex_unlock(&p_data->m1);
        pthread_exit(NULL);

}

int main(){
        initQueue(&lista_pronto);
        pthread_t escalonador;
        pthread_create(&escalonador, NULL, escalonador_func, NULL);
        int wait_s = 2;
        printf("Escalonador iniciado, vai começar em %ds", wait_s);
        sleep(wait_s);
        pthread_join(escalonador, NULL); // apenas espera o escalonador terminar

        return 0;
}
