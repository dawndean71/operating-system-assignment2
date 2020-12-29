/* Author: Dawn Dean A00401493 and Rafid Akhtar A00391501
 * CSCI 3431 - Operating Systems
 * Assignment #2
 * Professor: Dr. Jiju Poovvancheri
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>
#include <iostream>
#include <semaphore.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <time.h>


/*-------------------------------1. Create variables -----------------------*/
using namespace std;

// maximum number for INF
int INF = INT_MAX;
// matrix graph
int **graph = NULL;
//distance between each node
int **dist = NULL;
//number of nodes in the matrix
int nodeCount = 0;
int verticesCount = 0;
// Create the variables needed for synchronization: readCount, readCountLock,
// matrixObjectLock
int readCount = 0;
//Create mutex readCountLock for mutual exclusion
mutex readCountLock;
//create semaphore to allow one thread to write to the matrix at a time.
sem_t matrixObjectLock;


/*-------- 2. Create userGraph function that will take the user's input-------*/
void userGraph(){
    // retrieve nodes from the user
   while(1){
     printf("Enter the number of nodes: ");
     scanf("%d", &nodeCount);

     // if node count is less than 0, print an error msg
     if (nodeCount < 1 ){
       printf("The node count must not be less than 0\n");
       printf(" ");
     } else {
       break;
     }
   }

   while(1){
     // retrieve the number of vertices from the user
        printf("\n");
        printf("Enter the number of vertices: ");
        scanf("%d", &verticesCount);
        if (verticesCount < 1 ){
          printf("The edge count must not be less than 0\n");
          printf(" ");
        } else {
          break;
        }
    }

   //graph allocation
   graph = (int**)malloc(sizeof (int*) * nodeCount);
   for (int i = 0 ; i < nodeCount ; i++){
       graph[i] = (int*) malloc(sizeof (int) *nodeCount);
   }
   //retrieve vertices count
   for (int i = 0 ; i < nodeCount ; i ++){
       for (int j = 0 ; j < nodeCount ;j ++){
           if (i == j ){
               graph[i][j] = 0;
           }
           else {
               graph[i][j] = INF;
           }
       }
   }
   int* vertices = (int*)malloc(sizeof (int) * 5);
   for (int i = 0 ; i < verticesCount ; i++){
     printf("\nEnter the vertice count [%d]:", i);
     // retrieve user input
       scanf("%d", &vertices[0]);
       scanf("%d", &vertices[1]);
       scanf("%d", &vertices[2]);
       printf("\n");

       if (vertices[2] < 0){
           printf("The weight (edge) must be greater than 0!!");
           i--;
       }
       // The path will be the same from both directions
       // Eg: (1)---5---(2)  is the same as (2)---5----(1)
       graph[vertices[0]-1][vertices[1]-1] = vertices[2];
       graph[vertices[1]-1][vertices[0]-1] = vertices[2];
   }
}


/*------ 3. Create thread synchronization function (readers-writers) --------*/
void floydWarshallMultiThreadWorker(int i, int k){
    // Note: firstNode = i;
    // Note: interNode = k;
    // Note: secondNode = j;

    //Create a boolean to check if a thread wants to write.
    bool threadNeedsToWrite;

    //increment j up to how many nodes the user inputs.
    for (int j = 0 ; j < nodeCount;  j++) {

      // The first thread grabs the lock so no other thread can enter and incrememnts
      // readCountLock at the same time.
       readCountLock.lock();
       // The first thread wants to let other threads know that it is inside to read
       readCount++;
       // Only one thread is inside currently reading...
       if (readCount == 1){
         // the first thread acquires the matrix lock and tells the other reader
         // threads to wait while it is writing to the matrix.
           sem_wait(&matrixObjectLock);
       }
       // the first thread releases the lock so other threads can enter and have
       // a chance to read/write.
       readCountLock.unlock();

       // If this boolean is false, the thread does not want to write,
       // so it grabs the readCountLock.
       threadNeedsToWrite = (dist[i][k] != INF && dist[k][j] != INF &&
               dist[i][j] > dist[i][k] + dist[k][j]);

               // Thread does not want to read anymore, so it grabs the lock.
               readCountLock.lock();

               // It then decrements the readCount to let the other threads know
               // that it is leaving.
               readCount--;

               // Since that thread left, we have to check if any threads are inside
               // If no thread is inside, we send a signal.
               if (readCount == 0 ){
                 //sending signal to the waiting threads so they can wake up.
                   sem_post(&matrixObjectLock);
               }
               //the threads that have woken up are now racing to grab the lock
               // since it is available now.
               readCountLock.unlock();

           // if the boolean is true (a shorter path is found)
           if(threadNeedsToWrite){
             //First thread to grab the matrix lock will enter the matrix
             sem_wait(&matrixObjectLock);
             // this thread will then write to the matrix and update it with
             // the shortest path
             dist[i][j] = dist[i][k] + dist[k][j];
             // After the thread is done writing to the matrix, it will let the
             // other threads know that it is done and they can now read/write
             // from/to the matrix.
             sem_post(&matrixObjectLock);
           }
         }
       }

