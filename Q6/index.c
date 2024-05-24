#include "queue_safe.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
// revisar para verificar se ta sefa
// a principio 1 para testar
// TODO: Mudar o codigo para ele conseguir executar mais de uma
#define time_testS 2
#define N 2 // se o N nao for compativel com a quantidade de processos, ele trava pois a propria fila trava
#define quatum_ms 1 // quantum fixo, em testes ele difrete de um tempo bom para cada pc
// com numeros pequenos de processamento ele simplesmente executa e para
#define processamento 200
int flag_quantum = 0; // se 0 ele executa com o mesmo quantum de todos se 1 ele vai aumentando o quantum por processo
fila* lista_pronto;

void *escalonador_func(void* args){
        pro** temp_pro = (pro**) malloc(sizeof(pro*) * N);
        int q = quatum_ms, t = time_testS;
        // ver alguma forma de para quando nao tiver mais processos na fila
        while(1){
                for (int i = 0; i < N; i++) {        
                        temp_pro[i] = pop(lista_pronto); // se a lista
                        if(flag_quantum){
                                // executar com quantum individual
                                printf("Executando processo %s por %d ms\n", temp_pro[i]->Nome_Processo, temp_pro[i]->quantum);
                        } else {
                                // mesmo quantum para todos
                                printf("Executando processo %s por %d ms\n", temp_pro[i]->Nome_Processo, q);
                        }

                }

                printf("Espera %ds\n", t); // novamente para nao ficar muito bagunçado o print
                sleep(t);
                for(int i = 0; i < N; i++){
                        pthread_mutex_lock(&temp_pro[i]->m2);
                        temp_pro[i]->flag_exe = 0;
                        pthread_mutex_unlock(&temp_pro[i]->m2);

                        pthread_cond_signal(&temp_pro[i]->con);
                }

                if (flag_quantum) {
                        // tirar o quantum variavel, pois com varios fica dificil sincronizar
                        // usleep(1000 * temp_pro->quantum);
                } else {
                        usleep(1000 * quatum_ms);
                }

                for(int i = 0; i < N; i++){
                        pthread_mutex_lock(&temp_pro[i]->m2);
                        temp_pro[i]->flag_exe = 1;
                        pthread_mutex_unlock(&temp_pro[i]->m2);               

                        pthread_mutex_lock(&temp_pro[i]->m1);
                        if(temp_pro[i]->flag_end){
                                printf("Processo %s finalizado\n", temp_pro[i]->Nome_Processo);
                                // liberar memoira
                                pthread_mutex_destroy(&temp_pro[i]->m1);
                                pthread_mutex_destroy(&temp_pro[i]->m2);
                                pthread_cond_destroy(&temp_pro[i]->con);
                                free(temp_pro[i]);
                        } else {
                                push(lista_pronto, temp_pro[i]);
                                // temp_pro[i] = NULL;
                                printf("Processo não terminou no tempo do quantum e voltara para o final da fila\nEsperando 2s\n");
                                // espera para nao ficar muito bagunçado no print
                                // tirar dps
                                // if(flag_quantum){
                                //         // executar com quantum individual
                                //         printf("Quantum aumentado\n");
                                //         temp_pro->quantum *= 2;
                                // } 

                                //sleep(time_testS);
                        }

                        pthread_mutex_unlock(&temp_pro[i]->m1);               
                }
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

        pthread_mutex_unlock(&p_data->m1); // alguns casos ele sai do for, se o p_data->exec_qtd for pequeno, com esse mutex locked e trava tudo
        pthread_mutex_lock(&p_data->m1);
        p_data->flag_end = 1;
        pthread_mutex_unlock(&p_data->m1);
        pthread_exit(NULL);

}

#define pT 4
int main(){
        initQueue(&lista_pronto);
        // testes-----------------------
        pthread_t t[pT];
        char* nomes[pT] = {"T1", "T2", "T3", "T4"};
        pro* temp;
        for (int i = 0; i < pT; i++) {
                temp = process_create(nomes[i], processamento);
                pthread_create(&temp->id, NULL, generic_func, temp);
                push(lista_pronto, temp);
        }
        //------------------------------

        pthread_t escalonador;
        pthread_create(&escalonador, NULL, escalonador_func, NULL);
        int wait_s = 2;
        printf("Escalonador iniciado, vai começar em %ds\n", wait_s);
        sleep(wait_s);
        pthread_join(escalonador, NULL); // apenas espera o escalonador terminar

        return 0;
}
