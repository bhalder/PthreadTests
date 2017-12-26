#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/syscall.h>

/* Different controls */
#define QUEUESIZE 4 
#define CONSUMERCOUNT 2
#define PRODUCERDELAY 50000
#define CONSUMERDELAY 10000
#define PRODUCEITEMS 100

/* Different State of the Queue */
#define ACTIVE 0
#define INACTIVE 1

/* Queue Structure */
typedef struct queue_t {
   int queue[QUEUESIZE];
   int front;
   int back;
   int state;
   pthread_mutex_t mutex;
} queue;

queue* initQueue(queue *newQueue) {
   if (newQueue != NULL) {
      newQueue->front = -1;
      newQueue->back = -1;
      newQueue->state = ACTIVE;
      pthread_mutex_init (&(newQueue->mutex), NULL); 
   }
   return newQueue;
}

queue commonQueue;

/* Helper Functions */
pid_t gettid() {
   return syscall(SYS_thread_selfid);
}

void printQueue(queue *q) {
   int i;

   printf("\n[TID : %d] ", gettid());
   for( i = 0; i < QUEUESIZE; i++) {
      printf("[ %d : %d ] ", i, q->queue[i]);
   }

}

/* Queue Operations */
void enqueue(int element) {
   if ( (commonQueue.back + 1) % QUEUESIZE == commonQueue.front) {
      printf("\nQueue Full");
      return;
   }

   commonQueue.back = (commonQueue.back + 1) % QUEUESIZE;
   commonQueue.queue[commonQueue.back] = element;
   printf("\nElement inserted : %d", element);
}

int dequeue() {
   int ret = -1;

   if (commonQueue.back == -1 || commonQueue.back == commonQueue.front) {
      printf("\nQueue Empty");
      return ret;
   }

   commonQueue.front = (commonQueue.front + 1) % QUEUESIZE;
   ret = commonQueue.queue[commonQueue.front];
   printf("\nRET : %d", ret);
   return ret;
}

/* Thread Operations */
void *produce() {
   int number = 0; 
   int itemsProduced = 0;
   
   while (itemsProduced != PRODUCEITEMS) {
      number = rand() % 100;
      pthread_mutex_lock(&commonQueue.mutex);
      enqueue(number);
      printQueue(&commonQueue);
      //usleep(PRODUCERDELAY);
      printf(" Produced %d/%d", itemsProduced, PRODUCEITEMS);
      itemsProduced++;
      pthread_mutex_unlock(&commonQueue.mutex);
   }

   commonQueue.state = INACTIVE;
   printf("\nExiting producer.");
   return NULL;
}

void *consume() {
   while (commonQueue.state == ACTIVE) {
      pthread_mutex_lock(&commonQueue.mutex);
      dequeue();
      printQueue(&commonQueue);
      //usleep(CONSUMERDELAY);
      pthread_mutex_unlock(&commonQueue.mutex);
   }

   printf("\nExiting consumer.");
   return NULL;
}

int main() {
   pthread_t producer;
   pthread_t consumer[CONSUMERCOUNT];
   int i = 0;

   initQueue(&commonQueue);
   pthread_create(&producer, NULL, produce, NULL);   

   for (i = 0; i < CONSUMERCOUNT; i++) {
      pthread_create(&consumer[i], NULL, consume, NULL);
   }

   pthread_join(producer, NULL);
   for (i = 0; i < CONSUMERCOUNT; i++) {
      pthread_join(consumer[i], NULL);
   }

   printf("\n");
   return 0;
}
