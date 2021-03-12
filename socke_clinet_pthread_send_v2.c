#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<netinet/tcp.h>

#define MAXLINE         4096
#define VNET_NUM        100
#define VNET_SN         11350000
#define READ_TIMEOUT    5000

#define DATA    "POST /v1/project/api_path/apiname HTTP/1.1\r\n" \
                "User-Agent: VNet\r\n" \
                "Accept: */*\r\n" \
                "Host: %s:%d\r\n" \
                "Content-txt: %d\r\n" \
                "Content-Type: application/json\r\n" \
                "Connection: keep-alive\r\n" \
                "Content-Length: 263\r\n" \
                "\r\n" \
                "{\"token\":\"token_key\"," \
                "\"project_id\":\"%04d\",\"project_sn\":\"%d\",\"data_time\":" \
                "\"%s\",\"temperature\":\"41\",\"keyG_result\":" \
                "\"1\",\"keyB_result\":\"1\",\"keyS_result\":\"1\"," \
                "\"keyE_result\":\"1\",\"result\":\"1\",\"keyJ_result\":\"1\"," \
                "\"keyL_result\":\"1\"}\r\n"

typedef struct {
    char service_ip[16];
    int service_port;
    int vnet_sn;
} Vnet_Info_t;

char* getNowtime(void) 
{
    static char s[30]={0};
    char YMD[15] = {0};
    char HMS[10] = {0};
    time_t current_time;
    struct tm* now_time;

    char *cur_time = (char *)malloc(21*sizeof(char));
    time(&current_time);
    now_time = localtime(&current_time);

    strftime(YMD, sizeof(YMD), "%F ", now_time);
    strftime(HMS, sizeof(HMS), "%T", now_time);
    
    strncat(cur_time, YMD, 11);
    strncat(cur_time, HMS, 8);

    //printf("\nCurrent time: %s\n\n", cur_time);
    memcpy(s, cur_time, strlen(cur_time)+1);
    free(cur_time);

    cur_time = NULL;

    return s;
}

int Read(int fd, char *buff,int len,int ms){

    fd_set rfds;
    struct timeval tv;
    int ret;

    if(fd < 0){
        printf("*****fd is wrong in readdata():fd = %d******\r\n",fd);
        return -3;
    }

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    tv.tv_sec = ms/1000;
    tv.tv_usec = (ms%1000)*1000;
    ret = select(fd + 1, &rfds, NULL, NULL, &tv);
    if (ret > 0){
        if(FD_ISSET(fd, &rfds)){
            usleep(100000);
            ret = read(fd,buff,len);
            if(0 == ret){
                if (EINTR != errno) {
                    ret = -1;
                }
            }
        }
    }else if(ret==0){
        ret = -4;
    }
    return ret;
}

int socket_thread_func(void* arg)
{
    struct sockaddr_in  servaddr;
    int sockfd = -1;
    int syncnt = 1;
    char readdata[1024] = {0};
    char writedata[1024] = {0};
    int ret = 0;
    Vnet_Info_t *vnet = (Vnet_Info_t *)arg;

    while (1) {
        if(sockfd < 0) {
            if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
                printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
                break;
            }
            memset(&servaddr, 0, sizeof(servaddr));
            servaddr.sin_family = AF_INET;
            servaddr.sin_port = htons(vnet->service_port);
            if (inet_pton(AF_INET, vnet->service_ip, &servaddr.sin_addr) <= 0){
                printf("inet_pton error for %s\n",vnet->service_ip);
                break;
            }
        
            setsockopt(sockfd, IPPROTO_TCP, TCP_SYNCNT, &syncnt, sizeof(syncnt));
            if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
                printf("%d: connect error: %s(errno: %d)\n", vnet->vnet_sn, strerror(errno),errno);
                break;
            }
        }


        snprintf(writedata, sizeof(writedata), DATA, vnet->service_ip, vnet->service_port, 
                 vnet->vnet_sn, vnet->vnet_sn % 1000, vnet->vnet_sn, getNowtime());
#if 0
        printf("------------------------------------------\n");
        printf("writedata: \n%s\n", writedata);
        printf("------------------------------------------\n\n");
#endif

        ret = write(sockfd, writedata, strlen(writedata));
        if(ret < 0) {
            printf("write error!\n");
            break;
        }

        ret = Read(sockfd, readdata, sizeof(readdata), READ_TIMEOUT);

#if 0
        printf("------------------------------------------\n");
        printf("readdata: \n%s\n", readdata);
        printf("------------------------------------------\n\n");
#endif
        if(ret < 0) {
            printf("<%d> read error %s(errno: %d)!\n", ret, strerror(errno),errno);
            break;
        }

        sleep(1);
    }

    if(sockfd > 0) {
        close(sockfd);
    }
    sleep(1);

    printf("%d=>%s\n", vnet->vnet_sn, getNowtime());
    return 0;

}

int main(int argc, char** argv){
    Vnet_Info_t vnet[VNET_NUM];
    pthread_t tid[VNET_NUM];
    int i;
    

    if(argc != 3){
        printf("usage: ./client <ipaddress> <port>\n");
        return 0;
    }
    printf("start time: %s\n", getNowtime());
    for (i = 0; i < VNET_NUM; i++) {
        snprintf(vnet[i].service_ip, sizeof(vnet[i].service_ip), "%s", argv[1]);
        vnet[i].service_port = atoi(argv[2]);
        vnet[i].vnet_sn = VNET_SN + i + 1;
        pthread_create(&tid[i], NULL, (void*)socket_thread_func, (void *)&vnet[i]);
        usleep(100000);
    }

    for (i = 0; i < VNET_NUM; i++) {
        pthread_join(tid[i], NULL);
    }


    return 0;
}
