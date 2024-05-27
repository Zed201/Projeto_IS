#include "queue_safe.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

#define time_testS 1 // tempo de espera para nao ficar bagunçado no print 
#define N 3 // se o N nao for compativel com a quantidade de processos, ele trava pois a propria fila trava
#define quatum_ms 1 // quantum fixo, em testes ele difrete de um tempo bom para cada pc
// com numeros pequenos de processamento ele simplesmente executa e para
fila* lista_pronto;

void *escalonador_func(void* args){
        process** temp_pro = (process**) malloc(sizeof(process*) * N);
        int q = quatum_ms, t = time_testS;
        // ver alguma forma de para quando nao tiver mais processos na fila
        while(1){
                if (lista_pronto->lenght == 0) {
                        printf("Fila Vazio\n");
                        pthread_mutex_lock(&lista_pronto->mutex);
                        pthread_cond_wait(&lista_pronto->cond, &lista_pronto->mutex); // dorme se a queue estiver vazia
                        pthread_mutex_unlock(&lista_pronto->mutex);
                }

                for (int i = 0; i < N; i++) {        
                        // fazer alguma verificação de quantidade de != NULL

                        temp_pro[i] = pop(lista_pronto); // se a lista
                        if(temp_pro[i] != NULL){ 
                                // mesmo quantum para todos
                                printf("Executando processo %s por %d ms\n", temp_pro[i]->name, q);

                        }
                }

                // printf("Espera %ds\n", t); // novamente para nao ficar muito bagunçado o print
                // sleep(t);
                printf("------------------------------------\n");
                for(int i = 0; i < N; i++){
                        if(temp_pro[i] != NULL){
                                pthread_mutex_lock(&temp_pro[i]->m_exec);
                                temp_pro[i]->flag_exec = 0;
                                pthread_mutex_unlock(&temp_pro[i]->m_exec);

                                pthread_cond_signal(&temp_pro[i]->con);
                        }
                }

                usleep(1000 * quatum_ms);

                for(int i = 0; i < N; i++){
                        if(temp_pro[i] != NULL){

                printf("Iteração escalonador\n");
                                pthread_mutex_lock(&temp_pro[i]->m_exec);
                                temp_pro[i]->flag_exec = 1;
                                pthread_mutex_unlock(&temp_pro[i]->m_exec);               

                                pthread_mutex_lock(&temp_pro[i]->m_end);
                                if(temp_pro[i]->flag_end){
                                        printf("Processo %s finalizado\n", temp_pro[i]->name);
                                        // liberar memoira
                                        pthread_mutex_destroy(&temp_pro[i]->m_end);
                                        pthread_mutex_destroy(&temp_pro[i]->m_exec);
                                        pthread_cond_destroy(&temp_pro[i]->con);
                                        free(temp_pro[i]);
                                } 
                                else {
                                        printf("Processo %s não terminou no tempo do quantum e voltara para o final da fila\n", temp_pro[i]->name);
                                        push(lista_pronto, temp_pro[i]);
                                }

                                pthread_mutex_unlock(&temp_pro[i]->m_end);               
                        }
                }
        }
        pthread_exit(NULL);
}

void *user(void* agrs){
        pthread_exit(NULL);
}

void *generic_func(void *args){
        // a propria thread tem acesso à sua estrutura de processo
        process* p_data = (process* ) args;
        printf("Processo de nome %s iniciado\n", p_data->name);
        pthread_mutex_lock(&p_data->m_end);
        pthread_cond_wait(&p_data->con, &p_data->m_end);
        pthread_mutex_unlock(&p_data->m_end);
        for (int i = 0; i < p_data->exec_qtd; i++) {

                // pthread_mutex_lock(&p_data->m_end);
                if(p_data->flag_exec){
                        // trocar para exec os mutex
                        // pthread_cond_wait(&p_data->con, &p_data->m_end);
                        pthread_mutex_lock(&p_data->m_exec);
                        pthread_cond_wait(&p_data->con, &p_data->m_exec);
                        pthread_mutex_unlock(&p_data->m_exec);
                }
                // pthread_mutex_unlock(&p_data->m_end);

                printf("%s-%d\n", p_data->name, i);
                fflush(stdout);
        }
        // pthread_mutex_unlock(&p_data->m_end); // alguns casos ele sai do for, se o p_data->exec_qtd for pequeno, com esse mutex locked e trava tudo
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
        printf("Escalonador iniciado, vai começar em %ds\n", wait_s);
        sleep(wait_s);
        pthread_join(escalonador, NULL); // apenas espera o escalonador terminar

        return 0;
}
