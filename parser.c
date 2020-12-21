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
#include <regex.h>
#include "client_lag.c"

// функкция принимает на вход указатель на сообщение, макс. длину разбиения и массив для складирования результата
// возвращает количество кусков, на которое разбила
int long_str_parser(char massive1[][256], char* msg, int len) { 
	len = len - 1;
	char* res_massive = malloc(50); // вспомогательный массив
	int msg_len = strlen(msg);
	int counter = -1; // счётчик бегущий по кусочкам
	int slot_counter = 0; // счётчик бежит по массиву для результата
	int num_of_pieces = 0;

	for (int e = 0; e < msg_len; e++){
		counter = counter + 1;
		res_massive[counter] = msg[e];

		if (counter == len){
			strcpy(massive1[slot_counter], res_massive);
			memset(res_massive, ' ', 50); // зануляем вспомогательный массив
			slot_counter = slot_counter + 1;
			counter = -1;
			num_of_pieces = num_of_pieces + 1;
			continue;
		}

		if (msg[e] == ' ' && counter < len && counter > len - 5){
			strcpy(massive1[slot_counter], res_massive);
			memset(res_massive, ' ', 50);
			slot_counter = slot_counter + 1;
			counter = -1;
			num_of_pieces = num_of_pieces + 1;
		}
	}
	strcpy(massive1[slot_counter], res_massive);
	num_of_pieces = num_of_pieces + 1;
	return num_of_pieces;
}

// функия принимает массив для результата, указатель на сообщение и символ разделитель
// возвращает количество частей, на которые разделил
int str_separator(char massive1[][256], char msg[], char sep){
	char res_massive[50] = {'\0'};
	int counter = -1;
	int slot_counter = 0;
	int num_of_pieces = 0;
	char zero_str[50] = {'\0'};
	for (int e = 0; e < strlen(msg); e++){
		counter = counter + 1;
		if (msg[e] != sep){
			res_massive[counter] = msg[e];
			continue;
		}

		if (msg[e] == sep){
			if (strcmp(res_massive, zero_str) == 0){
				counter = -1;
				continue;
			}
			strcpy(massive1[slot_counter], res_massive);
			memset(res_massive, '\0', 50);
			slot_counter = slot_counter + 1;
			counter = -1;
			num_of_pieces = num_of_pieces + 1;
		}
	}
	if (strcmp(res_massive, zero_str) != 0){
		counter = -1;
		for (int x = 0; x < 50; x++){
			if (res_massive[x] != ' '){
				counter = counter + 1;
				massive1[slot_counter][counter] = res_massive[x];
			}
		}
		num_of_pieces = num_of_pieces + 1;
	}
	return num_of_pieces;
}



