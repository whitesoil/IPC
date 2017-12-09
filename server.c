#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/sem.h>

#define PERM 0644
#define MAXLINE 100



struct sembuf lock_op[1] = {{0, -4, SEM_UNDO}};
struct sembuf unlock_op[1] = {{0, 4, 0}};
struct sembuf get_lock_op[1] = {{0,-1,SEM_UNDO}};
struct sembuf get_unlock_op[1] = {{0,1,0}};

int semid;


// union semun{
//   int val;
//   struct semid_ds *buf;
//   ushort*array;
// };

union semun arg;

void lock() {
  semop(semid, lock_op, 1);
}

void unlock() {
  semop(semid, unlock_op, 1);
}

void get_lock() {
  semop(semid,get_lock_op, 1);
}

void get_unlock() {
  semop(semid, get_unlock_op, 1);
}

int put_msgq, del_msgq,mod0_msgq,mod1_msgq,mod2_msgq,mod3_msgq,recv_msgq;

typedef struct data{
  long mtype;
  int key;
  char value[MAXLINE];
}data;

data lru_list[8] = {};

void remake_list(data insert,int index){
  for(int k = index;k<7;k++){
    lru_list[k] = lru_list[k+1];
  }
  lru_list[7] = insert;
}

int check_cache(data insert){
   for(int i = 0;i<8;i++){
     if(lru_list[i].key == insert.key){
       return i;
     }
   }
   return -1;
}

char* hashTable [4][11] = {};

void *put_recv(){
  struct data msgBuf;
  int row,col;
  int temp;
  while(1){
    msgrcv(put_msgq,&msgBuf,sizeof(data)-sizeof(long),1,0);
    lock();
    if((temp = check_cache(msgBuf)) != -1){
      msgBuf.mtype = 4;
      lru_list[temp] = msgBuf;
    }
    row = msgBuf.key%4;
    col = msgBuf.key/4;
    hashTable[row][col] = msgBuf.value;
    unlock();
  }
}

void *del_recv(){
  data msgBuf;
  int row,col;
  int temp;
  data garbage = {0,0,"null"};
  while(1){
    msgrcv(del_msgq,&msgBuf,sizeof(data)-sizeof(long),2,0);
    lock();
    if((temp = check_cache(msgBuf)) != -1){
      lru_list[temp] = garbage;
    }
    row = msgBuf.key%4;
    col = msgBuf.key/4;
    hashTable[row][col] = NULL;
    unlock();
  }
}

void *mod0_recv(){
  data msgBuf;
  data tempbuf;
  int row,col;
  int temp;
  while(1){

    msgrcv(mod0_msgq,&msgBuf,sizeof(data)-sizeof(long),3,0);
    get_lock();
    if((temp = check_cache(msgBuf))!=-1){
      msgsnd(recv_msgq,&lru_list[temp],sizeof(data)-sizeof(long),0);
      remake_list(lru_list[temp],temp);
    }else{
      row = msgBuf.key%4;
      col = msgBuf.key/4;
      tempbuf.mtype = 4;
      tempbuf.key = msgBuf.key;
      memcpy((void*)tempbuf.value,(void*)hashTable[row][col],MAXLINE);
      msgsnd(recv_msgq,&tempbuf,sizeof(data)-sizeof(long),0);
      remake_list(tempbuf,0);
    }
    get_unlock();
  }
}

void *mod1_recv(){
   data msgBuf;
   data tempbuf;
  int row,col;
  int temp;
  while(1){

    msgrcv(mod1_msgq,&msgBuf,sizeof(data)-sizeof(long),3,0);
    get_lock();
    if((temp = check_cache(msgBuf))!=-1){
      msgsnd(recv_msgq,&lru_list[temp],sizeof(data)-sizeof(long),0);
      remake_list(lru_list[temp],temp);
    }else{
      row = msgBuf.key%4;
      col = msgBuf.key/4;
      tempbuf.mtype = 4;
      tempbuf.key = msgBuf.key;
      memcpy((void*)tempbuf.value,(void*)hashTable[row][col],MAXLINE);
      msgsnd(recv_msgq,&tempbuf,sizeof(data)-sizeof(long),0);
      remake_list(tempbuf,0);
    }
    get_unlock();
  }
}

