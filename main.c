/*
    ChibiOS - Copyright (C) 2006-2014 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"
#include "test.h"

#include "shell.h"
#include "sysinfo.h"
#include "socketstreams.h"

#include "lwip/sockets.h"
#include "lwip/tcpip.h"
#include "ppp/ppp.h"

#include <string.h>


#define PPP_CONNECTED_EVENT_FLAG      1
#define PPP_DISCONNECTED_EVENT_FLAG 	2
#define PPP_ERROR_EVENT_FLAG          4

EVENTSOURCE_DECL(pppEventSource);


/*
 * Shell related
 */
#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(1024)

static const ShellCommand shell_commands[] = {
	SYSINFO_SHELL_COMMANDS,
	{NULL, NULL}
};


/*
 * UDP Echo server thread
 */
static THD_WORKING_AREA(waEchoServerThread, 512 + 1024);
static THD_FUNCTION(EchoServerThread, arg) {
	(void) arg;

	event_listener_t pppEventListener;

	int sock;
	struct sockaddr_in sa;
	char buffer[1024];
	int recsize;
	socklen_t fromlen;

  chRegSetThreadName("EchoServerThread");

	chEvtRegisterMask(&pppEventSource, &pppEventListener, ALL_EVENTS);

	while (!chThdShouldTerminateX()) {
		chEvtWaitOne(PPP_CONNECTED_EVENT_FLAG);  // Wait for interface to come up

		sock = lwip_socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (sock == -1) {
			return MSG_RESET;
		}

		memset(&sa, 0, sizeof sa);
		sa.sin_family = AF_INET;
		sa.sin_addr.s_addr = htonl(INADDR_ANY);
		sa.sin_port = htons(8000);
		fromlen = sizeof(sa);

		if (lwip_bind(sock, (struct sockaddr *) &sa, sizeof(sa)) == -1) {
			lwip_close(sock);
			return MSG_RESET;
		}

		while (!chThdShouldTerminateX()) {
			recsize = lwip_recvfrom(sock, buffer, sizeof(buffer), 0,
					(struct sockaddr *) &sa, &fromlen);
			if (recsize < 0) {
				break;
			}
			lwip_sendto(sock, (void *) buffer, recsize, 0, (struct sockaddr *) &sa,
					fromlen);
		}

		lwip_close(sock);
	}

	return MSG_OK;
}


/*
 * TCP Shell server thread
 */
static THD_WORKING_AREA(waShellServerThread, 512);
static THD_FUNCTION(ShellServerThread, arg) {
	(void) arg;

	event_listener_t pppEventListener;

  int sockfd, newsockfd;
  socklen_t cli_addr_len;
	struct sockaddr_in serv_addr, cli_addr;

	SocketStream sbp;
	ShellConfig shell_cfg;
	thread_t *shelltp;

	chRegSetThreadName("ShellServerThread");

	chEvtRegisterMask(&pppEventSource, &pppEventListener, ALL_EVENTS);

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = PP_HTONS(25);

	while (!chThdShouldTerminateX()) {
		chEvtWaitOne(PPP_CONNECTED_EVENT_FLAG);  // Wait for interface to come up

		sockfd = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sockfd < 0)
				return MSG_RESET;

		if (lwip_bind(sockfd, (const struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
			lwip_close(sockfd);
			return MSG_RESET;
		}

		if (lwip_listen(sockfd, 1) < 0) {
			lwip_close(sockfd);
			return MSG_RESET;
		}

		cli_addr_len = sizeof(cli_addr);

		while(!chThdShouldTerminateX()) {
			newsockfd = lwip_accept(sockfd, (struct sockaddr * )&cli_addr, &cli_addr_len);
			if (newsockfd < 0) {
				break;
			}

			ssObjectInit(&sbp, newsockfd);
			shell_cfg.sc_channel = (BaseSequentialStream*) &sbp;
			shell_cfg.sc_commands = shell_commands;

			shelltp = shellCreate(&shell_cfg, SHELL_WA_SIZE, NORMALPRIO);
			chThdWait(shelltp);

			lwip_close(newsockfd);
		}

		if(lwip_shutdown(sockfd, SHUT_RDWR) < 0) {
			// oops
		}
		lwip_close(sockfd);
	}

	return MSG_OK;
}


/*
 * This is a periodic thread that does absolutely nothing except flashing
 * a LED.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {
  (void)arg;

  chRegSetThreadName("Thread1");

  while (TRUE) {
    palSetPad(GPIOD, GPIOD_LED3);       /* Orange.  */
    chThdSleepMilliseconds(500);
    palClearPad(GPIOD, GPIOD_LED3);     /* Orange.  */
    chThdSleepMilliseconds(500);
  }

	return MSG_OK;
}


/*
 * Handle link status events
 */
static void ppp_linkstatus_callback(void *ctx, int errCode, void *arg) {
	(void) arg;
	(void) ctx;

	if (errCode == PPPERR_NONE) {
			chEvtBroadcastFlags(&pppEventSource, PPP_CONNECTED_EVENT_FLAG);
	}
}


/*
 * Application entry point.
 */
int main(void) {
  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /*
   * Activates the serial driver 2 using the driver default configuration.
   * PA2(TX) and PA3(RX) are routed to USART2.
   */
  sdStart(&SD2, NULL);
  palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
  palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));

  /*
   * Creates the example thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO + 10, Thread1, NULL);

  /*
   * lwip ppp
   */
	tcpip_init(NULL, NULL);
	pppInit();

	/*
	 * Setup shell
	 */
	shellInit();

	/*
	 * Echo server
	 */
	chThdCreateStatic(waEchoServerThread, sizeof(waEchoServerThread), NORMALPRIO + 1, EchoServerThread, NULL);

	/*
	 * TCP Shell server
	 */
	chThdCreateStatic(waShellServerThread, sizeof(waShellServerThread), NORMALPRIO, ShellServerThread, NULL);

  /*
   * Keep ppp connection up.
   */
  while (TRUE) {
  	pppOverSerialOpen(&SD2, ppp_linkstatus_callback, NULL);
		chThdSleep(TIME_INFINITE);
		// Would like to restart here but at the moment we can't
		// See: http://lists.gnu.org/archive/html/lwip-users/2012-02/msg00124.html
  }
}
