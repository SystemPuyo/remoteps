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
#include<stdbool.h>
#include<time.h>
#include <ctype.h>

#define MAXLINE 127
struct hw_info{
	char class[100];
	char description[300];
};

struct PROCESS_INFO{
	int pid;
	char user[20];
	float cpu_pct;
	float mem_pct;
	char run_time[20];
	char command[30];
};	//top의 결과를 담을 구조체

struct PS_INFO{
	int pid;
	char user[20];
	char run_time[20];
	char command[100];	//top이랑 다르게 나온다 
};	//ps의 결과를 담을 구조체

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


int display_menu();
void kill_ps(int, char *, int);
void get_ps(struct sockaddr_in, int);
void hardware_info(struct sockaddr_in, int, struct hw_info[], int *);
void history_analysis();
void program_exit();
void get_file(struct sockaddr_in, int,char (*)[20]);
void read_file(char *);
void show_file_list();
void set_file_list();
void print_hw_info(struct hw_info[], int);
int submenu_2(struct sockaddr_in, int);
struct PROCESS_INFO* make_top_info(char[]);
struct PS_INFO* 	 make_ps_info(char[]);

int file_amt = 0;
bool isFileSimple[40];
time_t timeList[40];
bool isListSet = false;
struct PROCESS_INFO process[10000];

int main(int argc, char * argv[]){
	struct hw_info h[100] = {};
	struct sockaddr_in servaddr;
	int s;
    char filename[30];
    int kill_pid;
	char kill_ps_name[30];
	int sel = 0;
	int hw_size = 0;
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
        if(sel != 2)
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
                sel = submenu_2(servaddr,s);
                if(sel == -1) continue;
                send(s,&sel,sizeof(int),0);
		        break;
		    case 3:
			    hardware_info(servaddr, s, h, &hw_size);
		        break;
	        case 4:
		        history_analysis();
		        break;
            case 5:
                show_file_list();
                printf("select file to see : ");
                scanf("%d",&sel);
                if(sel > file_amt){
                    printf("the selected file does not exist");
                    break;
                }
                sprintf(filename,"%s%ld.txt",isFileSimple[sel]?"ps":"top",timeList[sel]);
                read_file(filename);
                break;
		    case -1:
		        printf("프로그램을 종료하겠습니다.\n");
                return 0;
		    default:
		        fprintf(stderr, "input error\n");
		        exit(1);
	    }
	printf("================================================================\n");
        printf("press any key to continue...\n");
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
		printf("\n\t\t\t=\t2) %-25s\t\t=", "Get ps");
		printf("\n\t\t\t=\t3) %-25s\t\t=", "Show hardware info");
		printf("\n\t\t\t=\t4) %-25s\t\t=", "Compare two process use histories");
		printf("\n\t\t\t-\t5) %-25s\t\t=", "Show received top file list");
        printf("\n\t\t\t=\tq) %-25s\t\t=", "exit");
		printf("\n\t\t\t=================================================");
		printf("\n\t\t\t=> ");
		scanf("%s", input);
		
        if(strcmp(input,"q") == 0 || strcmp(input,"Q") == 0)
            return -1;
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
    printf("killed process in remote machine. pid : %d, psname : %s\n",index,psname);
}


void get_ps(struct sockaddr_in servaddr, int s){
	// by top command, get deatil info
    char filename[20];
    get_file(servaddr, s,&filename);
    file_amt++;
}

