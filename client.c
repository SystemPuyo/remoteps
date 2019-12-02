#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<fcntl.h>
#include<arpa/inet.h>

#define MAXLINE 127

int main(int argc, char * argv[]){
	struct sockaddr_in servaddr;
	int s, nbyte;
	char filename[20];
	char buf[MAXLINE+1],server_ip[20];
	int filesize, fp, filenamesize;
	int sread, total = 0;

	if(argc !=3){
		printf("usage : %s ip_address port",argv[0]);
		exit(1);
	}

	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    		perror("socket fail");
    		exit(0);
  	}

  // 에코 서버의 소켓주소 구조체 작성
  	bzero((char * ) & servaddr, sizeof(servaddr));
  	servaddr.sin_family = AF_INET;
  	inet_pton(AF_INET, argv[1], & servaddr.sin_addr);
  	servaddr.sin_port = htons(atoi(argv[2]));

  // 연결요청
  	if (connect(s, (struct sockaddr * ) & servaddr, sizeof(servaddr)) < 0) {
   		 perror("connect fail");
    		exit(0);
  	}

  //연결 되었으면 수신 준비
    puts("서버와 연결됨..");

    inet_ntop(AF_INET, & servaddr.sin_addr.s_addr, server_ip, sizeof(server_ip));
    printf("IP : %s ", server_ip);
    printf("Port : %x ", ntohs(servaddr.sin_port));

    bzero(filename, 20);
    recv(s, filename, sizeof(filename), 0);
    printf("%s ", filename);

    // recv( accp_sock, &filesize, sizeof(filesize), 0 );
    read(s, & filesize, sizeof(filesize));
    printf("%d ", filesize);

    strcat(filename, "_backup");
    fp = open(filename, O_WRONLY | O_CREAT | O_TRUNC);

    while (total != filesize) {
      sread = recv(s, buf, 100, 0);
      printf("file is receiving now.. ");
      total += sread;
      buf[sread] = 0;
      write(fp, buf, sread);
      bzero(buf, sizeof(buf));
      printf("processing : %4.2f%% ", total * 100 / (float) filesize);
      usleep(1000);

    }

}
