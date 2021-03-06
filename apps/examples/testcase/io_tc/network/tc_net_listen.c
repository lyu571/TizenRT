/****************************************************************************
 *
 * Copyright 2016 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/

/// @file tc_net_listen.c
/// @brief Test Case Example for listen() API
#include <tinyara/config.h>
#include <stdio.h>
#include <errno.h>

#include <sys/stat.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/in.h>
//#include <arch/board/board.h>
#include <apps/netutils/netlib.h>

#include <sys/socket.h>

#include "tc_internal.h"
/**
   * @fn                   :tc_net_listen_p
   * @brief                :
   * @scenario             :
   * API's covered         :listen()
   * Preconditions         :
   * Postconditions        :
   * @return               :void
   */
static void tc_net_listen_p(void)
{
	struct sockaddr_in sa;
	int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	memset(&sa, 0, sizeof sa);

	sa.sin_family = AF_INET;
	sa.sin_port = htons(1100);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(SocketFD, (struct sockaddr *)&sa, sizeof(sa));
	int ret = (listen(SocketFD, 10));
	close(SocketFD);

	if (ret == -1) {
		printf("tc_net_listen_p: fail\n");

		nw_total_fail++;
		RETURN_ERR;
	}

	printf("tc_net_listen_p: PASS\n");
	nw_total_pass++;
}

/**
   * @fn                   :tc_net_listen_fd_n
   * @brief                :
   * @scenario             :
   * API's covered         :listen()
   * Preconditions         :
   * Postconditions        :
   * @return               :void
   */
static void tc_net_listen_fd_n(void)
{
	struct sockaddr_in sa;
	int fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	memset(&sa, 0, sizeof sa);

	sa.sin_family = AF_INET;
	sa.sin_port = htons(1101);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(fd, (struct sockaddr *)&sa, sizeof(sa));
	int ret = (listen(-1, 10));

	if (ret == 0) {
		printf("tc_net_listen_fd_n: FAIL\n");

		nw_total_fail++;
		RETURN_ERR;
	}

	printf("tc_net_listen_fd_n: PASS\n");
	nw_total_pass++;
}

/**
   * @fn                   :tc_net_listen_backlog_p
   * @brief                :
   * @scenario             :
   * API's covered         :listen()
   * Preconditions         :
   * Postconditions        :
   * @return               :void
   */
static void tc_net_listen_backlog_p(void)
{
	struct sockaddr_in sa;
	int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	memset(&sa, 0, sizeof sa);

	sa.sin_family = AF_INET;
	sa.sin_port = htons(1100);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(SocketFD, (struct sockaddr *)&sa, sizeof(sa));
	int ret = (listen(SocketFD, -1));
	close(SocketFD);

	if (ret == -1) {
		printf("tc_net_listen_backlog_p: fail\n");

		nw_total_fail++;
		RETURN_ERR;
	}

	printf("tc_net_listen_backlog_p: PASS\n");
	nw_total_pass++;
}

/**
   * @fn                   :tc_net_listen_fd_backlog_n
   * @brief                :
   * @scenario             :
   * API's covered         :listen()
   * Preconditions         :
   * Postconditions        :
   * @return               :void
   */
static void tc_net_listen_fd_backlog_n(void)
{
	struct sockaddr_in sa;
	int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	memset(&sa, 0, sizeof sa);

	sa.sin_family = AF_INET;
	sa.sin_port = htons(1100);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(SocketFD, (struct sockaddr *)&sa, sizeof(sa));
	int ret = (listen(-1, -1));
	if (ret == 0) {
		printf("tc_net_listen_fd_backlog_n: fail\n");

		nw_total_fail++;
		RETURN_ERR;
	}

	printf("tc_net_listen_fd_backlog_n: PASS\n");
	nw_total_pass++;
}

/****************************************************************************
 * Name: listen()
 ****************************************************************************/

int net_listen_main(void)
{
	tc_net_listen_p();
	tc_net_listen_fd_n();
	tc_net_listen_backlog_p();
	tc_net_listen_fd_backlog_n();
	return 0;
}
