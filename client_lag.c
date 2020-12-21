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

enum MainBackground
{
	BackgoundInput,
	BackgroundChat,
	BackgroundUsers
};

int START_NET = 1;
int BACKGROUND_POINTER = 0;

struct pollfd FDS[2];

int LOGIN[256] = {'\0'};
int PASSWORD[256] = {'\0'};

int SCREEN = BackgoundInput;

char INPUT[256] = {'\0'};
int INPUT_SIZER[256] = {0};
int INPUT_SIZER_POINTER = 0;
int INPUT_SIZER_LAST_SYMBOL_POINTER = 0;
int ISRUN = 1;								// ISRUN

char CURSOR_X = 39;
char CURSOR_Y = 37;

char USER_LIST[4096][256];
int USER_LIST_POINTER = 0;
int USER_POINTER = 0;

char MSG_LIST[4096][256];
int MSG_LIST_POINTER = 0;
int MSG_POINTER = 0;

char CHAT[65536] = {'\0'};
int CHAT_POINTER = 0;

char USERS[4096] = {'\0'};

int SOCK_FD = 0;

int SHIFT_X = 39;
int SHIFT_Y = 37;

void reverse(char s[])
 {
     int i, j;
     char c;
 
     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
 }

 void itoa(int n, char s[])
 {
     int i, sign;
 
     if ((sign = n) < 0)  /* записываем знак */
         n = -n;          /* делаем n положительным числом */
     i = 0;
     do {       /* генерируем цифры в обратном порядке */
         s[i++] = n % 10 + '0';   /* берем следующую цифру */
     } while ((n /= 10) > 0);     /* удаляем */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
 }

void Zerolizer()
{
	int START_NET = 1;
	int BACKGROUND_POINTER = BackgoundInput;

	memset(LOGIN, '\0', 256);
	memset(PASSWORD, '\0', 256);
	memset(INPUT, '\0', 256);
	memset(INPUT_SIZER, 0, 256);
	
	INPUT_SIZER_POINTER = 0;
	INPUT_SIZER_LAST_SYMBOL_POINTER = 0;
	ISRUN = 1;								// ISRUN

	CURSOR_X = 39;
	CURSOR_Y = 41;

	USER_POINTER = 0;
	MSG_POINTER = 0;

	USER_LIST[4096][256];
	USER_LIST_POINTER = 0;

	MSG_LIST[4096][256];
	MSG_LIST_POINTER = 0;

	SOCK_FD = 0;

	SHIFT_X = 39;
	SHIFT_Y = 37;
}
// ====================================================
// функция принимает массив с данными, количество элементов и координаты в терминале
// пишет в столбик данные из массива начиная с указанных координат

// функция для установки каретки на указанное место
void SetCursorPos(int XPos, int YPos)
{
    printf("\033[%d;%dH", YPos+1, XPos+1);
}

void print_massive_in_x_y(char massive[][256], int num_of_pieces, int start_segment, int x, int y) {
	for (int a = 0; a < num_of_pieces; a++){
			SetCursorPos(x, y);
			printf(massive[start_segment + a]);
			y = y + 1;
	}
}

void GetMainScreen()
{
    char main_screen[4096];
    FILE *fp;
    char *file_name[32];
	system("clear");
    switch (BACKGROUND_POINTER)
    {
        case BackgoundInput:
        {
            *file_name = "static/main_input.txt";
            break;
        }
        case BackgroundChat:
        {
            *file_name = "static/main_chat.txt";
            break;
        }
        case BackgroundUsers:
        {
            *file_name = "static/main_users.txt";
        }
        default:
        {
            system("clear");
            printf("Неизвестный бэкграунд\n");
            break;
        }
    }
    fp = fopen(*file_name, "r");
    SetCursorPos(0, 0);
    while (fgets(main_screen, 2812, fp) != NULL)
        printf("%s", main_screen);
    fclose(fp);
}

void CopyToBuffer(char source[], char target[], int start, int size)
{
    int last = start + size;
    for (int i = start; i < last; i++)
        target[i - start] = source[i];
}