void hardware_info(struct sockaddr_in servaddr, int s, struct hw_info h[], int *count){
	// by lshw command, get hardware info
	// and display resource use with progress bar
    static bool hasInfo = false; 
    FILE *fp = NULL;
    if(hasInfo){
        print_hw_info(h, *count);
        return;
    }
	
    int isSudo;
    char filename[20];
    char str[300];
    struct hw_info temp;
    int i, j;
    int cnt = 0;
    recv(s,&isSudo,sizeof(int),0);
    get_file(servaddr, s, &filename);
    fp = fopen(filename, "r");
    fscanf(fp, "%s", str);
    while(!feof(fp)){
	if (cnt >= 99){
		break;
	}
    	if (!strcmp(str, "processor")) {
		strcpy(h[cnt].class, "processor");
		fgets(str, sizeof(str), fp);
		if (str[1] == ' '){
			strcpy(h[cnt].description, str);
			cnt++;
		}
		else{
			memset(h[cnt].class, '\0', sizeof(h[cnt].class));
		}
	}
	if (!strcmp(str, "memory")) {
		strcpy(h[cnt].class, "memory");
		fgets(str, sizeof(str), fp);
		if (str[1] == ' '){
			strcpy(h[cnt].description, str);
			cnt++;
		}
		else{
			memset(h[cnt].class, '\0', sizeof(h[cnt].class));
		}
	}
	if (!strcmp(str, "display")) {
		strcpy(h[cnt].class, "display");
		fgets(str, sizeof(str), fp);
		if (str[1] == ' '){
			strcpy(h[cnt].description, str);
			cnt++;
		}
		else {
			memset(h[cnt].class, '\0', sizeof(h[cnt].class));
		}
	}
	fscanf(fp, "%s", str);

    }
    for (i = 0; i < cnt - 1; i++){
    	for (j = 0; j < cnt - 1 - i; j++){
		if (strcmp(h[j].class, h[j + 1].class) > 0){
			temp = h[j];
			h[j] = h[j + 1];
			h[j + 1] = temp;
		}
	}
    }
    *count = cnt;
    hasInfo = true;
}

void history_analysis(){
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
    struct PS_INFO *ps_info;
    struct PROCESS_INFO *top_info;
    if(filename[0] == 't'){//reading the top result
        top_info = make_top_info(filename);
        for(int i = 0;top_info[i].pid != -1;i++){
            //print blah
            if(strcmp(top_info[i].user,"root") == 0){
                red();
            }
        
            printf("%d %s %.1f %.1f %s %s\n", top_info[i].pid, top_info[i].user,
                    top_info[i].cpu_pct, top_info[i].mem_pct, top_info[i].run_time, top_info[i].command);

            reset();
        }
    }
    else{//reading ps result
        ps_info = make_ps_info(filename);
        for(int i = 0;ps_info[i].pid != -1;i++){
            if(strcmp(ps_info[i].user,"root") == 0){
                red();
            }
            printf("%s %d %s %s\n", ps_info[i].user, ps_info[i].pid, ps_info[i].run_time, ps_info[i].command);
            reset();
        }
    }
}

void show_file_list(){
    char * temp;
    set_file_list();
    printf("받아온 파일들의 리스트\n");
    for(int i = 0;i<file_amt;i++){
        printf("%d) ",i+1);
        temp = ctime(&timeList[i]);
        temp[strlen(temp)-1] = '\0';
        printf("%s",temp);
        if(!isFileSimple[i]){
            printf(" -detailed");
        }
        puts("");
    }
}

void set_file_list(){
    FILE * fp;
    char line[20];
    if(isListSet)return;
    system("ls *.txt |sort| grep -E 'top*|ps'> filelist.txt");
    fp = fopen("filelist.txt","r");
    while(EOF!= fscanf(fp,"%s",line)){
        if(line[0] == 't'){//if the line is about top
            sscanf(line,"top%ld.txt",&timeList[file_amt]);
            isFileSimple[file_amt] = false;
        }
        if(line[0] == 'p'){//else the line is about ps
            sscanf(line,"ps%ld.txt",&timeList[file_amt]);
            isFileSimple[file_amt] = true;
        }
        file_amt++;
    }
    isListSet = true;
    fclose(fp);
}

void print_hw_info(struct hw_info h[], int size){
	int i;
	int j;
	int temp;
	char prev[100] = {'\0', };
	printf("HW_INFO\n");
	printf("================================================================\n");
	for (i = 0; i < size; i++){
		if (!strcmp(prev, h[i].class)){
			temp = (int)strlen(h[i].class);
			for (j = 0; j <= temp; j++){
				printf(" ");
			}	
			printf("%s\n", h[i].description);
		}
		else{
			printf("%s:%s\n", h[i].class, h[i].description);
		}
		strcpy(prev, h[i].class);
	}
}

