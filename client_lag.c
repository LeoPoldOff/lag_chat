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

#define CHAT_BUFFER_L 150 * 16
#define USERS_POINTERS_L 45 * 16


//int SCREEN = BackgoundInput;

// char INPUT[256] = {'\0'};
// int INPUT_SIZER[256] = {0};
// int INPUT_SIZER_POINTER = 0;
// int INPUT_SIZER_LAST_SYMBOL_POINTER = 0;
int ISRUN = 1;								// ISRUN

char CURSOR_X = 0;
char CURSOR_Y = 0;

char USER_LIST[4096][255];

char MSG_LIST[4096][255];
int MSG_LIST_POINTER = 0;

pthread_mutex_t CURSOR_MUTEX;

char CHAT[65536] = {'\0'};
int CHAT_POINTER = 0;

char USERS[4096] = {'\0'};

int SOCK_FD = 0;

int SHIFT_X = 39;
int SHIFT_Y = 28;

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
				handle_LOGIN(sock_fd, "agaffosh", "123");
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
//*******************COMMAND HANDLING*********************

//************************DAEMON**************************

int daemon_parser(char catched_commands[][255], char msg[])  		// парсер прилетающих с демона данных
{
	char* aux_arr = malloc(255);
	int counter = -1;
	int slot_counter = 0;
	int comms_count = 0;
	char* zero_str = malloc(255);
	memset(zero_str, ' ', 255);
	for (int e = 0; e < strlen(msg); e++){
		counter = counter + 1;
		if (msg[e] != '\n'){
			aux_arr[counter] = msg[e];
			continue;
		}

		if (msg[e] == '\n')
		{
			if (strcmp(aux_arr, zero_str) == 0){
				counter = -1;
				continue;
			}
			strcpy(catched_commands[slot_counter], aux_arr);
			memset(aux_arr, '\0', 255);
			slot_counter = slot_counter + 1;
			counter = -1;
			comms_count = comms_count + 1;
		}
	}
	if (strcmp(aux_arr, zero_str) != 0){
		counter = -1;
		for (int x = 0; x < 255; x++){
			if (aux_arr[x] != '\n'){
				counter = counter + 1;
				catched_commands[slot_counter][counter] = aux_arr[x];
			}
		}
		comms_count = comms_count + 1;
	}

	pthread_mutex_lock(&CURSOR_MUTEX);
	for (int i = 0; i < 4096; i++)
		for (int j = 0; j < 255; j++)
		{
			USER_LIST[i][j] = '\0';
		}
	pthread_mutex_unlock(&CURSOR_MUTEX);

	slot_counter = 0;
	counter = 0;

	pthread_mutex_lock(&CURSOR_MUTEX);
	for (int i = 0; i < comms_count; i++)
	{
		if (strstr(catched_commands[i], "MSGFROM [") != NULL)
		{
			counter = -1;
			for (int a = 0; a < strlen(catched_commands[i]); a++)
			{
				counter = counter + 1;
				MSG_LIST[MSG_LIST_POINTER][counter] = catched_commands[i][a];
			}
			// printf("\n----\n");
			// printf(MSG_LIST[MSG_LIST_POINTER]);
			// printf("\n----\n");

			MSG_LIST_POINTER = MSG_LIST_POINTER + 1;

		}
		else if (strcmp(catched_commands[i], "+") == 0)
		{
			continue;
		//	printf("logged\n");
		}
		else if (strstr(catched_commands[i], "Too fast") != NULL)
		{
			continue;
		//	printf("Too fast\n");
		}
		else 
		{
			counter = -1;
			//printf("\n-----\n");
			//printf(catched_commands[i]);
			//printf("\n-----\n");
			for (int a = 0; a < strlen(catched_commands[i]); a++)
			{
				counter = counter + 1;
				USER_LIST[i][counter] = catched_commands[i][a];
			}
			// printf("\n");
			// printf(USER_LIST[i]);
			// printf("\n");
		}
	}
	pthread_mutex_unlock(&CURSOR_MUTEX);
	update();

	return comms_count;
}

void auth(int sock_fd, char login[], char password[])			// авторизация по логину и паролю
{
	if(valid_login_password(login) && valid_login_password(password))
	{
		sleep(1);
		char result[MAX_BUF_SIZE];
		handle_LOGIN(sock_fd, login, password);
		memset(result, 0, sizeof(result));
		errwrap(recv(sock_fd, result, MAX_BUF_SIZE, 0));
		// printf(result);
	}
	else
	{
		return;													// тут подпилить неверный логин/пароль
		// printf("Invalid login/password");
	}
}