void PrintContentChat()
{
    print_massive_in_x_y(MSG_LIST, 16,  MSG_POINTER, 39, 18);
}

int getPointer(int symbol_number)
{
    int result = 0;
    for (int i = 0; i < symbol_number; i++)
        result = result + INPUT_SIZER[i];
    return result;
}

void PrintContentUsers()
{
    print_massive_in_x_y(USER_LIST, 20, USER_POINTER, 133, 18);
}

void PrintContentInput()
{
    SetCursorPos(SHIFT_X, SHIFT_Y);
    printf(INPUT);
}

void PrintLeftSymbols()
{
	char max_count[] = " / 85";
	char current_count[10];
    itoa(INPUT_SIZER_POINTER, current_count);
    strcat(current_count, max_count);
	char res[1][8];
	copystr_0(current_count, res[0], 0, 8);
	print_massive_in_x_y(res, 1, 0, SHIFT_X, SHIFT_Y + 1);
}

void Update()
{
	got_msgfrom();	
    GetMainScreen();
    PrintContentChat();
    PrintContentUsers();
    PrintContentInput();
	PrintLeftSymbols();
    SetCursorPos(CURSOR_X, CURSOR_Y);
}

// ====================================================

//*******************SERVICE FUNCTIONS******************
void err(char *msg, const char *arg, bool critical) {
    if(msg == NULL || strlen(msg) == 0) {
        msg = strerror(errno);
		if (!strcmp(msg, "Success") || !strcmp(msg, "Выполнено")) {
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

void Piza_tower(char msg[]){
	int ch = '|';
	if (strchr(msg, ch) != NULL){
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

int return_sticker(int num, char buf[][256]){
	FILE *file;
	file = fopen("static/stickers.txt", "r");
	// char result[100] = {'\0'};
	char arr[45];
	int skip = 6;
	skip = skip * num;
	for (int e = 0; e < skip; e++)
		fgets(arr, 45, file);

	for (int i = 0; i < 5; i++){
		fgets(arr, 45, file);
		// strcat(result, arr);
		// printf("%s\n", arr);
		strcpy(buf[i], arr);
	}

    fclose(file);
    return 5;
}

char *return_titres(){
	FILE *file;
	file = fopen("static/titres.txt", "r");
	char result[100] = {'\100'};
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
	char result = {'\0'};
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


int send_buf(int sock_fd, char buf[]) {
	int result = errwrap(send(sock_fd, buf, strlen(buf), 0));
	return result;
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

int getLength(char arr[])
{
	for (int i = 0; i < 256; i++)
	{
		if (arr[i] == '\0')
			return i;
	}
	system("clear");
	printf("BIG LENGTH");
	exit(-1);
}

int handle_LOGIN(int sock_fd, char login[], char password[]) {
	char cmd[] = "LOGIN|username=";
	char msg[36];
	char cmd1[] = "|password=";
	char msg1[36];
	//scanf("%s", &msg);
	Piza_tower(login);
	//scanf("%s", &msg1);
	int *p1;
	Piza_tower(password);
	char* res = malloc(strlen(cmd) + getLength(login) + strlen(cmd1) + getLength(password) + 1);
	strcpy(res, cmd);
	strcat(res, login);
	strcat(res, cmd1);
	strcat(res, password);
	
	int val =  send_buf(sock_fd, res);
	return val;
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

//*******************COMMAND HANDLING*********************

//************************DAEMON**************************

int copystr(char source[], char target[], int start, int count)
{
	for (int i = start; i < start + count; i++)
	{
		target[i] = source[i];
		if (source[i] == '\0')
			return 0;
	}
	return 1;
}

int copystr_0(char source[], char target[], int start, int count)
{
	for (int i = start; i < start + count; i++)
	{
		target[i - start] = source[i];
		if (source[i] == '\0')
			return 0;
	}
	return 1;
}

int daemon_parser(char catched_commands[][256], char msg[])  		// парсер прилетающих с демона данных
{
	char aux_arr[256] = {'\0'};
	int counter = -1;
	int slot_counter = 0;
	int comms_count = 0;
	char zero_str[256] = {'\0'};
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

	slot_counter = 0;
	counter = 0;

	int dirty = 1;
	for (int i = 0; i < comms_count; i++)
	{
		if (strstr(catched_commands[i], "MSGFROM [") != NULL || strstr(catched_commands[i], "v. 0.") != NULL || strstr(catched_commands[i], "Available commands") != NULL)
		{
		//============================ STICKERS =================================

			// if(strstr(catched_commands[i], "=)") != NULL)
			if (catched_commands[i][strlen(catched_commands[i]) - 1] == ')' && catched_commands[i][strlen(catched_commands[i]) - 2] == '=' && catched_commands[i][strlen(catched_commands[i]) - 3] == ' ' && catched_commands[i][strlen(catched_commands[i]) - 4] == ':')
			{
				copystr(catched_commands[i], MSG_LIST[MSG_LIST_POINTER], 0, strlen(catched_commands[i]) - 3);
				MSG_LIST_POINTER ++;

				parse_sticker_to_MSGLIST(0);
			}
			else if (catched_commands[i][strlen(catched_commands[i]) - 1] == ')' && catched_commands[i][strlen(catched_commands[i]) - 2] == 'X' && catched_commands[i][strlen(catched_commands[i]) - 3] == ' ' && catched_commands[i][strlen(catched_commands[i]) - 4] == ':')
			{
				copystr(catched_commands[i], MSG_LIST[MSG_LIST_POINTER], 0, strlen(catched_commands[i]) - 3);
				MSG_LIST_POINTER ++;

				parse_sticker_to_MSGLIST(1);
			}
			else if (catched_commands[i][strlen(catched_commands[i]) - 1] == ')' && catched_commands[i][strlen(catched_commands[i]) - 2] == '8' && catched_commands[i][strlen(catched_commands[i]) - 3] == ' ' && catched_commands[i][strlen(catched_commands[i]) - 4] == ':')
			{
				copystr(catched_commands[i], MSG_LIST[MSG_LIST_POINTER], 0, strlen(catched_commands[i]) - 3);
				MSG_LIST_POINTER ++;

				parse_sticker_to_MSGLIST(2);
			}
			else if (catched_commands[i][strlen(catched_commands[i]) - 1] == '(' && catched_commands[i][strlen(catched_commands[i]) - 2] == '=' && catched_commands[i][strlen(catched_commands[i]) - 3] == ' ' && catched_commands[i][strlen(catched_commands[i]) - 4] == ':')
			{
				copystr(catched_commands[i], MSG_LIST[MSG_LIST_POINTER], 0, strlen(catched_commands[i]) - 3);
				MSG_LIST_POINTER ++;

				parse_sticker_to_MSGLIST(3);
			}
			else if (catched_commands[i][strlen(catched_commands[i]) - 1] == '/' && catched_commands[i][strlen(catched_commands[i]) - 2] == '=' && catched_commands[i][strlen(catched_commands[i]) - 3] == ' ' && catched_commands[i][strlen(catched_commands[i]) - 4] == ':')
			{
				copystr(catched_commands[i], MSG_LIST[MSG_LIST_POINTER], 0, strlen(catched_commands[i]) - 3);
				MSG_LIST_POINTER ++;

				parse_sticker_to_MSGLIST(4);
			}

		//=======================================================================
			else
			{
				int val = copystr_0(catched_commands[i], MSG_LIST[MSG_LIST_POINTER], 0, 88);
				int counter = 0;
				while (val > 0)
				{
					MSG_LIST_POINTER = MSG_LIST_POINTER + 1;
					counter++;
					int position = counter * 88;
					val = copystr_0(catched_commands[i], MSG_LIST[MSG_LIST_POINTER], position, 88);
				}
				MSG_LIST_POINTER = MSG_LIST_POINTER + 1;
			}
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
		else if (strstr(catched_commands[i], "Выполнено") != NULL)
		{
			continue;
		}
		else 
		{
			if (dirty)
			{
				for (int k = 0; k < 4096; k++)
					for (int j = 0; j < 255; j++)
						USER_LIST[k][j] = '\0';
				dirty = 0;
				USER_LIST_POINTER = comms_count;
			}
			copystr(catched_commands[i], USER_LIST[i], 0, 20);
		}
	}

	// if (MSG_LIST_POINTER > 2)
	// {
	// 	printf("%d\n", MSG_LIST_POINTER);
	// 	exit(0);
	// }
	return comms_count;
}

void parse_sticker_to_MSGLIST(int sticker_number)
{
	char tmp_sticker_arr[5][256];
	int a = return_sticker(sticker_number, tmp_sticker_arr);

	//int sticker_arr_counter = 0;
	for(int j = 0; j < 5; j++)
	{
		int val0 = copystr(tmp_sticker_arr[j], MSG_LIST[MSG_LIST_POINTER], 0, 150);
		MSG_LIST_POINTER++;
	}
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
		sleep(1);

		handle_USERS(SOCK_FD);
		//printf(result);
	}
	else
	{
		return;													// тут подпилить неверный логин/пароль
		// printf("Invalid login/password");
	}
}

void daemon_checker() 				// собсна демон
{
	SOCK_FD = sock_init();
	sleep(1);

	char cur_users[MAX_BUF_SIZE];
	char recv_buf[MAX_BUF_SIZE];

	struct pollfd fds[2];						// хероборина для обновления логина

	int users_timing_counter = 0;

    while(ISRUN)
    {
		auth(SOCK_FD, LOGIN, PASSWORD);
		sleep(1);

		fds[0].fd = SOCK_FD;
		fds[0].events = POLLIN;

		//timeout = 3000;
		int ret = poll( &fds, 2, 1000 );


		if (ret == -1)
		{
		//	printf("poll error");
			break;
		}
		else if (ret == 0)
		{
			if (users_timing_counter == 0)
			{			
				handle_USERS(SOCK_FD);
	  			memset(cur_users, 0, sizeof(cur_users));
	  			errwrap(recv(SOCK_FD, cur_users, MAX_BUF_SIZE, 0));
				
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
													

				handle_USERS(SOCK_FD);
	  			memset(cur_users, 0, sizeof(cur_users));
	  			errwrap(recv(SOCK_FD, cur_users, MAX_BUF_SIZE, 0));


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
			
			// sleep(1);								// принимаем сообщения

			// memset(recv_buf, 0, sizeof(recv_buf));
			// errwrap(recv(SOCK_FD, recv_buf, 2048, 0));


			// char tmp_msg_array[4096][255];
			// int res3 = daemon_parser(tmp_msg_array, recv_buf);
			// sleep(1);
		}

    }
	//close(SOCK_FD);
    return NULL;
}

void got_msgfrom()
{
    FDS[0].fd = SOCK_FD;
	FDS[0].events = POLLIN;

	char recv_buf[MAX_BUF_SIZE] = {'\0'};				// принимаем сообщения
	int ret = poll(&FDS, 2, 20);
	if (ret == -1 || ret == 0)
	{
		//printf("empty poll");
		return;
	}
	memset(recv_buf, '\0', MAX_BUF_SIZE);
	errwrap(recv(SOCK_FD, recv_buf, MAX_BUF_SIZE, 0));
	char tmp_msg_array[4096][256];
	daemon_parser(tmp_msg_array, recv_buf);
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

/*
void update()									// заглушка (выпилить впоследствии)
{
	printf("\nupdated\n");
}
*/

// void daemon_loop()				// петля с потоком для демона
// {
// 	pthread_t daemon_thread;
// 	//printf("kek\n");
//     pthread_create(&daemon_thread, NULL, daemon_checker, NULL);
// 	//printf("kek\n");
//     pthread_join(&daemon_thread, NULL);
// }


// int main(int argc, char **argv) 
// {
// 	// main_loop();

// 	//pthread_t 
// 	//daemon_loop();
// 	//daemon_checker();

// 	//kek();
// 	char recv_buf[MAX_BUF_SIZE] = {'\0'};
// 	int sock_fd = sock_init();
// 	sleep(1);
// 	handle_LOGIN(sock_fd, "agaffosh", "123");
// 	printf("%d\n", errwrap(recv(sock_fd, recv_buf, 2048, 0)));

//     exit(0);

// }
