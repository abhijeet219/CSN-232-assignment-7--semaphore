#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <assert.h> 


typedef struct
{
        pthread_mutex_t lock;
        pthread_cond_t wait;
        int value;
  int waiters;
} sema;

sema *InitSem(int count)
{       
        sema *s;

        s = (sema *)malloc(sizeof(sema));
        if(s == NULL) {
                return(NULL);
        }
        s->value = count;
        s->waiters = 0;
        pthread_cond_init(&(s->wait),NULL);
        pthread_mutex_init(&(s->lock),NULL);

        return(s);
}

void P(sema *s)
{
        pthread_mutex_lock(&(s->lock));
        s->value--;

        while(s->value < 0) {
                if(s->waiters < (-1 * s->value)) {
                        s->waiters++;
                        pthread_cond_wait(&(s->wait),&(s->lock));
                        s->waiters--;
                } else {
                        break;
                }
        }

        pthread_mutex_unlock(&(s->lock));

        return;
}

void V(sema *s)
{

        pthread_mutex_lock(&(s->lock));
        s->value++;

        if(s->value <= 0)
        {
                pthread_cond_signal(&(s->wait));
        }

        pthread_mutex_unlock(&(s->lock));
}


#define LEN 4

struct sema sem_producer;  
struct sema sem_consumer;  /

pthread_mutex_t mut_buf = PTHREAD_MUTEX_INITIALIZER;

int buf[LEN];
int first_occupied = 0;
int first_empty = 0;

void push_buf(int val) {
  buf[first_empty] = val;
  first_empty++;
  if (first_empty >= LEN) {
    first_empty = 0;
  }
}

int take_from_buf() {
  int val = buf[first_occupied];
  first_occupied++;
  if (first_occupied >= LEN) {
    first_occupied = 0;
  }
  return val;
}

void *producer(void *arg) {
  int work_item = 1;

  while (1) {
    sleep( rand() % 5 );
    P(&sem_producer);  

    pthread_mutex_lock(&mut_buf);
      push_buf(work_item++);  
    pthread_mutex_unlock(&mut_buf);

    V(&sem_consumer);  
  }
}

void *consumer(void *arg) {
  while (1) {
    int work_item;

    sleep( rand() % 5 );
    P(&sem_consumer); 

    pthread_mutex_lock(&mut_buf);
      work_item = take_from_buf();
    pthread_mutex_unlock(&mut_buf);

    V(&sem_producer); 

    printf("%d ", work_item);
    fflush(stdout);  
  }
  return NULL;
}
int run(int num_P, int num_C) {
  sem_producer=*InitSem(LEN);
  sem_consumer = * InitSem(0);
  int rc;

	for (int i = 0; i < num_P; i++) {

	  pthread_t pro;
	  rc = pthread_create(&pro, NULL, &producer, NULL);
    assert(rc == 0);
	}
   
  for (int i = 0; i < num_P; i++) {

	  pthread_t con;
	  rc = pthread_create(&con, NULL, &consumer, NULL);
    assert(rc == 0);
  }
  
  while (1) {
    sleep(1);
  }
}

int main() {
  int numProducer;
  int numConsumer;

  printf("How many producers?\n");
  scanf("%d", &numProducer);

  printf("How many consumers?\n");
  scanf("%d", &numConsumer);

	run(numProducer, numConsumer);

	return 0;
}