// функция принимает указатель на сообщение, и массив для складирование значений аргументов
// возвращает -1 если была ошибка при вводе, 0 если всё ок, но аргументов нет и 1 если всё ок и аргументы есть 
int request_parser(char* msg[], char args[][100]){
	system("clear");
	// printf("%s\n", msg);
	char massive[3][256];
	for (int i = 0; i < 3; i++)
		for(int j = 0; j < 256; j++)
			massive[i][j] = '\0';

	// printf("%s\n", massive[1]);

	char work_str[25] = {'\0'};
	char work_str1[25] = {'\0'};
	char null_str[25] = {'\0'};
	char null_str1[25] = {'\0'};

	
	int res = str_separator(massive, msg, '|');
	char command[10] = {'\0'};	
	char arg_header1[10] = {'\0'};
	char arg_header2[10] = {'\0'};
	memcpy(command, massive[0], 10);
	
	
	
	// printf("%s\n", massive[0]);
	// printf("%s\n", massive[1]);
	// printf("%d\n", strlen(massive[1]));
	// printf("%s\n", massive[2]);
	// printf("%d\n", strlen(massive[0]));
	// printf("%d\n", strlen(massive[1]));
	// printf("%d\n", strlen(massive[2]));
	// printf("%s\n", command);
	// printf("%s\n", arg_header1);
	// printf("%d\n", strlen(command));
	// printf("%d\n", strlen(arg_header1));
	
	
	if (strcmp(command, "ECHO") == 0) {
		if (strlen(massive[1]) == 0){
			// printf("%s\n", "Вставай Саня, ты опять обосрался!");
			return -1;
		}
		ECHO_SEND = 1;
		return 0;
	}

	else if (strcmp(command, "PING") == 0) {
		if (strlen(massive[1]) != 0){
			// printf("%s\n", "Вставай Саня, ты опять обосрался!");
			return -1;
		}
		return 0;
	}

	else if (strcmp(command, "USERS") == 0) {
		if (strlen(massive[1]) != 0){
			// printf("%s\n", "Вставай Саня, ты опять обосрался!");
			return -1;
		}
		return 0;
	}

	else if (strcmp(command, "HELP") == 0) {
		if (strlen(massive[1]) != 0){
			// printf("%s\n", "Вставай Саня, ты опять обосрался!");
			return -1;
		}
		return 0;
	}

	else if (strcmp(command, "SNDALL") == 0) {
		if (strlen(massive[1]) == 0){
			// printf("%s\n", "Вставай Саня, ты опять обосрался!1");
			return -1;
		}
		
		memcpy(arg_header1, massive[1], 4);
		if (strcmp(arg_header1, "msg=") != 0){
			// printf("%s\n", "Вставай Саня, ты трижды обосрался!2");
			return -1;
		}
		if (strlen(massive[1]) - 4 == 0){
			// printf("%s\n", "пустой первый аргумент");
			return -1; 
		}

		for (int a = 4; a < strlen(massive[1]); a++)
			work_str[a - 4] = massive[1][a];
		
		strcpy(args[0], work_str);
		printf("%s\n", args[0]);
		return 1;
	}

	else if (strcmp(command, "SEND") == 0) {
		if (strlen(massive[1]) == 0 || strlen(massive[2]) == 0){
			printf("%s\n", "Вставай Саня, ты опять обосрался!1");
			return -1;
		}
		
		printf("%s\n", arg_header1);
		printf("%d\n", strlen(arg_header1));
		memcpy(arg_header1, massive[1], 9);
		// strncpy(arg_header1, massive[1], 8);
		if (strcmp(arg_header1, "username=") != 0){
			// printf("%s\n", "Вставай Саня, ты трижды обосрался!2");
			// printf("%s\n", arg_header1);
			// printf("%d\n", strlen(arg_header1));
			return -1;
		}

		memcpy(arg_header2, massive[2], 4);
		if (strcmp(arg_header2, "msg=") != 0){
			// printf("%s\n", "Вставай Саня, ты трижды обосрался!3");
			return -1;
		}

		if (strlen(massive[1]) - 9 == 0){
			// printf("%s\n", "пустой первый аргумент");
			return -1; 
		}
		if (strlen(massive[1]) - 4 == 0){
			// printf("%s\n", "пустой второй аргумент");
			return -1; 
		}


		memset(work_str, ' ', strlen(massive[1]) - 9);
		// memset(null_str, ' ', strlen(massive[1]) - 9);
		for (int a = 9; a < strlen(massive[1]); a++)
			work_str[a - 9] = massive[1][a];
		// memcpy(arg_header1, massive[1], 9);
		
		memset(work_str1, ' ', strlen(massive[2]) - 4);
		// memset(null_str1, ' ', strlen(massive[2]) - 4);
		for (int a = 4; a < strlen(massive[2]); a++)
			work_str1[a - 4] = massive[2][a];

		
		strcpy(args[0], work_str);
		strcpy(args[1], work_str1);
		return 0;
	}

	else if (strcmp(command, "LOGIN") == 0) {
		if (strlen(massive[1]) == 0 || strlen(massive[2]) == 0){
			// printf("%s\n", "Вставай Саня, ты опять обосрался!1");
			return -1;
		}
		// strncpy(arg_header1, massive[1], 4);
		memcpy(arg_header1, massive[1], 9);
		if (strcmp(arg_header1, "username=") != 0){
			// printf("%s\n", "Вставай Саня, ты трижды обосрался!2");
			return -1;
		}
		memcpy(arg_header2, massive[2], 9);
		if (strcmp(arg_header2, "password=") != 0){
			// printf("%s\n", "Вставай Саня, ты трижды обосрался!3");
			return -1;
		} 
		memset(work_str, ' ', strlen(massive[1]) - 9);
		memset(null_str, ' ', strlen(massive[1]) - 9);
		for (int a = 9; a < strlen(massive[1]); a++)
			work_str[a - 9] = massive[1][a];
		// memcpy(arg_header1, massive[1], 9);
		if (strcmp(work_str, null_str) == 0){
			// printf("%s\n", "пустой первый аргумент");
			return -1;
		}
		
		memset(work_str1, ' ', strlen(massive[2]) - 9);
		memset(null_str1, ' ', strlen(massive[2]) - 9);
		for (int a = 9; a < strlen(massive[2]); a++)
			work_str1[a - 9] = massive[2][a];

		if (strcmp(work_str1, null_str1) == 0){
			// printf("%s\n", "пустой второй аргумент");
			return -1;
		}
		
		strcpy(args[0], work_str);
		strcpy(args[1], work_str1);
		return 0;
	}
	else {
		// printf("%s\n", "Вставай Саня, ты обосрался!4");
		return -1;
	}
	return 1;
}

// функция принимает input строку и инт сокета
// проверяет input на валидность(возвращает -1 или 1) и очищает input
// если валиден, то отправляет запрос серверу и вызывает функцию update 

//=======================================================//
/*
int main(int argc, char **argv) {
	int sock_fd = sock_init();
	// char msg[40] = "SEND|username=dasd|msg=dasds";
	char INPUT[256] = "SEND|username=dasd|msg=dasds";
	char args[50][100];
	char massive[50][100];
	int *p;
	// p = &msg;
	p = &INPUT;
	// int num_of_pieces = long_str_parser(massive, p, 6);
	// int num_of_pieces = str_separator(massive, p, '|');
	// print_massive_in_x_y(massive, num_of_pieces, 5, 5); '\0'
	// int res = request_parser(p, args);
	int res = Press_Enter(p, sock_fd);
	printf("%d\n", res);
	printf("%s\n", INPUT);
	// printf("%s\n", args[0]);
	// printf("%s\n", args[1]);
    return 0;
}
*/