/*-----------------------END THREAD SYNCHRONIZATION FLOYDD W -----------------*/

/*------------- 5. Create multithreaded FW Algorithm function ----------------*/
void floydWarshallMultiThread(){
  //Create a thread pointer to point to all the threads in memory.
    thread* thread_floyd = (thread*)malloc(sizeof (thread) * nodeCount);
    for (int k = 0 ; k < nodeCount;  k++) {
        for (int i = 0 ; i < nodeCount;  i++){
            // Instead of copying the thread object to another object, we mov()
            // all of the resources from one thread object (the flodWarshallWorker)
            // to multiple threads. Why? We used this in order to limit the
            // amount of resources, time, power and memory required to get the
            // job done. Each thread will use the floydWarshallMultiThreadWorker
            // to calculate the shortest path. Since it is multithreading, each
            // thread will have a chance to acquire and release the readLock.
            thread_floyd[i] = move(thread(floydWarshallMultiThreadWorker, i, k));
        }
        // Here we join the threads. So each thread must wait for another to end,
        // so that it can safely access its result. This is a way to keep threads
        // synchronized.
        /// We want to synchronize the threads in order to avoid two threads
        // entering the critical section at the same time.
        for (int thread = 0; thread < nodeCount ; thread++){
            thread_floyd[thread].join();
        }
    }
  }
/*------------------------------END MULTI-THREADED ---------------------------*/


/*----- 6.Create distance between nodes function (no intermediate nodes) -----*/
//create distance function where the 2d distance array is created.
void initialDistance () {
  // Allocating array of integer pointers.
  // i.e. pointer --> pointer --> integer
  dist = (int**)malloc(sizeof (int*) *nodeCount);
  for (int i = 0 ; i < nodeCount; i++){
    //this points to an actual integer in memory. These integers are the values
    // in the matrix.
      dist[i] = (int*) malloc(sizeof (int) *nodeCount);
  }
   // Initial values of shortest distances - No intermediate node used
  for (int i = 0 ; i < nodeCount ; i ++){
      for (int j = 0 ; j < nodeCount; j++){
        dist[i][j] = graph[i][j];
      }
  }
}
/*--------------------END DISTANCE BETWEEN NODES FUNCTION---------------------*/

/*--------- 7. Create print function to print the initial graph ----------------*/
//create function to print out the graph
void printInitialGraph() {
  //print out the graph
  printf("\nDistance Matrix:\n");
  for (int i = 0 ; i < nodeCount; i++){
      for (int j = 0; j < nodeCount; j++){
    if (graph[i][j] == INF) {
      printf("INF ");

    } else {
      printf("%d", graph[i][j]);
      printf(" ");
    }
  }
  printf("\n");
}
printf("\n");
}
/*---------------- END INITIAL GRAPH - No Intermediate Nodes ------------------*/

/*---- 8. Create print function to print updated graph - intermiate nodes------*/
//create function to print out the updated graph with the shortest paths
void printUpdatedGraph() {
  //print out the updated matrix
  printf("\nUpdated Matrix:\n");
  for (int i = 0 ; i < nodeCount ; i ++){
      for (int j = 0 ; j < nodeCount; j++){
        //check if distance is equal to infinity. If so, make that value in the
        // matrix INF
          if (dist[i][j]  == INF){
            printf("INF ");
          } else {
            printf("%d", dist[i][j]);
            printf(" ");
          }
      }
      printf("\n");
  }
}
/*------------------------------ END UPDATED GRAPH FUNCTION --------------------*/

/*------------- 9. Call all functions in main to display results -------------*/
int main()
{
  clock_t startingTime;
  clock_t endingTime;
  double cpu_time_used;

  startingTime = clock();

  userGraph();
  //intitialize matrix lock semaphore
  // first param:  initializes semaphore at the matrixObectLock address
  // second param: whether the semaphore should be shared between threads. If it
  // is 0 (false) it is shared and is located at an address visible to all threads.
  // third param: value is the initial value of the semaphore. A positive value
  // (greater than zero) indicates that the semaphore is unlocked and a value of
  // 0 indicates a locked semaphore.
  sem_init(&matrixObjectLock, false, 1);

  //call initial distance function
  initialDistance();
  floydWarshallMultiThread();
  //call print graph function to print the graph
  printInitialGraph();
  //call print updated graph here
  printUpdatedGraph();
  printf("\n");

  endingTime = clock();
  cpu_time_used = ((double) (endingTime - startingTime)) / CLOCKS_PER_SEC;



  // print time in milliseconds and seconds
  printf("------------------");
  printf("     \nMULTI THREAD TIME");
  printf("\n------------------");
  printf("\nTime taken by multiple threads was %f seconds\n", cpu_time_used);
}
/*----------------------------- END MAIN -----------------------------------------*/
