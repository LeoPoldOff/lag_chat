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

int parser(char massive1[][100], char* msg, int len) {
	len = len - 1;
	char* res_massive = malloc(50);
	int msg_len = strlen(msg);
	int counter = -1;
	int slot_counter = 0;
	
	if (msg_len <= len){
		printf("waste of time");
		return 0;
	}
	
	for (int e = 0; e < msg_len; e++){
		counter = counter + 1;
		res_massive[counter] = msg[e];

		if (counter == len){
			strcpy(massive1[slot_counter], res_massive);
			memset(res_massive, ' ', 50);
			slot_counter = slot_counter + 1;
			counter = -1;
			continue;
		}

		if (msg[e] == ' ' && counter < len && counter > len - 5){
			strcpy(massive1[slot_counter], res_massive);
			// printf(massive1[slot_counter]);
			memset(res_massive, ' ', 50);
			slot_counter = slot_counter + 1;
			counter = -1;
		}
	}
	strcpy(massive1[slot_counter], res_massive);
	return 1;
}

int main(int argc, char **argv) {
	char msg[40] = "abca bcabcab cabcabca bcab cabc";
	char massive[50][100];
	int *p;
	p = &msg;
	int ans = parser(massive, p, 6);
	printf("%d\n", ans);
	printf(massive[0]);
	printf("\n");
	printf(massive[1]);
	printf("\n");
	printf(massive[2]);
	printf("\n");
	printf(massive[3]);
	printf("\n");
	printf(massive[4]);
    return 0;
}