void *mod2_recv(){
   data msgBuf;
   data tempbuf;
  int row,col;
  int temp;
  while(1){

    msgrcv(mod2_msgq,&msgBuf,sizeof(data)-sizeof(long),3,0);
    get_lock();
    if((temp = check_cache(msgBuf))!=-1){
      msgsnd(recv_msgq,&lru_list[temp],sizeof(data)-sizeof(long),0);
      remake_list(lru_list[temp],temp);
    }else{
      row = msgBuf.key%4;
      col = msgBuf.key/4;
      tempbuf.mtype = 4;
      tempbuf.key = msgBuf.key;
      memcpy((void*)tempbuf.value,(void*)hashTable[row][col],MAXLINE);
      msgsnd(recv_msgq,&tempbuf,sizeof(data)-sizeof(long),0);
      remake_list(tempbuf,0);
    }
    get_unlock();
  }
}

void *mod3_recv(){
   data msgBuf;
   data tempbuf;
  int row,col;
  int temp;
  while(1){

    msgrcv(mod3_msgq,&msgBuf,sizeof(data)-sizeof(long),3,0);
    get_lock();
    if((temp = check_cache(msgBuf))!=-1){
      msgsnd(recv_msgq,&lru_list[temp],sizeof(data)-sizeof(long),0);
      remake_list(lru_list[temp],temp);
    }else{
      row = msgBuf.key%4;
      col = msgBuf.key/4;
      tempbuf.mtype = 4;
      tempbuf.key = msgBuf.key;
      memcpy((void*)tempbuf.value,(void*)hashTable[row][col],MAXLINE);
      msgsnd(recv_msgq,&tempbuf,sizeof(data)-sizeof(long),0);
      remake_list(tempbuf,0);
    }
    get_unlock();
  }
}


int main(int argc, char **argv){
  int err;
  int i;
  int thread_num = 6;
  int cpu_num = 4;

  for(int j =0;j<4;j++){
    for(int k=0;k<11;k++){
      hashTable[j][k] = "null";
    }
  }

  if((semid = semget(IPC_PRIVATE,1,PERM|IPC_CREAT|IPC_EXCL))<0){
    perror("semget error");
    exit(1);
  }
  arg.val = 4;
  semctl(semid,0,SETVAL,arg);


  if((put_msgq = msgget(0001,PERM|IPC_CREAT))<0){
    perror("put message queue : ");
    exit(1);
  }
  if((del_msgq = msgget(0002,PERM|IPC_CREAT))<0){
    perror("del message queue : ");
    exit(1);
  }
  if((mod0_msgq = msgget(0003,PERM|IPC_CREAT))<0){
    perror("mod0 message queue : ");
    exit(1);
  }
  if((mod1_msgq = msgget(0004,PERM|IPC_CREAT))<0){
    perror("mod1 message queue : ");
    exit(1);
  }
  if((mod2_msgq = msgget(0005,PERM|IPC_CREAT))<0){
    perror("mod2 message queue : ");
    exit(1);
  }
  if((mod3_msgq = msgget(0006,PERM|IPC_CREAT))<0){
    perror("mod3 message queue : ");
    exit(1);
  }
  if((recv_msgq = msgget(0007,PERM|IPC_CREAT))<0){
    perror("recv message queue : ");
    exit(1);
  }


  pthread_t *tidp = (pthread_t*)malloc(sizeof(pthread_t)*thread_num);

  int numbering[6] = {0,1,2,3,4,5};

  if((err = pthread_create(&(tidp[0]),NULL,put_recv,(void*)&numbering[0]))<0){
    perror("thread create error : ");
    exit(1);
  }
  if((err = pthread_create(&(tidp[1]),NULL,del_recv,(void*)&numbering[1]))<0){
    perror("thread create error : ");
    exit(1);
  }
  if((err = pthread_create(&(tidp[2]),NULL,mod0_recv,(void*)&numbering[2]))<0){
    perror("thread create error : ");
    exit(1);
  }
  if((err = pthread_create(&(tidp[3]),NULL,mod1_recv,(void*)&numbering[3]))<0){
    perror("thread create error : ");
    exit(1);
  }
  if((err = pthread_create(&(tidp[4]),NULL,mod2_recv,(void*)&numbering[4]))<0){
    perror("thread create error : ");
    exit(1);
  }
  if((err = pthread_create(&(tidp[5]),NULL,mod3_recv,(void*)&numbering[5]))<0){
    perror("thread create error : ");
    exit(1);
  }

  for(i=0;i<thread_num;i++){
    pthread_join(tidp[i],NULL);
    printf("\n%d end\n",i);
  }

  return 0;




}
