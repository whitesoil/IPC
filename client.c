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

#define PERM 0644
#define MAXLINE 100

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int put_msgq,del_msgq,mod0_msgq,mod1_msgq,mod2_msgq,mod3_msgq,recv_msgq;

typedef struct data{
  long mtype;
  int key;
  char value[MAXLINE];
}data;


void *send_thread(void* parm){
  int* x;
  int select;
  int k = *(int*)parm;
  data msgBuf;

  while(1){
    pthread_mutex_lock(&mutex);
    printf("\n%d thread\n",k);
    printf("Selet Operation\n1.Put\n2.Delete\n3.Get\nselect : ");
    scanf("%d",&select);
    getchar();

    if(select == 1){
      int key;
      char val[MAXLINE];

      printf("key,value : ");
      scanf("%d,%s",&key,val);
      getchar();

      msgBuf.mtype = 1;
      msgBuf.key = key;
      memcpy((void*)msgBuf.value,(void*)val,MAXLINE);

      msgsnd(put_msgq,&msgBuf,sizeof(data)-sizeof(long),0);

    }
    else if(select == 2){
      int key;

      printf("Delete key : ");
      scanf("%d",&key);
      getchar();

      msgBuf.mtype = 2;
      msgBuf.key = key;

      msgsnd(del_msgq,&msgBuf,sizeof(data)-sizeof(long),0);
    }
    else if(select == 3){
      int key;
      data buf;


      printf("Get key : ");
      scanf("%d",&key);
      getchar();

      int temp = key%4;
      msgBuf.mtype = 3;
      msgBuf.key = key;




      if(temp == 0){
	msgsnd(mod0_msgq,&msgBuf,sizeof(data)-sizeof(long),0);
      }else if(temp == 1){
	msgsnd(mod1_msgq,&msgBuf,sizeof(data)-sizeof(long),0);
      }else if(temp == 2){
	msgsnd(mod2_msgq,&msgBuf,sizeof(data)-sizeof(long),0);
      }else if(temp == 3){
	msgsnd(mod3_msgq,&msgBuf,sizeof(data)-sizeof(long),0);
      }else{

      }


    }
    else{

    }

        pthread_mutex_unlock(&mutex);

    sleep(5);
   }
  msgctl(put_msgq, IPC_RMID, NULL);
  msgctl(del_msgq, IPC_RMID, NULL);
  msgctl(mod0_msgq, IPC_RMID, NULL);
  msgctl(mod1_msgq, IPC_RMID, NULL);
  msgctl(mod2_msgq, IPC_RMID, NULL);
  msgctl(mod3_msgq, IPC_RMID, NULL);
  return (void*)x;
}

void *recv_thread(){
  data msg;
  int k;
  while(1){
    msgrcv(recv_msgq,&msg,sizeof(data)-sizeof(long),4,0);
    printf("\nresponse : key = %d, value = %s\nselect : ",msg.key,msg.value);
  }
}



int main(int argc, char **argv){
  int err;
  int i;
  int thread_num = 4;
  int cpu_num = 4;

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

  int numbering[10] = {0,1,2,3,4};

  for(i =0;i<thread_num;i++){
    if((err = pthread_create(&(tidp[i]),NULL,send_thread,(void*)&numbering[i]))<0){
      perror("thread create error : ");
      exit(1);
    }
  }
  if((err = pthread_create(&(tidp[i]),NULL,recv_thread,(void*)&numbering[i]))<0){
    perror("thread create error : ");
    exit(1);
  }

  for(i=0;i<thread_num;i++){
    pthread_join(tidp[i],NULL);
    printf("\n%d end\n",i);
  }

  return 0;


}
