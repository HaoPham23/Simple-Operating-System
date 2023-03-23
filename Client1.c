#include<pthread.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#define KEYSEND 123
#define KEYRECEIVE 456
struct mess_buf{
    long cnt;
    char mess[100];
};
void* SendMess(void*p){

}

void* ReceiveMess(){

}
int main()
{
    pthread_t send, receive;
    pthread_create(&send,0,SendMess,0);
    pthread_create(&receive,0,ReceiveMess,0);
    pthread_join(send,0);
    pthread_join(receive,0);
    return 0;

}