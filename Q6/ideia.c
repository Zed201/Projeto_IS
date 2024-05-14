#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// ideia da struct processo

struct pro{
        char *Nome_Processo;
        pthread_cond_t con;
        pthread_mutex_t m1, m2;
        int flag_exe, flag_end;
        // colocar o pthread_t aqui tbm
};

struct pro* process_create(char *nome){
        struct pro *ptr = (struct pro*) malloc(sizeof(struct pro));
        ptr->Nome_Processo = nome;
        // ver onde coloca o destroy, talvez na hora da struct tiver finalizado a função
        pthread_cond_init(&ptr->con, NULL);
        pthread_mutex_init(&ptr->m1, NULL);
        pthread_mutex_init(&ptr->m2, NULL);
        return ptr;
}

int flag = 0;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;
int finalizer = 0;

void *func(void *agrs){
        pthread_cond_wait(&c, &m);
        for (int i = 0; i < 1000; i++) {
                // sem esses mutex ele vai, por algum motivo, era para funcionar, mas n sei pq
                //pthread_mutex_lock(&m2);
                if(flag){ // flag 1 ele para, zero ele continua
                        printf("Thread Vai entrar em espera\n");
                        pthread_cond_wait(&c, &m);
                        printf("Thread saindo de hibernacao\n");
                }
                //pthread_mutex_unlock(&m2);

                // alguma execuçao
                printf("Alguma coisa %d\n", i);
        }

        pthread_mutex_unlock(&m); // tem de dar unlock pois o m ta trancado
        printf("Saiu loop\n");
        pthread_mutex_lock(&m);
        finalizer = 1;
        pthread_mutex_unlock(&m);
        pthread_exit(NULL);
}



int main(){
        pthread_t ids;
        // TODO:
        // testar se a inicialização dinamica do cond e do mutex ele funciona para ser passado
        // em uma struct ou se nao pensar em algo
        int exTms = 1, waitTms = 3000; 
        pthread_create(&ids, NULL, func, NULL);
        printf("thread criada\n");
        sleep(2);
        printf("Inciando\n");
        int i = 1;
        while(i){
                printf("Iniciando thread por %d ms\n", exTms);
                // se nao vai colocar no thread esse m2, ele nao tem necessidade de usar ele aqui
                pthread_mutex_lock(&m2);
                flag = 0; 
                pthread_mutex_unlock(&m2);

                pthread_cond_signal(&c);

                usleep(1000 * exTms);
                printf("Thread dormindo por %d ms\n", waitTms);

                pthread_mutex_lock(&m2);
                flag = 1;
                pthread_mutex_unlock(&m2);

                usleep(waitTms * 1000);

                pthread_mutex_lock(&m);
                if(finalizer){ 
                        i = 0;
                }
                pthread_mutex_unlock(&m);
        }

        return 0;
}
