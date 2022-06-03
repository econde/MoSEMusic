#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>	//strlen
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addr

#define SEND_TIME_S 300
#define IP "127.0.0.1"
#define PORT 8000

typedef struct{
	int send_counter;
	int socket;
} Client_Struct;


Client_Struct *initClient();	//Iniciar o cliente socket

void connectSocket(Client_Struct *cli,int socket_desc,struct sockaddr_in server);	//conectar ao socket do servidor
void sendJson(Client_Struct *cli,char *json);	//enviar o json

#endif
