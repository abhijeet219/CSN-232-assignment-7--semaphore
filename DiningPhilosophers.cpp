#include <iostream>

#include <thread>
#include <ctime>
#include <chrono>

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
        if(s->value>0)
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
        if(s->value==0)
        s->value++;

        if(s->value <= 0)
        {
                pthread_cond_signal(&(s->wait));
        }

        pthread_mutex_unlock(&(s->lock));
}

bool try_lock(sema *s){
    bool ch =false;
    pthread_mutex_lock(&(s->lock));
    if(s->value>0) ch = true;
     pthread_mutex_unlock(&(s->lock));
     return ch;
}
constexpr int num_Phil = 5;
std::thread philosphers[num_Phil]; 
sema mtx[num_Phil];
sema cout_mutex; 
int ate[5] = {0};
int intrupt = 100; 

void print(std::string str){ 
    P(&cout_mutex);
    std::cout << str << std::endl;
    V(&cout_mutex);
}

void think(int id){ 
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); 
    print("Philosopher " + std::to_string(id) + " is thinking."); 
}

bool eat(int id, int left, int right) { 

    while(1) if (try_lock(&mtx[left])) { 

        print("Philosopher " + std::to_string(id) + " got the fork " + std::to_string(left));
    	
        if (try_lock(&mtx[right])) {
            print("Philosopher " + std::to_string(id) + " got the fork " + std::to_string(right) + 
                "\nPhilosopher " + std::to_string(id) + " eats."); // print
            return true;
        } else {
            V(&mtx[left]); 
        }

    }

    return false;
}

void putDownForks(int left, int right) {
    V(&mtx[left]);
    V(&mtx[right]);
}

void dinner_started(int philID) {
	
    int lIndex = std::min(philID, (philID + 1) % (num_Phil));
    int rIndex = std::max(philID, (philID + 1) % (num_Phil));

    while (intrupt-- > 0) { 
        if (eat(philID, lIndex, rIndex)) { 
            ate[philID]++;
            std::this_thread::sleep_for(std::chrono::milliseconds(600));
        }
    }

}

void dine(){
	
    for (int i = 0; i < num_Phil; ++i) philosphers[i] = std::thread(dinner_started, i);
	for (int i = 0; i < num_Phil; ++i) philosphers[i].join();
}

int main() { 
    dine();
    for (int i = 0; i < num_Phil; ++i) std::cout << i << " = " << ate[i] << std::endl;
}