void *daemon_checker(void * arg) 				// собсна демон
{
	
	pthread_mutex_lock(&CURSOR_MUTEX);
	SOCK_FD = sock_init();	
	pthread_mutex_unlock(&CURSOR_MUTEX);					// наш сокет на приём данных (вырезать впоследствии)

	char cur_users[MAX_BUF_SIZE];
	char recv_buf[MAX_BUF_SIZE];

	struct pollfd fds[2];						// хероборина для обновления логина

	int users_timing_counter = 0;

    while(ISRUN)
    {
		pthread_mutex_lock(&CURSOR_MUTEX);

		auth(SOCK_FD, "agaffosh", "123");		// авторизируемся (вырезать впоследствии)

		pthread_mutex_unlock(&CURSOR_MUTEX);
		sleep(1);
		pthread_mutex_lock(&CURSOR_MUTEX);

		fds[0].fd = SOCK_FD;
		fds[0].events = POLLIN;

		pthread_mutex_unlock(&CURSOR_MUTEX);

		//timeout = 3000;
		int ret = poll( &fds, 2, 1000 );


		if (ret == -1)
		{
			printf("poll error");
			break;
		}
		else if (ret == 0)
		{
			if (users_timing_counter == 0)
			{											// пингуем список юзеров
													
				pthread_mutex_lock(&CURSOR_MUTEX);

				handle_USERS(SOCK_FD);
	  			memset(cur_users, 0, sizeof(cur_users));
	  			errwrap(recv(SOCK_FD, cur_users, MAX_BUF_SIZE, 0));
				
				pthread_mutex_unlock(&CURSOR_MUTEX);

				char tmp_user_array[4096][255];
				int res = daemon_parser(tmp_user_array, cur_users);
				users_timing_counter = users_timing_counter + 1;
				
	  			sleep(1);
			}
			else if (users_timing_counter == 10)
			{
				users_timing_counter = 0;
			}
			else
			{
				users_timing_counter = users_timing_counter + 1;
			}
			
		}
		else
		{
			if (users_timing_counter == 0)
			{											// пингуем список юзеров
													
				pthread_mutex_lock(&CURSOR_MUTEX);

				handle_USERS(SOCK_FD);
	  			memset(cur_users, 0, sizeof(cur_users));
	  			errwrap(recv(SOCK_FD, cur_users, MAX_BUF_SIZE, 0));

				pthread_mutex_unlock(&CURSOR_MUTEX);

				char tmp_user_array[4096][255];
				int res = daemon_parser(tmp_user_array, cur_users);
				users_timing_counter = users_timing_counter + 1;
				
	  			sleep(1);
			}
			else if (users_timing_counter == 10)
			{
				users_timing_counter = 0;
			}
			else
			{
				users_timing_counter = users_timing_counter + 1;
			}
			
			sleep(1);														
			pthread_mutex_lock(&CURSOR_MUTEX);				// принимаем сообщения

			memset(recv_buf, 0, sizeof(recv_buf));
			errwrap(recv(SOCK_FD, recv_buf, 2048, 0));

			pthread_mutex_unlock(&CURSOR_MUTEX);

			char tmp_msg_array[4096][255];
			int res3 = daemon_parser(tmp_msg_array, recv_buf);
			sleep(1);
		}

    }
	close(SOCK_FD);
    return NULL;
}

void kek()						// самая полезная функция эвер
{
	int i = 4;
    while(i > 0)
    {
        sleep(1);
		continue;
        //printf("kek\n");
		//i = i - 1;
    }   
}

void update()									// заглушка (выпилить впоследствии)
{
	printf("\nupdated\n");
}

void daemon_loop()				// петля с потоком для демона
{
	pthread_t daemon_thread;
	//printf("kek\n");
    pthread_create(&daemon_thread, NULL, daemon_checker, NULL);
	//printf("kek\n");
    pthread_join(&daemon_thread, NULL);
}


// int main(int argc, char **argv) 
// {
// 	// main_loop();

// 	//pthread_t 
// 	pthread_mutex_init(&CURSOR_MUTEX, NULL);
// 	daemon_loop();
// 	//daemon_checker();

// 	kek();
//     exit(0);

// }
