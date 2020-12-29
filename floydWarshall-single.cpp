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


/*-------- 2. Create userGraph function that will take the user's input-------*/
void userGraph(){
    // retrieve nodes from the user
   while(1) {
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

   while(1) {
     // retrieve the number of vertices from the user
        printf("\n");
        printf("Enter the number of vertices: ");
        scanf("%d", &verticesCount);
        if (verticesCount < 1 ){
          printf("The edge count must not be less than 0\n");;
          printf(" ");
        } else {
          break;
        }
    }

   // graph allocation
   graph = (int**)malloc(sizeof (int*) * nodeCount);
   for (int i = 0 ; i < nodeCount ; i++){
       graph[i] = (int*) malloc(sizeof (int) *nodeCount);
   }
   //retrieve the vertices count
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
     printf("\nEnter the vertices [%d]:", i);
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

/*------ 3. Create distance between nodes function (no intermediate nodes)----*/
//create distance function where the 2d distance array is created.
void initialDistance () {
  // Create a pointer to an integer pointer not to an actual integer.
  dist = (int**)malloc(sizeof (int*) *nodeCount);
  for (int i = 0 ; i < nodeCount; i++){
    //this points to an actual integer in memory. These integers are the values
    // in the matrix.
      dist[i] = (int*) malloc(sizeof (int) *nodeCount);
  }
  for (int i = 0 ; i < nodeCount ; i ++){
      for (int j = 0 ; j < nodeCount; j++){
        // Initial values of shortest distances - No intermediate node used
        dist[i][j] = graph[i][j];
      }
  }
}
/*--------------------END DISTANCE BETWEEN NODES FUNCTION---------------------*/


/*-----------------------4. Create single threaded FW Algorithm--------------*/
void floydWarshallSingle() {
  initialDistance();
  for (int k = 0; k < nodeCount; k++) {
      for (int i = 0; i < nodeCount; i++) {
          for (int j = 0; j < nodeCount; j++) {
              // If vertex k is on the shortest path from
              // i to j, then update the value of dist[i][j]
              if  (dist[i][k] != INF && dist[k][j] != INF &&
                dist[i][j] > dist[i][k] + dist[k][j]) {
                  dist[i][j] = dist[i][k] + dist[k][j];
                }
              }
            }
          }
        }
/*--------------------------- END SINGLE THREADED ----------------------------*/


/*---------5. Create print function to print the initial graph ----------------*/
//create function to print out the graph
void printInitialGraph() {
  //print out the graph
  printf("\nInitial Matrix:\n");
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
/*--------------- END INITIAL GRAPH - NO intermiate nodes ------------------*/

/*----6. Create print function to print updated graph - intermiate nodes------*/

//create function to print out the updated graph with the shortest paths
void printUpdatedGraph() {
  //print out the updated matrix
  printf("\nUpdated Matrix:\n");
  for (int i = 0 ; i < nodeCount; i ++){
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
/*------------------------- END UPDATED GRAPH FUNCTION -----------------------*/

/*------------- 6. Call all functions in main to display results -------------*/
int main()
{

  clock_t startingTime;
  clock_t endingTime;
  double cpu_time_used;

  startingTime = clock();
  userGraph();
  //call initial distance function
  initialDistance();
  floydWarshallSingle();
  //call print graph function to print the graph
  printInitialGraph();
  //call print updated graph here
  printUpdatedGraph();
  printf("\n");

  endingTime = clock();
  cpu_time_used = ((double) (endingTime - startingTime)) / CLOCKS_PER_SEC;

  // print time in milliseconds and seconds
  printf("------------------");
  printf("     \nSINGLE THREAD TIME");
  printf("\n------------------");
  printf("\nTime taken by one thread was %f seconds\n", cpu_time_used);
}
/*----------------------------- END MAIN -----------------------------------------*/
