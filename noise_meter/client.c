#include "client.h"

Client_Struct *initClient(){
	
	Client_Struct *cli = malloc(sizeof *cli);
	cli->send_counter = 1;//inicialização do contador de envio de json a 1 (mais conveniente) 
						//Sempre que ocorre um envio este é incrementado
	cli->socket = 0;//Socket
	
	return cli;
}

void connectSocket(Client_Struct *cli,int socket_desc,struct sockaddr_in server){
	
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	
	if (socket_desc < 0){
		fprintf(stderr, "Could not create socket\n");
	}
	memset(&server, 0, sizeof server);
	server.sin_addr.s_addr = inet_addr(IP);
	server.sin_family = AF_INET;
	server.sin_port = htons( PORT );

	if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0){
		fprintf(stderr, "connect error\n");
		return;
	}
	fprintf(stderr, "Connected\n");
	
	cli->socket = socket_desc;//Afetar oo socket da estrutura com o socket da ligação
}

void sendJson(Client_Struct *cli,char *json){
	if( send(cli->socket, json, strlen(json), 0) < 0) {	//envio do Json
		printf("Send failed");
		return;
	}
	printf("Json Sent \n");
}