int submenu_2( struct sockaddr_in servaddr,int s){
    int menu;
    char input[20],filename[20];
	struct PROCESS_INFO* info; 	//top
	struct PS_INFO* ps_info;	//ps
    system("clear");
    printf("\n\t\t\t\t%16s\n", "Get ps MENU");
	printf("\n\t\t\t=================================================");
	printf("\n\t\t\t=\t1) %-25s\t\t=", "Get simple ps");
	printf("\n\t\t\t=\t2) %-25s\t\t=", "Get detail ps");
    printf("\n\t\t\t=\tq) %-25s\t\t=", "Go Back to main menu");
    printf("\n\t\t\t=================================================");
	printf("\n\t\t\t=> ");

    scanf("%s",input);
    if(strcmp(input,"q") == 0 || strcmp(input,"Q") == 0){
        return -1;
    }
    menu = atoi(input);
    menu += 20; 

    send(s, &menu, sizeof(int), 0);
    get_file(servaddr,s,&filename);

	if(menu == 21){
		ps_info = make_ps_info(filename);
	}

	if(menu == 22){
		info = make_top_info(filename);
	}

/*
	for (int i = 0; i < 1000; i++)
	{
		if(ps_info[i].pid == 0)
			break;
		printf("%s %d %s %s\n", ps_info[i].user, ps_info[i].pid, ps_info[i].run_time, ps_info[i].command);
	}
*///확인용

/*
	for (int i = 0; i < 1000; i++)
	{
		if(info[i].pid == 0)
			break;
		printf("%d %s %.1f %.1f %s %s\n", info[i].pid, info[i].user, info[i].cpu_pct, info[i].mem_pct, info[i].run_time, info[i].command);
	}
*///확인용	

    isListSet = false;
    return menu;
}

struct PROCESS_INFO* make_top_info(char filename[20]){
	char line[120];
	int pr, ni, virt, res, shr;
	char s;
	int cur = 0;
	struct PROCESS_INFO* temp = (struct PROCESS_INFO*)malloc(sizeof(struct PROCESS_INFO)*1000);

	for (int i = 0; i < 1000; i++)
	{
		temp[i].pid = -1;
	}	//초기화
	

	FILE* fp = fopen(filename, "r");	
	if(fp == NULL){
		fprintf(stderr, "파일 열기 에러");
		exit(1);
	}

	while(1){
		if( fgets(line, 120, fp) == NULL)
			break;

			if(isdigit(line[4]) != 0){ //%5d로 되어 있어서 5번째 칸이 숫자인지 확인
				sscanf(line, "%d %s %d %d %d %d %d %c %f %f %s %s", &(temp[cur].pid), temp[cur].user, &pr, &ni, &virt, &res, &shr, &s, &(temp[cur].cpu_pct), &(temp[cur].mem_pct), temp[cur].run_time, temp[cur].command);
				cur++;
			}
	}
	fclose(fp);
	return temp;
}

struct PS_INFO* make_ps_info(char* filename){
	char line[2000];
	int ppid;
	int c;
	char stime[10];
	char tty_name[10];
	int cur = 0;
	struct PS_INFO* temp = (struct PS_INFO*)malloc(sizeof(struct PS_INFO) * 1000);

	FILE* fp = fopen(filename, "r");
	if(fp == NULL){
		fprintf(stderr, "파일 열기 에러");
		exit(1);
	}

	for (int i = 0; i < 1000; i++)
	{
		temp[i].pid = -1;
	}
	

	fgets(line, 2000, fp);	//첫 한줄은 스킵
	while(1){
		//if(fgets(line, 200, fp) == NULL)
		//	break;
		if(feof(fp))
			break;
		fgets(line, 2000, fp);
		sscanf(line, "%s %d %d %d %s %s %s %s", temp[cur].user, &(temp[cur].pid), &ppid, &c, stime, tty_name, temp[cur].run_time, temp[cur].command);
		temp[cur].command[sizeof(temp->command) - 1] = '\0';
		cur++;
	}
	return temp;
}
