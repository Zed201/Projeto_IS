#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>

struct pro{
        pthread_cond_t *con;
        pthread_mutex_t *mutex;
        // colocar o pthread_t aqui tbm
};

// atomic_flag condi = ATOMIC_FLAG_INIT;
// a forma de verificar o atomic flag é dificl
volatile int flag = 0;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
int finalizer = 0;

void *func(void *agrs){
        pthread_cond_wait(&c, &m);
        for (int i = 0; i < 2000; i++) {
                if(!atomic_load(&flag)){ // flag 1 ele para, zero ele continua
                        pthread_cond_wait(&c, &m);
                }
                // alguma execuçao
                printf("Alguma coisa %d\n", i);
        }
        pthread_mutex_unlock(&m);
        printf("Saiu loop\n");
        pthread_mutex_lock(&m); // Funcionando pois tem de destravar o mutex
        finalizer = 1;
        pthread_mutex_unlock(&m);
        pthread_exit(NULL);
}



int main(){
        pthread_t ids;
        // TODO:
        // testar se a inicialização dinamica do cond e do mutex ele funciona para ser passado
        // em uma struct ou se nao pensar em algo
        int exTms = 5, waitTms = 3000; 
        pthread_create(&ids, NULL, func, NULL);
        printf("thread criada\n");
        sleep(2);
        printf("Inciando\n");
        int i = 1;
        while(i){
                printf("Iniciando thread por %d ms\n", exTms);
                atomic_store(&flag, 1);
                pthread_cond_signal(&c);

                usleep(1000 * exTms); // deixa executar por 3 seg
                printf("Thread dormindo por %d ms\n", waitTms);
                atomic_store(&flag, 0);
                usleep(waitTms * 1000);
                pthread_mutex_lock(&m);
                if(finalizer){ // talvez usar _atomic nisso
                        i = 0;
                }
                pthread_mutex_unlock(&m);
        }

        return 0;
}
