#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <time.h>
#include <pwd.h>
#include<sys/stat.h>
#include<stdbool.h>

#define MAXLINE 127

int init(struct sockaddr_in servaddr, char * port, int listen_sock, struct sockaddr_in cliaddr);

void red() {
	printf("\033[1;31m");
}

void yellow() {
	printf("\033[1;33m");
}

void purple() {
	printf("\033[0;35m");
}
void green() {
	printf("\033[0;32m");
}
void blue() {
	printf("\033[0;34m");
}
void reset() {
	printf("\033[0m");
}

int main(int argc, char * argv[]) {
    struct sockaddr_in servaddr, cliaddr;
    int listen_sock, accp_sock,
        nbyte, nbuf;
    char buf[MAXLINE + 1];
    char filename[20];
    int filesize = 0, filenamesize;
    int total = 0, sread, fp;
    int sel;
    char systemarg[100];
    int topcnt = 0,euid;
    struct stat tmp_stat;
    bool hasSent = false;
    uid_t my_uid = getuid();
    if (argc != 2) {
        printf("usage: %s port ", argv[0]);
        exit(0);
    }
    // 소켓 생성
    // servaddr을 ''으로 초기화
    euid = geteuid();
    if(euid != 0 ){
        red();
        puts("현재 일반 유저로 서버 프로그램을 실행하고 있습니다.");
        puts("sudo 로 실행 하면 더 자세한 lshw를 볼 수 있습니다.");
        reset();
    }
    accp_sock = init(servaddr,argv[1],listen_sock,cliaddr);
    send(accp_sock, & my_uid, sizeof(my_uid), 0); 
    while (1) {
        printf("명령어를 기다리고 있습니다.\n");
        read(accp_sock, & sel, sizeof(int));
        printf("got %d\n",sel);
        //여기는 클라이언트에서 입력을 넘기면 그 입력 된 숫자를 받음 
        switch (sel) {
        case 21://ps 만들어서 클라로 보내주기
            sprintf(filename,"ps%ld.txt",time(NULL));
            sprintf(systemarg, "ps -fa > %s",filename);
            system(systemarg);
            break;
        case 1:
        case 22: //top 만들어서 클라로 보내주기
            sprintf(filename, "top%ld.txt", time(NULL));
            sprintf(systemarg, "top -b -n 1 -o PID > %s", filename);
            system(systemarg);
            break;
        case 3: //lshw 명령어 결과를 클라로 보내주기
            if(hasSent) continue;
            strcpy(filename,"lshw.txt");
            sprintf(systemarg,"lshw -short > %s",filename);
            send(accp_sock,&euid,sizeof(euid),0);//euid를 보내서 정보를 알림
            system(systemarg);
            hasSent = true;
            break;
        case 4:
        case 5:
            continue;//이건 클라 혼자서 하는 것이니 여기서는 그냥 continue 하자구?
            break;

        case -1:
            system("rm lshw.txt");
            for(int i = 0;i<topcnt;i++){
                sprintf(systemarg,"rm top%d.txt",i);
                system(systemarg);
            }//clear exit
	    printf("클라이언트가 종료 요청을 보냈습니다.\n종료합니다.\n");
            exit(0);//임시방편 . 소켓 닫는것좀 구현해줘
           // close(accp_sock);
           //accp_sock = init(servaddr,argv[1],listen_sock,cliaddr);
            continue;
        }
        filenamesize = strlen(filename);

        if ((fp = open(filename, O_RDONLY)) < 0) {
            printf("open failed");
            exit(0);
        }
        send(accp_sock, filename, sizeof(filename), 0); //파일의 이름과
        
        fstat(fp,&tmp_stat);
        filesize = tmp_stat.st_size;
        printf("filename : %s filesize : %d",filename,filesize);
        send(accp_sock, & filesize, sizeof(filesize), 0); //파일의 사이즈를 전송
        lseek(fp, 0, SEEK_SET);
        printf("file is sending now.. \n");
        while (total != filesize) {
            sread = read(fp, buf, 100);
            total += sread;
            buf[sread] = 0;
            write(accp_sock, buf, sread);
            printf("processing :%4.2f%%\r", total * 100 / (float) filesize);
        }
        printf("\n");
        total = 0;
        if(sel == 1){
            recv(accp_sock,&sel,sizeof(int),0);
            if(sel == -1){
                continue;
            }
            sprintf(systemarg,"kill -9 %d",sel);//pid 받아서 그걸로 ps kill함
            system(systemarg);

        }
        if(sel !=3)
            read(accp_sock, & sel, sizeof(int));
        //do something later
    }

    printf("접속을 종료합니다.\n");
    return 0;
}

int init(struct sockaddr_in servaddr, char * port, int listen_sock, struct sockaddr_in cliaddr) {
    int accp_sock, addrlen = sizeof(cliaddr);
    char cli_ip[20];
    if ((listen_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fail");
        exit(0);
    }
    bzero((char * ) & servaddr, sizeof(servaddr));
    // servaddr 세팅
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(port));

    // bind() 호출
    if (bind(listen_sock, (struct sockaddr * ) & servaddr,
            sizeof(servaddr)) < 0) {
        perror("bind fail");
        exit(0);
    }
    // 소켓을 수동 대기모드로 세팅
    listen(listen_sock, 5);

    puts("서버가 연결요청을 기다림..");
    // 연결요청을 기다림
    accp_sock = accept(listen_sock,
        (struct sockaddr * ) & cliaddr, & addrlen);

    if (accp_sock < 0) {
        perror("accept fail");
        exit(0);
    }

    puts("클라이언트가 연결됨..");
    inet_ntop(AF_INET, & cliaddr.sin_addr.s_addr, cli_ip, sizeof(cli_ip));
    printf("IP : %s ", cli_ip);
    printf("Port : %d \n", ntohs(cliaddr.sin_port));


    return accp_sock;
}
