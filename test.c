#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <time.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <poll.h>

#define HOST "3.9.16.135"
#define PORT 81
#define MAX_BUF_SIZE 4096

int ISRUN = 1;


void err(char *msg, const char *arg, bool critical) {
    if(msg == NULL || strlen(msg) == 0) {
        msg = strerror(errno);
		if (!strcmp(msg, "Success")) {
			return;
		}
    }
    if (arg != NULL) {
        fprintf(stderr, "%s, '%s'\n", msg, arg);
    } else {
        fprintf(stderr, "%s\n", msg);
    }
    if(critical) {
        exit(-1);
    }
}

// handy error checker
int errwrap(int ret) {
    if(ret == -1) {
		// critical, show error and exit
        err(NULL, NULL, true);
        return -1;
    } else {
		// show error and continue execution
        err(NULL, NULL, false);
        return ret;
    }
}

int sock_init() {
	struct sockaddr_in serv_addr;
    int sock_fd = errwrap(socket(AF_INET, SOCK_STREAM, 0));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
	errwrap(inet_pton(AF_INET, HOST, &serv_addr.sin_addr));
	errwrap(connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)));
	return sock_fd;
}

int handle_USERS(int sock_fd) {
	char* cmd = "USERS";
	return send_buf(sock_fd, cmd);
}

void Piza_tower(char* msg){
	int ch = '|';
	if (strchr(msg, ch) != NULL){
		printf("\nВсе '|' заменены на '/'\n");
		for (int i = 0; i < strlen(msg); i++) 
			if (msg[i] == '|') 
				msg[i] = '/';
	}
}

int valid_login_password(char* msg){
	char* valid = "QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm1234567890-=!@#$%^&*()_+`~|}{\\][:;/?.>,<";
	for (int i = 0; i < strlen(msg); i++)
		if (strchr(valid, msg[i]) == NULL)
			return 0;
	return 1;
}

int send_buf(int sock_fd, char* buf) {
	return errwrap(send(sock_fd, buf, strlen(buf), 0));
}

int handle_LOGIN(int sock_fd, char* login, char* password) {
	char* cmd = "LOGIN|username=";
	char msg[36];
	char* cmd1 = "|password=";
	char msg1[36];
	//scanf("%s", &msg);
	int *p;
	p = login;
	Piza_tower(p);
	//scanf("%s", &msg1);
	int *p1;
	p1 = password;
	Piza_tower(p1);
	char* res = malloc(strlen(cmd) + strlen(p) + strlen(cmd1) + strlen(p1) + 1);
	strcpy(res, cmd);
	strcat(res, p);
	strcat(res, cmd1);
	strcat(res, p1);
	return send_buf(sock_fd, res);
}

void auth(int sock_fd, char* login, char* password)			// авторизация по логину и паролю
{
	if(valid_login_password(login) && valid_login_password(password))
	{
		sleep(1);
		char result[MAX_BUF_SIZE];
		handle_LOGIN(sock_fd, login, password);
		memset(result, 0, sizeof(result));
		errwrap(recv(sock_fd, result, MAX_BUF_SIZE, 0));
		printf(result);
	}
	else
	{
		printf("Invalid login/password");
	}
}

void * daemon_checker(void * arg)
{
    int sock_fd = sock_init();
    printf("sock created\n");

    char cur_users[MAX_BUF_SIZE];
	char recv_buf[MAX_BUF_SIZE];
    printf("buffers created\n");

    struct pollfd fds[2];
    printf("poll struct created\n");

    while(ISRUN)
    {
        printf("\nWe're in cycle\n\n");
        auth(sock_fd, "123", "456");

        fds[0].fd = sock_fd;
		fds[0].events = POLLIN;
        int ret = poll( &fds, 2, 3000);
        printf ("poll ret created\n");
        sleep(1);

        if (ret == -1)
		{
			printf("poll error");
			break;
		}
		else if (ret == 0)
		{
            auth(sock_fd, "123", "456");
            printf("\nlogged again\n");
			sleep(1);

            handle_USERS(sock_fd);
	  		memset(cur_users, 0, sizeof(cur_users));
	  		errwrap(recv(sock_fd, cur_users, MAX_BUF_SIZE, 0));
	  		printf(cur_users);
            printf("\n\n");
	  		sleep(1);
        }
        else
        {
            handle_USERS(sock_fd);
	  		memset(cur_users, 0, sizeof(cur_users));
	  		errwrap(recv(sock_fd, cur_users, MAX_BUF_SIZE, 0));
	  		printf(cur_users);
	  		printf("\n\n");
	  		sleep(1);

            sleep(1);
			memset(recv_buf, 0, sizeof(recv_buf));
			errwrap(recv(sock_fd, recv_buf, 2048, 0));
			printf(recv_buf);
			printf("\n\n");
			sleep(1);
        }
        

        sleep(1);
        //printf("my turn!\n");
    }
    close(sock_fd);
    return NULL;
}

void kek()						// самая полезная функция эвер
{
	int i = 4;
    while(i > 0)
    {
        sleep(1);
        continue;
        //printf("\nkek\n");
		//i = i - 1;
    }   
}

void daemon_loop()
{

    printf("kek\n");
    pthread_t daemon_thread;

    pthread_create(&daemon_thread, NULL, daemon_checker, NULL);

    pthread_join(&daemon_thread, NULL);

}

int main(int argc, char **argv)
{
    daemon_loop();

   // pthread_t test_thread_2;

    //pthread_create(&test_thread_2, NULL, my_turn, NULL);

    // sleep(10);
    // ISRUN = 0;

    kek();

    // sleep(1);
    // printf("your_turn!\n");

    exit(0);
    
}