#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include<math.h>

#define MAXLINE 127
int display_menu();
void kill_ps(int, char*,int);
void detail_ps(struct sockaddr_in, int);
void hardware_info(struct sockaddr_in, int);
void get_history();
void program_exit();
void get_file(struct sockaddr_in, int,char (*)[20]);
void read_file(char *);

int main(int argc, char * argv[]){
	struct sockaddr_in servaddr;
	int s, nbyte;
    int kill_pid;
	char filename[20],kill_ps_name[30];
	int  filenamesize;
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
                printf("Enter pid or process name to kill : ");
		        scanf("%s",kill_ps_name);
                if((unsigned long)log10(atoi(kill_ps_name)) + 1 == strlen(kill_ps_name)){
                    //if input was pure integer input
                    kill_pid = atoi(kill_ps_name);
                    kill_ps(kill_pid,NULL,s);//kill by pid
                }
                else{
                    kill_ps(-1,kill_ps_name,s);//kill by name
                }
    		    break;
	    	case 2:
		        detail_ps(servaddr,s);
		        break;
		    case 3:
		        hardware_info(servaddr,s);
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
        printf("press any key to continue...");
        getchar();
        getchar();
    }
}

int display_menu(void) {
	int menu = 0;
	char input[100] = { '\0' };
	while (1) {
		system("clear");
		printf("\n\t\t\t\t%s", "Remote Ps");
		printf("\n\t\t\t=================================================");
		printf("\n\t\t\t\t%16s\n", "PS MENU");
		printf("\n\t\t\t=================================================");
		printf("\n\t\t\t=\t1) %-25s\t\t=", "kill process");
		printf("\n\t\t\t=\t2) %-25s\t\t=", "Show detail ps");
		printf("\n\t\t\t=\t3) %-25s\t\t=", "Show hardware info");
		printf("\n\t\t\t=\t4) %-25s\t\t=", "Show process use history");
		printf("\n\t\t\t=\t5) %-25s\t\t=", "exit");
		printf("\n\t\t\t=================================================");
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

void kill_ps (int index, char *psname,int s){
	// kill process by pid or psname
    if(!psname){//pid가 아니라 psname으로 할려고 할려면ㄴ
        //psname 에서 pid 찾으려면 따로 작업 해야지
    }
    else{
        //psname을 구하는 코드
    }
    send(s,&index,sizeof(index),0);
    printf("killed process in remote machine. pid : %d, psname : %s",index,psname);
}

void detail_ps(struct sockaddr_in servaddr,int s){
	// by top command, get deatil info
    char filename[20];
    get_file(servaddr, s,&filename);
}

void hardware_info(struct sockaddr_in servaddr,int s){
	// by lshw command, get hardware info
	// and display resource use with progress bar
    int isSudo;
    char filename[20];
    recv(s,&isSudo,sizeof(int),0);
    get_file(servaddr, s,&filename);
}

void get_history(){
	// get result of top command by file (by 1s), and find most used program,
	// most resource reused program by multithreading
}
void get_file(struct sockaddr_in servaddr,int s,char (*filename)[20]){
    char server_ip[20],buf[MAXLINE+1];
    int filesize,fp,total = 0,sread;
    inet_ntop(AF_INET, & servaddr.sin_addr.s_addr, server_ip, sizeof(server_ip));
    printf("IP : %s ", server_ip);
    printf("Port : %x ", ntohs(servaddr.sin_port));

    recv(s, *filename, 20, 0);
    printf("%s ", *filename);

    // recv( accp_sock, &filesize, sizeof(filesize), 0 );
    read(s, & filesize, sizeof(filesize));
    printf("%d ", filesize);

    fp = open(*filename, O_WRONLY | O_CREAT | O_TRUNC);

    printf("file is receiving now.. \n");
    while (total != filesize) {
      sread = recv(s, buf, 100, 0);
      total += sread;
      buf[sread] = 0;
      write(fp, buf, sread);
      bzero(buf, sizeof(buf));
      printf("processing : %4.2f\r%%", total * 100 / (float) filesize);
    }
    puts("");
}
void read_file(char * filename){
    char temp[20];//임시 구현
    sprintf(temp,"cat %s",filename);
    system(temp);
}
