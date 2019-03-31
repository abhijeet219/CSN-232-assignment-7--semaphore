#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

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

sema agent_ready;

sema smoker_semaphors[3];
char* smoker_types[3] = { "matches & tobacco", "matches & paper", "tobacco & paper" };

bool items_on_table[3] = { false, false, false };

sema pusher_semaphores[3];

void* smoker(void* arg)
{
	int smoker_id = *(int*) arg;
	int type_id   = smoker_id % 3;

	for (int i = 0; i < 3; ++i)
	{
		printf("\033[0;37mSmoker %d \033[0;31m>>\033[0m Waiting for %s\n",
			smoker_id, smoker_types[type_id]);


		printf("\033[0;37mSmoker %d \033[0;32m<<\033[0m Now making the a cigarette\n", smoker_id);
		usleep(rand() % 50000);
		V(&agent_ready);

		printf("\033[0;37mSmoker %d \033[0;37m--\033[0m Now smoking\n", smoker_id);
		usleep(rand() % 50000);
	}

	return NULL;
}

sema pusher_lock;

void* pusher(void* arg)
{
	int pusher_id = *(int*) arg;

	for (int i = 0; i < 12; ++i)
	{
		P(&pusher_semaphores[pusher_id]);
		P(&pusher_lock);

		if (items_on_table[(pusher_id + 1) % 3])
		{
			items_on_table[(pusher_id + 1) % 3] = false;
			V(&smoker_semaphors[(pusher_id + 2) % 3]);
		}
		else if (items_on_table[(pusher_id + 2) % 3])
		{
			items_on_table[(pusher_id + 2) % 3] = false;
			V(&smoker_semaphors[(pusher_id + 1) % 3]);
		}
		else
		{
			items_on_table[pusher_id] = true;
		}

		V(&pusher_lock);
	}

	return NULL;
}

void* agent(void* arg)
{
	int agent_id = *(int*) arg;

	for (int i = 0; i < 6; ++i)
	{
		usleep(rand() % 200000);

		P(&agent_ready);

		V(&pusher_semaphores[agent_id]);
		V(&pusher_semaphores[(agent_id + 1) % 3]);

		printf("\033[0;35m==> \033[0;33mAgent %d giving out %s\033[0;0m\n",
			agent_id, smoker_types[(agent_id + 2) % 3]);
	}

	return NULL;
}


int main(int argc, char* arvg[])
{
	srand(time(NULL));

	agent_ready=*(InitSem(1));
	pusher_lock = *(InitSem(1));
	for (int i = 0; i < 3; ++i)
	{
		smoker_semaphors[i] = *InitSem(0);
		pusher_semaphores[i] = *InitSem(0);
	}



	int smoker_ids[6];

	pthread_t smoker_threads[6];

	for (int i = 0; i < 6; ++i)
	{
		smoker_ids[i] = i;

		if (pthread_create(&smoker_threads[i], NULL, smoker, &smoker_ids[i]) == EAGAIN)
		{
			perror("Insufficient resources to create thread");
			return 0;
		}
	}

	int pusher_ids[6];

	pthread_t pusher_threads[6];

	for (int i = 0; i < 3; ++i)
	{
		pusher_ids[i] = i;

		if (pthread_create(&pusher_threads[i], NULL, pusher, &pusher_ids[i]) == EAGAIN)
		{
			perror("Insufficient resources to create thread");
			return 0;
		}
	}

	int agent_ids[6];

	pthread_t agent_threads[6];

	for (int i = 0; i < 3; ++i)
	{
		agent_ids[i] =i;

		if (pthread_create(&agent_threads[i], NULL, agent, &agent_ids[i]) == EAGAIN)
		{
			perror("Insufficient resources to create thread");
			return 0;
		}
	}

	for (int i = 0; i < 6; ++i)
	{
		pthread_join(smoker_threads[i], NULL);
	}

	return 0;
}