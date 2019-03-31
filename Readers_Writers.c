#include<stdio.h>
#include<pthread.h>
// #include<semaphore.h>
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

// sem_t mutex,writeblock;
sema mutex,writeblock;
int data = 0,rcount = 0;

void *reader(void *arg)
{
  int f;
  f = ((int)arg);
  // P(&mutex);
  P(&mutex);
  rcount = rcount + 1;
  if(rcount==1)
   // P(&writeblock);
   P(&writeblock);
  V(&mutex);
  printf("Data read by the reader%d is %d\n",f,data);
  sleep(1);
  P(&mutex);
  rcount = rcount - 1;
  if(rcount==0)
   V(&writeblock);
  V(&mutex);
}

void *writer(void *arg)
{
  int f;
  f = ((int) arg);
  P(&writeblock);
  data++;
  printf("Data writen by the writer%d is %d\n",f,data);
  sleep(1);
  V(&writeblock);
}

int main()
{
  int i,b; 
  pthread_t rtid[5],wtid[5];
  sem_init(&mutex,0,1);
  sem_init(&writeblock,0,1);
  for(i=0;i<=2;i++)
  {
    pthread_create(&wtid[i],NULL,writer,(void *)i);
    pthread_create(&rtid[i],NULL,reader,(void *)i);
  }
  for(i=0;i<=2;i++)
  {
    pthread_join(wtid[i],NULL);
    pthread_join(rtid[i],NULL);
  }
  return 0;
}