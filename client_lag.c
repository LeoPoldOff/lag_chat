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

#define HOST "3.9.16.135"
#define PORT 81
#define MAX_BUF_SIZE 256

//*******************SERVICE FUNCTIONS******************
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

char *return_sticker(int num){
	FILE *file;
	file = fopen("static/stickers.txt", "r");
	char *result = malloc(100);
	char arr[45];
	int skip = 6;
	skip = skip * num;
	for (int e = 0; e < skip; e++)
		fgets(arr, 45, file);

	for (int i = 0; i < 6; i++){
		fgets(arr, 45, file);
		strcat(result, arr);
	}

    fclose(file);
    return result;
}

char *return_titres(){
	FILE *file;
	file = fopen("static/titres.txt", "r");
	char *result = malloc(3200);
	char arr[200];

	for (int i = 0; i < 16; i++){
		fgets(arr, 200, file);
		strcat(result, arr);
	}

    fclose(file);
    return result;
}

char *return_avatar(int num){
	FILE *file;
	file = fopen("static/avatars.txt", "r");
	char *result = malloc(100);
	char arr[45];
	int skip = 5;
	skip = skip * num;
	for (int e = 0; e < skip; e++)
		fgets(arr, 45, file);

	for (int i = 0; i < 5; i++){
		fgets(arr, 45, file);
		strcat(result, arr);
	}

    fclose(file);
    return result;
}

char *return_joke(int num){
	FILE *file;
	file = fopen("static/jokes.txt", "r");
	char *result = malloc(500);
	char arr[100];
	int skip = 6;
	skip = skip * num;
	for (int e = 0; e < skip; e++)
		fgets(arr, 100, file);

	for (int i = 0; i < 6; i++){
		fgets(arr, 100, file);
		if (strlen(arr) == 1)
			continue;
		strcat(result, arr);
	}

    fclose(file);
    return result;
}
//*******************SERVICE FUNCTIONS******************

//*******************COMMAND HANDLING******************


int send_buf(int sock_fd, char* buf) {
	return errwrap(send(sock_fd, buf, strlen(buf), 0));
}

int handle_ECHO(int sock_fd) {
	char* cmd = "ECHO|msg=";
	char msg[247];
	scanf("%s", &msg);
	// int ch = '|';
	// if (strchr(msg, ch) != NULL){
	// 	printf("\nВсе '|' заменены на '/'\n");
	// 	for (int i = 0; i < sizeof(msg); i++) 
	// 		if (msg[i] == '|') 
	// 			msg[i] = '/';
	// }
	int *p;
	p = &msg;
	Piza_tower(p);
	printf(msg);
	char* res = malloc(strlen(cmd) + strlen(msg) + 1);
	strcpy(res, cmd);
	strcat(res, msg);
	// Сервер принимает команду ECHO с 1 аргументом msg
	// Необходимо считать аргумент к ECHO с stdin
	// и отправить на сервер команду в формате протокола, показанного в документации.
	// Если всё сделано правильно, сообщение из аргумента вернётся и отобразится в клиенте
	// Для отладки можно использовать netcat
	// Или переписать всё на python
	// Успешное выполнение задания - наличие в логах сервера успешно выполненной команды ECHO <ваше_имя>
	return send_buf(sock_fd, res);
}

int handle_LOGIN(int sock_fd) {
	char* cmd = "LOGIN|username=";
	char msg[36];
	char* cmd1 = "|password=";
	char msg1[36];
	scanf("%s", &msg);
	int *p;
	p = &msg;
	Piza_tower(p);
	scanf("%s", &msg1);
	int *p1;
	p1 = &msg1;
	Piza_tower(p1);
	char* res = malloc(strlen(cmd) + strlen(msg) + strlen(cmd1) + strlen(msg1) + 1);
	strcpy(res, cmd);
	strcat(res, msg);
	strcat(res, cmd1);
	strcat(res, msg1);
	return send_buf(sock_fd, res);
}

int handle_PING(int sock_fd) {
	char* cmd = "PING";
	return send_buf(sock_fd, cmd);
}

int handle_USERS(int sock_fd) {
	char* cmd = "USERS";
	return send_buf(sock_fd, cmd);
}

int handle_HELP(int sock_fd) {
	char* cmd = "HELP";
	return send_buf(sock_fd, cmd);
}

int handle_SEND(int sock_fd) {
	char* cmd = "SEND|username=";
	char usr[36];
	scanf("%s", &usr);
	int *p;
	p = &usr;
	Piza_tower(p);

	char* cmd1 = "|msg=";
	char msg[201];
	scanf("%s", &msg);
	int *p1;
	p1 = &msg;
	Piza_tower(p1);

	char* res = malloc(strlen(cmd) + strlen(usr) + strlen(cmd1) + strlen(msg) + 1);
	strcpy(res, cmd);
	strcat(res, usr);
	strcat(res, cmd1);
	strcat(res, msg);
	return send_buf(sock_fd, res);
}

int handle_SNDALL(int sock_fd) {
	char* cmd = "SNDALL|msg=";
	char msg[245];
	scanf("%s", &msg);
	int *p;
	p = &msg;
	Piza_tower(p);
	char* res = malloc(strlen(cmd) + strlen(msg) + 1);
	strcpy(res, cmd);
	strcat(res, msg);
	return send_buf(sock_fd, res);
}

void main_loop() {
	int sock_fd = sock_init();
	int user_input = 0;
	char result[MAX_BUF_SIZE];
	char* help = 
		"Choose an option:\n\n"
		"1. Echo <msg>\n"
		"2. Ping\n"
		"3. Show users\n"
		"4. Show help\n"
		"5. Exit\n"
		"6. Login <login> <password>\n"
		"7. Send <username> <msg>\n"
		"8. Send All <msg>\n\n";

	while(1) {
		printf(help);
		scanf("%d", &user_input);
		switch(user_input) {
			case 1: {
				handle_ECHO(sock_fd);
				break;
			}
			case 2: {
				handle_PING(sock_fd);
				break;
			}
			case 3: {
				handle_USERS(sock_fd);
				break;
			}
			case 4: {
				handle_HELP(sock_fd);
				break;
			}
			case 5: {
				goto _exit;
			}
			case 6: {
				handle_LOGIN(sock_fd);
				break;
			}
			case 7: {
				handle_SEND(sock_fd);
				break;
			}
			case 8: {
				handle_SNDALL(sock_fd);
				break;
			}
			default: {
				printf("Wrong option");
				continue;
			}
		}
		memset(result, 0, sizeof(result));
		errwrap(recv(sock_fd, result, MAX_BUF_SIZE, 0));
		printf("Response: \n%s\n", result);
		sleep(1);
	}
_exit:
    close(sock_fd);
}
//*******************COMMAND HANDLING******************

int main(int argc, char **argv) {
	main_loop();
    return 0;
}
