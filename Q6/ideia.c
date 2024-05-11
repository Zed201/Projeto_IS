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
pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;
int finalizer = 0;

void *func(void *agrs){
        pthread_cond_wait(&c, &m);
        for (int i = 0; i < 999999; i++) {
               /* if(atomic_flag_test_and_set(&condi)){
                        // o problema é que ele retorna o valor, mas troca para true a variavel
                        // nao entra aqui
                        pthread_cond_wait(&c, &m);
                        // fica esperando
                } else {
                        // caso a flag esteja em false ele vai retornar false 
                        // e coloca ela como false denovo
                        //atomic_flag_clear(&condi);
                        // acaba executando tudo e so depois vai
                }*/
                if(atomic_load(&flag)){ // flag 1 ele para, zero ele continua
                        pthread_cond_wait(&c, &m);
                }
                // alguma execuçao
                printf("Alguma coisa %d\n", i);
                

        }
        pthread_mutex_lock(&m2); // usando o mutex m ele nao entra, ou seja o mutex ta travado por algum motivo
        printf("Terminando alguma coisa\n");
        finalizer = 1;
        printf("Terminando alguma coisa\n");
        pthread_mutex_unlock(&m2);
        pthread_exit(NULL);
}



int main(){
        pthread_t ids;
        // testar se a inicialização dinamica do cond e do mutex ele funciona para ser passado
        // em uma struct ou se nao pensar em algo
        pthread_create(&ids, NULL, func, NULL);
        printf("thread criada\n");
        sleep(2);
        printf("Inciando\n");
        while(1){
                printf("Iniciando thread por 1 seg\n");
                pthread_cond_signal(&c);

                sleep(1); // deixa executar por 3 seg
                // atomic_flag_clear(&c);
                // concertae if do finalizer
                atomic_store(&flag, 1);
                printf("Thread dormindo por 1 seg\n");
                atomic_store(&flag, 0);
                sleep(1);
                // pthread_mutex_lock(&m);
                if(finalizer == 1){
                        // a thread terminou
                        break;
                }
                // pthread_mutex_unlock(&m);
        }

        return 0;
}
