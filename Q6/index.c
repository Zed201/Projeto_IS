#include "queue_safe.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

#define N 10 // Representa o numero de "processos" executados ao "mesmo tempo", no caso o maximo que ele tira do escalonador
#define quatum_ms 1 // quantum fixo, ba, basicamente o tempo que elas vao ficar executando em mss

fila* lista_pronto;

void *escalonador_func(void* args){
        process** temp_pro = (process**) malloc(sizeof(process*) * N);
        // quantum colocado no define apenas para ele nao ficar dando erro de nao ser int
        int q = quatum_ms;
        while(1){
                if (lista_pronto->lenght == 0) { // fila vazia ele fica esperando algum processo entrar 
                        printf("Fila Vazio\n");
                        fflush(stdout); // para o print nao ficar acumulando buffer
                        pthread_mutex_lock(&lista_pronto->mutex);
                        pthread_cond_wait(&lista_pronto->cond, &lista_pronto->mutex); // dorme se a queue estiver vazia
                        pthread_mutex_unlock(&lista_pronto->mutex);
                }
                int nNULL = 0; // quantidade de processos dentro de temp_pro nao nulos(evitar iteraçoes de for desnecessarias)
                for (int i = 0; i < N; i++) {        
                        temp_pro[i] = pop(lista_pronto); // tira da a lista
                        if(temp_pro[i] != NULL){ 
                                printf("Executando processo %s por %d ms\n", temp_pro[i]->name, q); // para saber quais processo vao executar
                                fflush(stdout);
                                nNULL++;
                        }
                }

                printf("------------------------------------\n"); // para nao ficar muito bagunão o print
                fflush(stdout);
                for(int i = 0; i < nNULL; i++){
                        // modifica a flag para indicar que o processo vai "executar" e da o signal
                        pthread_mutex_lock(&temp_pro[i]->m_exec);
                        temp_pro[i]->flag_exec = 0;
                        pthread_mutex_unlock(&temp_pro[i]->m_exec);

                        pthread_cond_signal(&temp_pro[i]->con);
                }
                // tempo que os processos vao ficar em execucao
                usleep(1000 * quatum_ms);

                for(int i = 0; i < nNULL; i++){
                        // muda a flag para o processo entrar no if e dormir
                        pthread_mutex_lock(&temp_pro[i]->m_exec);
                        temp_pro[i]->flag_exec = 1;
                        pthread_mutex_unlock(&temp_pro[i]->m_exec);               
                        // verifica se o processo terminou ou nao, ai volta para o final da fila
                        pthread_mutex_lock(&temp_pro[i]->m_end);
                        if(temp_pro[i]->flag_end){
                                printf("Processo %s finalizado\n", temp_pro[i]->name);
                                fflush(stdout);
                                // liberar memoira
                                pthread_mutex_destroy(&temp_pro[i]->m_end);
                                pthread_mutex_destroy(&temp_pro[i]->m_exec);
                                pthread_cond_destroy(&temp_pro[i]->con);
                                free(temp_pro[i]);
                        } 
                        else {
                                printf("Processo %s nao terminou\n", temp_pro[i]->name);
                                fflush(stdout);
                                push(lista_pronto, temp_pro[i]);
                        }

                        pthread_mutex_unlock(&temp_pro[i]->m_end);               
                }
        }
        pthread_exit(NULL);
}

// usado para ficar inserindo os processos
void *user(void* agrs){
        pthread_exit(NULL);
}

// funcao generica usada para ficar em loop 
void *generic_func(void *args){
        // a propria thread tem acesso à sua estrutura de processo
        process* p_data = (process* ) args;
        printf("Processo de nome %s iniciado\n", p_data->name);
        pthread_mutex_lock(&p_data->m_end);
        pthread_cond_wait(&p_data->con, &p_data->m_end); // logo quando a thread e criada ela para para ser liberada apenas
        //  quando o escalonador liberar ela
        pthread_mutex_unlock(&p_data->m_end);

        for (int i = 0; i < p_data->exec_qtd; i++) {
                if(p_data->flag_exec){
                        // serve para travar a execucao do processo
                        pthread_mutex_lock(&p_data->m_exec);
                        pthread_cond_wait(&p_data->con, &p_data->m_exec);
                        pthread_mutex_unlock(&p_data->m_exec);
                }

                printf("%s-%d\n", p_data->name, i);
                fflush(stdout);
        }
        // para indicar que o processo terminou para o escalonador
        pthread_mutex_lock(&p_data->m_end);
        p_data->flag_end = 1;
        pthread_mutex_unlock(&p_data->m_end);
        pthread_exit(NULL);

}

#define pT 4
int main(){
        initQueue(&lista_pronto);
        // testes-----------------------
        pthread_t t[pT];
        char* nomes[pT] = {"T1", "T2", "T3", "T4"};
        int pros[pT] = {800, 900, 200, 700};
        process* temp;
        for (int i = 0; i < pT; i++) {
                temp = process_create(nomes[i], pros[i]);
                pthread_create(&temp->id, NULL, generic_func, temp);
                push(lista_pronto, temp);
        }
        //------------------------------

        pthread_t escalonador;
        pthread_create(&escalonador, NULL, escalonador_func, NULL);
        int wait_s = 2;
        printf("Escalonador criado, vai iniciar em %ds\n", wait_s);
        sleep(wait_s);
        pthread_join(escalonador, NULL); // apenas espera o escalonador terminar

        return 0;
}
