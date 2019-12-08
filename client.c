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
int display_menu();
void kill_ps(int, char*);
void detail_ps();
void hardware_info();
void get_history();
void program_exit();

int main(int argc, char * argv[]){
	struct sockaddr_in servaddr;
	int s, nbyte;
	char filename[20];
	char buf[MAXLINE+1],server_ip[20];
	int filesize, fp, filenamesize;
	int sread, total = 0;
	int sel = 0;
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

while(1){
	sel = display_menu();

	//scanf("%d", &sel);
	send(s, &sel, sizeof(int), 0);
	//서버로 입력한 값을 보낸다.

	
	switch(sel){
		case 1:
		//kill_ps();
		break;
		case 2:
		//detail_ps();
		break;
		case 3:
		//hardware_info();
		break;
		case 4:
		//get_history();
		break;
		case 5:
		printf("프로그램을 종료하겠습니다.\n");
		return 0;
		default:
		fprintf(stderr, "input error\n");
		exit(1);
	}
}
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

    printf("file is receiving now.. \n");
    while (total != filesize) {
      sread = recv(s, buf, 100, 0);
      total += sread;
      buf[sread] = 0;
      write(fp, buf, sread);
      bzero(buf, sizeof(buf));
      printf("processing : %4.2f\r%%", total * 100 / (float) filesize);
      usleep(1000);

    }
	printf("\n");
}

int display_menu(void) {
	int menu = 0;
	char input[100] = { '\0' };
	while (1) {
		system("clear");
		printf("\n\t\t\t\t%s", "Remote Ps");
		printf("\n\t\t\t=========================================");
		printf("\n\t\t\t\t%16s\n", "PS MENU");
		printf("\n\t\t\t=========================================");
		printf("\n\t\t\t=\t1) %s\t\t=", "kill process");
		printf("\n\t\t\t=\t2) %s\t\t=", "Show detail ps");
		printf("\n\t\t\t=\t3) %s\t\t=", "Show hardware info");
		printf("\n\t\t\t=\t4) %s\t\t=", "Show process use history");
		printf("\n\t\t\t=\t5) %s\t\t=", "exit");
		printf("\n\t\t\t=========================================");
		printf("\n\t\t\t=> ");
		scanf("%s", input);
		menu = atoi(input); // to handle some wrong input
		if (strlen(input) != 1) { // some wrong input(not int) ex) abcd, 1abcd, 2!af43 ...
			continue;
		}
		if (menu < 1 || menu > 5) { // int input, but not in menu number
			continue;
		}
		else {
			return menu;
		}
	
	}
	return 0;
}

void kill_ps (int index, char *psname){
	// kill process by pid or psname
}

void detail_ps(){
	// by top command, get deatil info
}

void hardware_info(){
	// by lshw command, get hardware info
	// and display resource use with progress bar
}

void get_history(){
	// get result of top command by file (by 1s), and find most used program,
	// most resource reused program by multithreading
}
