#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/syscall.h>

pthread_mutex_t counterMutex = PTHREAD_MUTEX_INITIALIZER;
long counterMutexWait = 0;

pthread_mutex_t waitMutex = PTHREAD_MUTEX_INITIALIZER;
long waitMutexWait = 0;

const int threadCount = 32;
const int totalWork = 128;
int threadCountCur = 1;
int worldCounter = 0;
int testCount = 15;

pid_t gettid() {
   return syscall(SYS_thread_selfid);
}

void upCountTask(pid_t pid) {
//      printf("\n uc[%d] ", pid);
      clock_t start, end;
      start = clock();
      pthread_mutex_lock(&counterMutex);
      end = clock();
      counterMutexWait += (end - start);
      worldCounter++;
      pthread_mutex_unlock(&counterMutex);
}

void writeTask(pid_t pid) {
   FILE *fp = fopen("test.txt", "a+");
//   printf("\n wr[%d] ", pid);

      if (fp != NULL) {
         fprintf(fp, "Writing");
         fflush(fp);
         usleep(2000);
         fclose(fp);
      }
}

void waitTask(pid_t pid) {
//      printf("\n wt[%d] ", pid);
      clock_t start, end;
      start = clock(); 
      pthread_mutex_lock(&waitMutex);
      end = clock();
      waitMutexWait += (end - start);
      usleep(100);
      pthread_mutex_unlock(&waitMutex);
}

void *thread_work_1() {
   clock_t start, end;
   int i = 0;
   pid_t tid = gettid();

//   printf("\nStarting thread_work\n");

   start = clock();
   for (i = 0 ; i < totalWork/threadCountCur; i++) {
      waitTask(tid);
      writeTask(tid);
      upCountTask(tid);
   }
   end = clock();
//   printf("\nTask done : Time taken is %lu\n", end - start / CLOCKS_PER_SEC);

   return NULL;
}


void *thread_work_2() {
   clock_t start, end;
   int i = 0;
   pid_t tid = gettid();

//   printf("\nStarting thread_work\n");

   start = clock();
   for (i = 0 ; i < totalWork/threadCountCur; i++) {
      upCountTask(tid);
      waitTask(tid);
      writeTask(tid);
   }
   end = clock();
//   printf("\nTask done : Time taken is %lu\n", end - start / CLOCKS_PER_SEC);

   return NULL;
}

int main() {
   pthread_t threads[threadCount];
   int i = 0;
   int curThread;
   void *ret = NULL;
   clock_t start, end;
   int curTestNumber = 0; 
   FILE *fResultCSV = fopen("threadResult.csv", "a+");
   
   if (fResultCSV == NULL) {
      printf("File open failed.\n");
      return 0;
   }
   
   fprintf(fResultCSV, "ThreadCount, TimeTaken, WaitTime - CounterMutex, WaitTime - WaitMutex, WaitTime/Thread - CounterMutex, WaitTime/Thread - WaitMutex");
   for (curTestNumber = 0; curTestNumber < testCount; curTestNumber++) {
      printf("\n ===== Starting Test #%d =====\n\n", curTestNumber + 1);
           for (curThread = 2; curThread <= threadCount; curThread*=2) {  
                   threadCountCur = curThread;

                   /* Reset counters for the current test run */
                   counterMutexWait = 0;
                   waitMutexWait = 0;

                   start = clock();
                   for ( i = 0; i < curThread; i++) {
                      if (i%2 == 0) {
                          pthread_create(&threads[i], NULL, thread_work_1, NULL);
                       } else {
                          pthread_create(&threads[i], NULL, thread_work_1, NULL);
                       }
                   } 

                   for ( i = 0; i < curThread; i++) {
                      pthread_join(threads[i], &ret);
                   }
                   end = clock();

                   printf("Done with all work with [%d] threads.", curThread);
                   printf("\n\tTime taken is %lu \n\tCounterMutexWait = %ld \n\tWaitMutexWait = %ld", (end - start) , counterMutexWait, waitMutexWait);
                   printf("\n\tCounterMutexWait/thread = %ld \n\tWaitMutexWait/thread = %ld\n\n", counterMutexWait/curThread, waitMutexWait/curThread);

                   fprintf(fResultCSV, "\n%d, %lu, %ld, %ld, %ld, %ld", curThread, (end - start) , counterMutexWait, waitMutexWait, counterMutexWait/curThread, waitMutexWait/curThread);
           }
           printf("\n");
   }

   fclose(fResultCSV);
   return 0;
}
