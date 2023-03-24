/*
 * udp_server.c
 *
 *  Created on: Mar 21, 2023
 *      Author: kolya
 */

#include "main.h"
#include "lwip.h"
#include "sockets.h"
#include "cmsis_os.h"
#include <string.h>

#include "LED_driver.h"
#include "Parser.h"

#define PORTNUM 5678UL

#if (USE_UDP_SERVER_PRINTF == 1)
#include <stdio.h>
#define UDP_SERVER_PRINTF(...) do { printf("[udp_server.c: %s: %d]: ",__func__, __LINE__);printf(__VA_ARGS__); } while (0)
#else
#define UDP_SERVER_PRINTF(...)
#endif

static struct sockaddr_in serv_addr, client_addr;
static int socket_fd;
static uint16_t nport;

enum {
	UPD_BufferSize = 256
};

static int udpServerInit(void)
{
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd == -1) {
		UDP_SERVER_PRINTF("socket() error\n");
		return -1;
	}

	nport = PORTNUM;
	nport = htons((uint16_t)nport);

	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = nport;

	if(bind(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
		UDP_SERVER_PRINTF("bind() error\n");
		close(socket_fd);
		return -1;
	}

	UDP_SERVER_PRINTF("Server is ready\n");

	return 0;
}

void StartUdpServerTask(void const * argument)
{
	osDelay(5000);// wait 5 sec to init lwip stack

	if(udpServerInit() < 0) {
		UDP_SERVER_PRINTF("updSocketServerInit() error\n");
		osThreadTerminate(NULL);
		return;
	}

	int nfds;
	fd_set rset;

	nfds = socket_fd + 1;

	for (;;) {
	    FD_ZERO(&rset);
	    FD_SET(socket_fd, &rset);
	    select(nfds, &rset, NULL, NULL, NULL);

	    if (FD_ISSET(socket_fd, &rset)) {
	    	memset(&client_addr, 0, sizeof(client_addr));

			char buffer[256];
			memset(buffer, 0, 256);
			socklen_t socklen = sizeof(client_addr);

			recvfrom(socket_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &socklen);

			char answer[256] = { 0 };

			LED_Operation_t operation = Parser_parseLedOperation(buffer);
			LED_driver_runLedOperation(operation);
			Parser_makeAnswer(operation, answer, sizeof(answer));

			sendto(socket_fd, answer, strlen(answer), 0, (struct sockaddr *)&client_addr, socklen);
	    }
	}

	close(socket_fd);
	osThreadTerminate(NULL);
}
