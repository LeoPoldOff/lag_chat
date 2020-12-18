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

// функкция принимает на вход указатель на сообщение, макс. длину разбиения и массив для складирования результата
// возвращает количество кусков, на которое разбила
int long_str_parser(char massive1[][100], char* msg, int len) { 
	len = len - 1;
	char* res_massive = malloc(50); // вспомогательный массив
	int msg_len = strlen(msg);
	int counter = -1; // счётчик бегущий по кусочкам
	int slot_counter = 0; // счётчик бежит по массиву для результата
	int num_of_pieces = 0;
	
	if (msg_len <= len){
		printf("waste of time");
		return 1;
	}
	
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
int str_separator(char massive1[][100], char* msg, char sep){
	char* res_massive = malloc(50);
	int counter = -1;
	int slot_counter = 0;
	int num_of_pieces = 0;
	char* zero_str = malloc(50);
	memset(zero_str, ' ', 50);
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
			memset(res_massive, ' ', 50);
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

// функция для установки каретки на указанное место
void SetCursorPos(int XPos, int YPos)
{
    printf("\033[%d;%dH", YPos+1, XPos+1);
}

// функция принимает массив с данными, количество элементов и координаты в терминале
// пишет в столбик данные из массива начиная с указанных координат
void print_massive_in_x_y(char massive[][100], int num_of_pieces, int x, int y) {
	system("clear");
	for (int a = 0; a < num_of_pieces; a++){
			SetCursorPos(x, y);
			printf(massive[a]);
			y = y + 1;
	}
}

// функция принимает указатель на сообщение, и массив для складирование значений аргументов
// возвращает -1 если была ошибка при вводе, 0 если всё ок, но аргументов нет и 1 если всё ок и аргументы есть 
int request_parser(char* msg, char args[][50]){
	char massive[10][100];
	char* work_str = malloc(25);
	char* work_str1 = malloc(25);
	char* null_str = malloc(25);
	char* null_str1 = malloc(25);
	int res = str_separator(massive, msg, '|');
	// print_massive_in_x_y(massive, res, 0, 0)
	char* command = malloc(10);
	char* arg_header1 = malloc(10);
	char* arg_header2 = malloc(10);
	memcpy(command, massive[0], 10);
	// memcpy(arg_header1, massive[1], 10);
	// memcpy(arg_header2, massive[1], 10);
	
	printf("%s\n", massive[0]);
	printf("%s\n", massive[1]);
	printf("%s\n", massive[2]);
	printf("%d\n", strlen(massive[0]));
	printf("%d\n", strlen(massive[1]));
	printf("%d\n", strlen(massive[2]));
	printf("%s\n", command);
	printf("%s\n", arg_header1);
	printf("%d\n", strlen(command));
	printf("%d\n", strlen(arg_header1));
	
	if (strcmp(command, "ECHO") == 0) {
		if (strlen(massive[1]) != 0){
			printf("%s\n", "Вставай Саня, ты опять обосрался!");
			return -1;
		}
		return 0;
	}
	else if (strcmp(command, "PING") == 0) {
		if (strlen(massive[1]) != 0){
			printf("%s\n", "Вставай Саня, ты опять обосрался!");
			return -1;
		}
		return 0;
	}
	else if (strcmp(command, "USERS") == 0) {
		if (strlen(massive[1]) != 0){
			printf("%s\n", "Вставай Саня, ты опять обосрался!");
			return -1;
		}
		return 0;
	}
	else if (strcmp(command, "HELP") == 0) {
		if (strlen(massive[1]) != 0){
			printf("%s\n", "Вставай Саня, ты опять обосрался!");
			return -1;
		}
		return 0;
	}
	else if (strcmp(command, "SNDALL") == 0) {
		if (strlen(massive[1]) == 0){
			printf("%s\n", "Вставай Саня, ты опять обосрался!1");
			return -1;
		}
		// strncpy(arg_header1, massive[1], 4);
		memcpy(arg_header1, massive[1], 4);
		if (strcmp(arg_header1, "msg=") != 0){
			printf("%s\n", "Вставай Саня, ты трижды обосрался!2");
			return -1;
		} 
		memset(work_str, ' ', strlen(massive[1]) - 4);
		memset(null_str, ' ', strlen(massive[1]) - 4);
		for (int a = 4; a < strlen(massive[1]); a++)
			work_str[a - 4] = massive[1][a];

		if (strcmp(work_str, null_str) == 0){
			printf("%s\n", "пустой первый аргумент");
			return -1;
		}
		
		strcpy(args[0], work_str);
		return 0;
	}
	else if (strcmp(command, "SEND") == 0) {
		if (strlen(massive[1]) == 0 || strlen(massive[2]) == 0){
			printf("%s\n", "Вставай Саня, ты опять обосрался!1");
			return -1;
		}
		// strncpy(arg_header1, massive[1], 4);
		memcpy(arg_header1, massive[1], 9);
		if (strcmp(arg_header1, "username=") != 0){
			printf("%s\n", "Вставай Саня, ты трижды обосрался!2");
			return -1;
		}
		memcpy(arg_header2, massive[2], 4);
		if (strcmp(arg_header2, "msg=") != 0){
			printf("%s\n", "Вставай Саня, ты трижды обосрался!3");
			return -1;
		} 
		memset(work_str, ' ', strlen(massive[1]) - 9);
		memset(null_str, ' ', strlen(massive[1]) - 9);
		for (int a = 9; a < strlen(massive[1]); a++)
			work_str[a - 9] = massive[1][a];
		// memcpy(arg_header1, massive[1], 9);
		if (strcmp(work_str, null_str) == 0){
			printf("%s\n", "пустой первый аргумент");
			return -1;
		}
		
		memset(work_str1, ' ', strlen(massive[2]) - 4);
		memset(null_str1, ' ', strlen(massive[2]) - 4);
		for (int a = 4; a < strlen(massive[2]); a++)
			work_str1[a - 4] = massive[2][a];

		if (strcmp(work_str1, null_str1) == 0){
			printf("%s\n", "пустой второй аргумент");
			return -1;
		}
		
		strcpy(args[0], work_str);
		strcpy(args[1], work_str1);
		return 0;
	}
	else if (strcmp(command, "LOGIN") == 0) {
		if (strlen(massive[1]) == 0 || strlen(massive[2]) == 0){
			printf("%s\n", "Вставай Саня, ты опять обосрался!1");
			return -1;
		}
		// strncpy(arg_header1, massive[1], 4);
		memcpy(arg_header1, massive[1], 9);
		if (strcmp(arg_header1, "username=") != 0){
			printf("%s\n", "Вставай Саня, ты трижды обосрался!2");
			return -1;
		}
		memcpy(arg_header2, massive[2], 9);
		if (strcmp(arg_header2, "password=") != 0){
			printf("%s\n", "Вставай Саня, ты трижды обосрался!3");
			return -1;
		} 
		memset(work_str, ' ', strlen(massive[1]) - 9);
		memset(null_str, ' ', strlen(massive[1]) - 9);
		for (int a = 9; a < strlen(massive[1]); a++)
			work_str[a - 9] = massive[1][a];
		// memcpy(arg_header1, massive[1], 9);
		if (strcmp(work_str, null_str) == 0){
			printf("%s\n", "пустой первый аргумент");
			return -1;
		}
		
		memset(work_str1, ' ', strlen(massive[2]) - 9);
		memset(null_str1, ' ', strlen(massive[2]) - 9);
		for (int a = 9; a < strlen(massive[2]); a++)
			work_str1[a - 9] = massive[2][a];

		if (strcmp(work_str1, null_str1) == 0){
			printf("%s\n", "пустой второй аргумент");
			return -1;
		}
		
		strcpy(args[0], work_str);
		strcpy(args[1], work_str1);
		return 0;
	}
	else {
		printf("%s\n", "Вставай Саня, ты обосрался!4");
		return -1;
	}
	return 1;
}

//=======================================================//
int main(int argc, char **argv) {
	char msg[40] = "SEND|username=dasd|msg=dasds";
	char args[50][50];
	char massive[50][100];
	int *p;
	p = &msg;
	// int num_of_pieces = long_str_parser(massive, p, 6);
	// int num_of_pieces = str_separator(massive, p, '|');
	// print_massive_in_x_y(massive, num_of_pieces, 5, 5);
	int res = request_parser(p, args);
	printf("%d\n", res);
	printf("%s\n", args[0]);
	printf("%s\n", args[1]);
    return 0;
}