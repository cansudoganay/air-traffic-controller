#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <queue>
#include <iostream>
#include <ctime>
#include <string.h>
#include "ATCLog.hpp"
using namespace std;


pthread_cond_t pcondl;
pthread_cond_t pcondd;
pthread_cond_t pcondatc;
pthread_mutex_t plock;
time_t init;

int simulationTime;
double prob;
bool isEmergency;
SimulationLog simulationLog;

queue <plane> landingq;
queue <plane> departingq;
queue <plane> emergencyq;

void* landing(void* );
void* emergency(void* );
void* departing(void* );
void* atc(void*);
int pthread_sleep (int seconds);

int planeID=1; //ID of first plane

void printSnapShot(int sec){
  cout << endl << "At " << sec << " sec ground: ";
  queue<plane> tmpDepart = departingq;
  for(auto i = 0; i < tmpDepart.size(); i++){
    cout << tmpDepart.front().planeID;
    if(tmpDepart.size() != 1 || (i+1) != tmpDepart.size()) cout << ",";
    tmpDepart.pop();
  }
  cout << endl;
  cout << "At " << sec << " air: ";
  queue<plane> tmpLand = landingq;
  for(auto i = 0; i < tmpLand.size(); i++){
    cout << tmpLand.front().planeID;
    if(tmpLand.size() != 1 || (i+1) != tmpLand.size()) cout << ",";
    tmpLand.pop();
  }
  cout << endl;
}

void* landing(void* ){
    plane p;
    p.planeID=planeID;
    planeID++;
    p.arrivalTime=time(NULL); 
    //cout << "ID of the Landing Plane: " << p.planeID << " landing time is: " << p.arrivalTime - init << endl;  
    pthread_mutex_lock(&plock);
    if(p.planeID==1){
      //cout << "Tower! this is captain speaking. This is the first plane. Its ID is: " << p.planeID << endl; 
      pthread_cond_signal(&pcondatc);
    }
    if(isEmergency){
      //cout << "Emergency landing occured!" << "ID of emergency plane: " << p.planeID << endl;
      p.status='E';
      emergencyq.push(p);
    }else{
      p.status='L';
      landingq.push(p);
    }
    pthread_cond_wait(&pcondl, &plock);
    pthread_mutex_unlock(&plock);
    pthread_exit(NULL);

}

void* departing(void* ){
    plane p;
    p.planeID=planeID;
    planeID++;  
    p.status='D';
    p.arrivalTime=time(NULL);
    //cout << "ID of Departing Plane: " << p.planeID << " departing time is: " << p.arrivalTime - init << endl;
    pthread_mutex_lock(&plock);
    departingq.push(p);
    if(p.planeID==1){ //if the plane is the first plane 
      //cout << "Tower! this is captain speaking. This is the first plane. Its ID is: " << p.planeID << endl;        
      pthread_cond_signal(&pcondatc);
    }
    pthread_cond_wait(&pcondd, &plock);    
    pthread_mutex_unlock(&plock);
    pthread_exit(NULL);

}


void* atc(void*){
  pthread_cond_wait(&pcondatc,&plock); //signal from the first plane 
  //cout << "Tower is awake!" << endl;
  while(time(0) <= init + simulationTime){
    if(emergencyq.size()>0){
      plane tmpP = emergencyq.front();
      time_t now = time(NULL);
      tmpP.runAwayTime = now - init;
      tmpP.arrivalTime -= init;
      simulationLog.addItem(tmpP);
      emergencyq.pop();
	    pthread_sleep(2);
    }else{
	    if((landingq.empty() && !(departingq.empty())) && (departingq.size()>5 || landingq.empty())){//starvation
        pthread_cond_signal(&pcondd);
        plane tmpP = departingq.front();
        time_t now = time(NULL);
        tmpP.runAwayTime = now - init;
        tmpP.arrivalTime -= init;
        if(tmpP.planeID != 0)
          simulationLog.addItem(tmpP);
        departingq.pop();
	      pthread_sleep(2);
	    }else{
        pthread_cond_signal(&pcondl);
        plane tmpP = landingq.front();
        time_t now = time(NULL);
        tmpP.runAwayTime = now - init;
        tmpP.arrivalTime -= init;
        if(tmpP.planeID)
          simulationLog.addItem(tmpP);
        landingq.pop(); 
        pthread_sleep(2);
	    }
    }
    pthread_mutex_unlock(&plock);	
   }
   pthread_exit(NULL);
}

int main(int argc, char *argv[]){

  pthread_mutex_init(&plock, NULL);
  pthread_cond_init(&pcondatc, NULL);
  pthread_cond_init(&pcondd, NULL);
  pthread_cond_init(&pcondl, NULL);

  init=time(0);

  simulationTime=60;
  prob=0.5;

  for(int i=1;i<argc;i++){
     if(strcmp(argv[i],"-s")==0){
       simulationTime=atoi(argv[i+1]);
       cout << "Simulation Time: " << simulationTime << endl;
     }else if(strcmp(argv[i],"-p")==0){
       prob=stod(argv[i+1]); 
	cout << "Probability is: " << prob << endl; 
     }
  }

  pthread_t atcthread;
  pthread_t landingthread;
  pthread_t departingthread;
  
  pthread_mutex_lock(&plock);
  pthread_create(&atcthread, NULL, atc, (void*) NULL);

  while(time(0) < init + simulationTime){

    srand(time(NULL));
    double random = rand() % 100;
    double r= random/100;

    if(((time(0)-init)>39) && ((time(0)-init)%40==0)){ //emergency landing
      isEmergency=true;
      pthread_create(&landingthread, NULL ,landing, (void*) NULL);

    }else{
      isEmergency=false;
      if(r <= prob){ //landing
       pthread_create(&landingthread, NULL ,landing, (void*) NULL);
    }
      if(r <= (1-prob)){ //departing
       pthread_create(&departingthread, NULL ,departing, (void*) NULL);
      }

    }
    printSnapShot(time(0) - init);
    pthread_sleep(1);

  } 
  simulationLog.saveLogFile();
}

 /****************************************************************************** 
  pthread_sleep takes an integer number of seconds to pause the current thread 
  original by Yingwu Zhu
  updated by Muhammed Nufail Farooqi
  *****************************************************************************/
int pthread_sleep (int seconds)
{
   pthread_mutex_t mutex;
   pthread_cond_t conditionvar;
   struct timespec timetoexpire;
   if(pthread_mutex_init(&mutex,NULL))
    {
      return -1;
    }
   if(pthread_cond_init(&conditionvar,NULL))
    {
      return -1;
    }
   struct timeval tp;
   //When to expire is an absolute time, so get the current time and add //it to our delay time
   gettimeofday(&tp, NULL);
   timetoexpire.tv_sec = tp.tv_sec + seconds; timetoexpire.tv_nsec = tp.tv_usec * 1000;

   pthread_mutex_lock (&mutex);
   int res =  pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
   pthread_mutex_unlock (&mutex);
   pthread_mutex_destroy(&mutex);
   pthread_cond_destroy(&conditionvar);

   //Upon successful completion, a value of zero shall be returned
   return res;

}


