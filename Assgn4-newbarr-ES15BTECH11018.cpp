#include <iostream>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>
#include <random>
#include <fstream>
#include <fcntl.h>

using namespace std;

int n, k;
int prel, postl;

default_random_engine gen1, gen2;
uniform_int_distribution<int> dist_pre(10000, 20000); //uniform distribution in given range
uniform_int_distribution<int> dist_post(10000, 20000);


ifstream in("inp-params.txt");
FILE *fp = fopen("new-barr-log.txt", "w");
FILE *fpp = fopen("pthread-barr-log.txt", "w");
FILE * fa = fopen("Average_time.txt", "w");

double average_time = 0.0, paverage_time = 0.0;

//get formatted time
string getTime(time_t input)
{
  struct tm * timeinfo;
  timeinfo = localtime (&input);
  static char output[10];
  sprintf(output,"%.2d:%.2d:%.2d",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
  string tim(output);
  return tim;
}

sem_t sem_counter;
sem_t sem_barrier;
int counter = 0;
pthread_barrier_t barrier;

void* worker(void* thr_id) {
    int tid = *(int *) thr_id;
    for(int i = 0; i < k; i++) {

        time_t beforeBarrSleep = time(NULL);
        string reqTime = getTime(beforeBarrSleep);
        
        fprintf(fp, "Going to sleep before the %dth barrier invocation by Thread %d at time : %s\n", i + 1, tid, reqTime.c_str());
        usleep(dist_pre(gen1));
        
        time_t entry_time = time(NULL);
        string entryTime = getTime(entry_time);
        fprintf(fp, "Before the %dth barrier invocation by Thread %d at time : %s\n", i + 1, tid, entryTime.c_str());
        //barrier point
	
	sem_wait(&sem_counter); //wait on counter 
	if(counter == n - 1) { //If last thread to enter
	   counter = 0;
	   sem_post(&sem_counter); //update counter to 0 and free it

	   for(int i = 0; i < n - 1; i++) { //post every thread waiting on barrier
	   	sem_post(&sem_barrier);
	   }
	}	
	else {
	   counter++; //update and free counter
	   sem_post(&sem_counter); 

	   sem_wait(&sem_barrier); //wait on barrier
	}

        time_t after_barrier = time(NULL);
        string afterTime = getTime(after_barrier);
        fprintf(fp, "After the %dth barrier invocation by Thread %d at time : %s\n", i + 1, tid, afterTime.c_str());


        time_t exit_time = time(NULL);
        string exitTime = getTime(exit_time);
        
        fprintf(fp, "Going to sleep after the %dth barrier invocation by Thread %d at time : %s\n", i + 1, tid, exitTime.c_str());
        average_time += (exit_time - entry_time); 
        usleep(dist_post(gen2));
    }
}


void* pworker(void* thr_id) {
    int tid = *(int *) thr_id;
    for(int i = 0; i < k; i++) { 

        time_t beforeBarrSleep = time(NULL);
        string reqTime = getTime(beforeBarrSleep);
        
        fprintf(fpp, "Going to sleep before the %dth barrier invocation by Thread %d at time : %s\n", i + 1, tid, reqTime.c_str());
        usleep(dist_pre(gen1));
        
        time_t entry_time = time(NULL);
        string entryTime = getTime(entry_time);
        fprintf(fpp, "Before the %dth barrier invocation by Thread %d at time : %s\n", i + 1, tid, entryTime.c_str());
        //barrier point
	pthread_barrier_wait(&barrier);

        time_t after_barrier = time(NULL);
        string afterTime = getTime(after_barrier);
        fprintf(fpp, "After the %dth barrier invocation by Thread %d at time : %s\n", i + 1, tid, afterTime.c_str());


        time_t exit_time = time(NULL);
        string exitTime = getTime(exit_time);
        
        fprintf(fpp, "Going to sleep after the %dth barrier invocation by Thread %d at time : %s\n", i + 1, tid, exitTime.c_str());
        paverage_time += (exit_time - entry_time); 
        usleep(dist_post(gen2));
    }
}

int main() {
    in >> n >> k >> prel >> postl;
    pthread_t worker_threads[n], pworker_threads[n];
    int ids[n];	
    gen1.seed(prel); //Assign respective seeds to the random generators
    gen2.seed(postl);
    
    sem_init(&sem_counter, 0, 1);
    sem_init(&sem_barrier, 0, 0);

    for(int i = 0; i < n; i++) { //Call threads
        ids[i] = i;
	pthread_create(&worker_threads[i], NULL, worker, &ids[i]);
    }

    for(int i = 0; i < n; i++) {
        pthread_join(worker_threads[i], NULL);
    }

    

    average_time /= n;
    
    fprintf(fa, "Average time to cross barrier semaphore : %f\n", average_time);
    
    pthread_barrier_init(&barrier, NULL, n);

    for(int i = 0; i < n; i++) { //Call threads
        ids[i] = i;
	pthread_create(&pworker_threads[i], NULL, pworker, &ids[i]);
    }

    for(int i = 0; i < n; i++) {
        pthread_join(pworker_threads[i], NULL);
    }

    
    paverage_time /= n;
    
    fprintf(fa, "Average time to cross barrier pthread : %f\n", paverage_time);
       
    
    
    return 0;
}
