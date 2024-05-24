#include "queue_safe.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

// a principio 1 para testar
#define N 1
#define quatum_ms 10 // quantum fixo, talvez mudar para um diferente para cada processo
int flag_quantum = 1; // se 0 ele executa com o mesmo quantum de todos se 1 ele vai aumentando o quantum por processo
fila* lista_pronto;

void *escalonador_func(void* args){
        pro* temp_pro;
        int q = quatum_ms;
        // ver alguma forma de para quando nao tiver mais processos na fila
        while(1){
                temp_pro = pop(lista_pronto); // se a lista
                if (flag_quantum) {

                        printf("Executando processo %s por %d ms\n", temp_pro->Nome_Processo, temp_pro->quantum);
                } else {
                        printf("Executando processo %s por %d ms\n", temp_pro->Nome_Processo, q);
                }
                pthread_mutex_lock(&temp_pro->m2);
                temp_pro->flag_exe = 0;
                pthread_mutex_unlock(&temp_pro->m2);

                pthread_cond_signal(&temp_pro->con);
                if (!flag_quantum) {
                        usleep(1000 * quatum_ms);
                } else {
                        usleep(1000 * temp_pro->quantum);
                }
                pthread_mutex_lock(&temp_pro->m2);
                temp_pro->flag_exe = 1;
                pthread_mutex_unlock(&temp_pro->m2);               

                pthread_mutex_lock(&temp_pro->m1);
                if(temp_pro->flag_end){
                        printf("Processo %s finalizado\n", temp_pro->Nome_Processo);
                } else {
                        push(lista_pronto, temp_pro);
                        printf("Processo não terminou no tempo do quantum e voltara para o final da fila\nEsperando 2s\n");
                        if (flag_quantum) {
                                printf("Quantum aumentado\n");
                                temp_pro->quantum *= 2;
                        }
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
        printf("Processo de nome %s iniciado\n", p_data->Nome_Processo);
        pthread_cond_wait(&p_data->con, &p_data->m1);
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
        // testes
        pthread_t t[3];
        char* nomes[3] = {"T1", "T2", "T3"};
        pro* temp;
        for (int i = 0; i < 3; i++) {
                temp = process_create(nomes[i]);
                pthread_create(&temp->id, NULL, generic_func, temp);
                push(lista_pronto, temp);
        }


        pthread_t escalonador;
        pthread_create(&escalonador, NULL, escalonador_func, NULL);
        int wait_s = 2;
        printf("Escalonador iniciado, vai começar em %ds\n", wait_s);
        sleep(wait_s);
        pthread_join(escalonador, NULL); // apenas espera o escalonador terminar

        return 0;
}
