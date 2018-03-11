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
FILE * fa = fopen("Average_time.txt", "a+");

double average_time = 0.0;

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

void* worker(void* thr_id) {
    int tid = *(int *) thr_id;
    for(int i = 0; i < k; i++) {
        time_t beforeBarrSleep = time(NULL);
        string reqTime = getTime(beforeBarrSleep);
        
        fprintf(fp, "Going to sleep before the %dth barrier invocation by Thread %d at time : %s\n", i + 1, tid, reqTime.c_str());
        usleep(dist_pre(gen1));
        
        //read
        time_t entry_time = time(NULL);
        string entryTime = getTime(entry_time);
        fprintf(fp, "Before the %dth barrier invocation by Thread %d at time : %s\n", i + 1, tid, entryTime.c_str());
        //barrier point

        time_t after_barrier = time(NULL);
        string afterTime = getTime(after_barrier);
        fprintf(fp, "After the %dth barrier invocation by Thread %d at time : %s\n", i + 1, tid, afterTime.c_str());


        time_t exit_time = time(NULL);
        string exitTime = getTime(exit_time);
        fprintf(fp, "%dth CS Exit by Reader Thread %d at %s\n", i + 1, tid, exitTime.c_str());
        average_time += (entry_time - beforeBarrSleep); 
        usleep(dist_post(gen2));
    }
}

int main() {
    in >> n >> k >> prel >> postl;
    pthread_t worker_threads[n];

    gen1.seed(prel); //Assign respective seeds to the random generators
    gen2.seed(postl);

    for(int i = 0; i < n; i++) { //Call threads
        pthread_create(&worker_threads[i], NULL, worker, &i);
    }

    for(int i = 0; i < n; i++) {
        pthread_join(worker_threads[i], NULL);
    }

    average_time /= n;
    
    fprintf(fa, "Average time to enter CS : %f\n", average_time);
    return 0;
}
