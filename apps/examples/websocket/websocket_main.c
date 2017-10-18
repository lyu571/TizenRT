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
/****************************************************************************
 * examples/websocket/websocket_main.c
 *
 *   Copyright (C) 2016 SAMSUNG ELECTRONICS CO., LTD. All rights reserved.
 *   Author: Jisuu Kim <jisuu.kim@samsung.com>
 *
 *   Copyright (C) 2008, 2011-2012 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
* @testcase		websocket_ws_01
* @brief		To run websocket example. Test packet size and number can be modified as parameters.
*			parameters:
*			serverip: websocket server IP address
*			80: port number
*			NULL: URI address. Websocket doesn't have regulation for URI.
*			0: TLS disabled
*			128: test packet size
*			10: test number
* @scenario		1. Start websocket server at TASH using the command "websocket server 0"
*			2. Start websocket client at TASH using the command "websocket client [serverip] 80 \ 0 128 10".
* @apicovered
* @precondition		Connect to Wi-Fi. Both ARTIK051 server and ARTIK051 client should be in the same network.
* @postcondition
*/

/**
* @testcase		websocket_wss_01
* @brief		To run websocket example with TLS enabled. Test packet size and number can be modified as parameters.
*			parameters:
*			serverip: ARTIK051 websocket server IP address
*			443: port number. Web Secure Socket(WSS) uses 443 port.
*			NULL: URI address. Websocket doesn't have regulation for URI.
*			0: TLS disabled
*			128: test packet size
*			10: test number
* @scenario		1. Start websocket server at TASH using the command "websocket server 1"
*			2. Start websocket client at TASH using the command "websocket client [serverip] 443 \ 1 128 10".
* @apicovered
* @precondition		Connect to Wi-Fi. Both ARTIK051 server and ARTIK051 client should be in the same network.
* @postcondition
*/

/**
* @testcase		http_ws_01 (client)
* @brief		To run HTTP server and websocket client. Test packet size and number can be modified as parameters.
* @scenario		1. Start webserver at TASH using the command "webserver start".  Refer to webserver_main.c to run HTTP server.
*			2. Start websocket client at TASH using this command "websocket client [serverip] 80 \ 0 128 10".
* @apicovered
* @precondition		Connect to Wi-Fi. Both ARTIK051 server and ARTIK051 client should be in the same network.
* @postcondition
*/

/**
* @testcase		http_wss_01 (client)
* @brief		To run HTTPS server and secure websocket client. Test packet size and number can be modified as parameters.
* @scenario		1. Start webserver at TASH using the command "webserver start".  Refer to webserver_main.c to run HTTP server.
*			2. Start websocket client at TASH using this command "websocket client [serverip] 443 \ 1 128 10".
* @apicovered
* @precondition		Connect to Wi-Fi. Both ARTIK051 server and ARTIK051 client should be in the same network.
* @postcondition
*/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <tinyara/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <apps/netutils/websocket.h>

#include <tls/config.h>
#include <tls/entropy.h>
#include <tls/ctr_drbg.h>
#include <tls/certs.h>
#include <tls/x509.h>
#include <tls/ssl.h>
#include <tls/net.h>
#include <tls/error.h>
#include <tls/debug.h>
#include <tls/ssl_cache.h>

#include <sys/socket.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define WEBSOCKET_EXAMPLE_STACKSIZE (1024 * 10)

/*
 * TLS debug configure (0 ~ 5)
 *
 * This configuration is good to debug TLS handshake state. But, more than
 * 2 level of debug sometimes make recv timeout. So we recommend to set
 * WEBSOCKET_SOCK_RCV_TIMEOUT than 5 seconds with debug level.
 * (in include/netutils/websocket.h)
 */
#define MBEDTLS_DEBUG_LEVEL 0

#define WEBSOCKET_USAGE										\
	"\n"													\
	"  websocket server usage:\n"							\
	"   $ websocket server [tls option]\n"					\
	"\n"													\
	"  websocket client usage:\n"							\
	"   $ websocket client [addr] [port] [path] [tls option] [size] [num]\n"	\
	"\n"													\
	"   [tls option] : %%d (0 - disable / 1 - enable)\n"		\
	"   [addr]       : %%s (IPv4 address or Domain name)\n"	\
	"   [path]       : %%s (Page address or zero)\n"			\
	"   [size]       : %%d (Test packet size)\n"				\
	"   [num]        : %%d (Test packet receive and send count)\n"	\
	"\n\n"													\
	"  examples:\n"											\
	"   $ websocket server 1\n"								\
	"   $ websocket client 127.0.0.1 443 0 1 100 10\n"

/****************************************************************************
 * Private Data
 ****************************************************************************/

int received_cnt;
int g_wsenabled;

/****************************************************************************
 * Public Functions
 ****************************************************************************/

static void websocket_tls_debug(void *ctx, int level, const char *file, int line, const char *str)
{
	printf("%s:%04d: %s", file, line, str);
}

websocket_return_t websocket_tls_init(int param, websocket_t *data, mbedtls_ssl_config *conf, mbedtls_x509_crt *cert, mbedtls_pk_context *pkey, mbedtls_entropy_context *entropy, mbedtls_ctr_drbg_context *ctr_drbg, mbedtls_ssl_cache_context *cache)
{
	int r;
	const char *crt = mbedtls_test_srv_crt;
	const char *key = mbedtls_test_srv_key;
	const char *ca_crt = mbedtls_test_cas_pem;
	size_t crt_len = mbedtls_test_srv_crt_len;
	size_t cacrt_len = mbedtls_test_cas_pem_len;
	size_t key_len = mbedtls_test_srv_key_len;

	if (param) {
		crt = mbedtls_test_cli_crt;
		key = mbedtls_test_cli_key;
		ca_crt = mbedtls_test_cas_pem;
		crt_len = mbedtls_test_cli_crt_len;
		cacrt_len = mbedtls_test_cas_pem_len;
		key_len = mbedtls_test_cli_key_len;
	}

	/* initialize tls context for server */
	mbedtls_ssl_config_init(conf);
	mbedtls_x509_crt_init(cert);
	mbedtls_pk_init(pkey);
	mbedtls_entropy_init(entropy);
	mbedtls_ctr_drbg_init(ctr_drbg);

	/* 1. Load the certificates and private RSA key */
	printf("  . Loading the cert. and key...");

	/* S/W Certificiate */
	if ((r = mbedtls_x509_crt_parse(cert, (const unsigned char *)crt, crt_len)) != 0) {
		printf("Error: mbedtls_x509_crt_parse when read crt returned %d\n", r);
		return WEBSOCKET_INIT_ERROR;
	}

	if ((r = mbedtls_x509_crt_parse(cert, (const unsigned char *)ca_crt, cacrt_len)) != 0) {
		printf("Error: mbedtls_x509_crt_parse when read caspem returned %d\n", r);
		return WEBSOCKET_INIT_ERROR;
	}

	if ((r = mbedtls_pk_parse_key(pkey, (const unsigned char *)key, key_len, NULL, 0)) != 0) {
		printf("Error: mbedtls_pk_parse_key returned %d\n", r);
		return WEBSOCKET_INIT_ERROR;
	}

	printf("Ok\n");

	/* 2. Seed the RNG */
	printf("  . Seeding the random number generator...");

	if ((r = mbedtls_ctr_drbg_seed(ctr_drbg, mbedtls_entropy_func, entropy, NULL, 0)) != 0) {
		printf("Error: mbedtls_ctr_drbg_seed returned %d\n", r);
		return WEBSOCKET_INIT_ERROR;
	}

	printf("Ok\n");

	/* 3. Setup ssl stuff */
	printf("  . Setting up the SSL data...");

	if ((r = mbedtls_ssl_config_defaults(conf, param ? MBEDTLS_SSL_IS_CLIENT : MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
		printf("Error: mbedtls_ssl_config_defaults returned %d\n", r);
		return WEBSOCKET_INIT_ERROR;
	}

	mbedtls_ssl_conf_rng(conf, mbedtls_ctr_drbg_random, ctr_drbg);
	mbedtls_ssl_conf_dbg(conf, websocket_tls_debug, stdout);
	if (!param) {
		mbedtls_ssl_cache_init(cache);
		mbedtls_ssl_conf_session_cache(conf, cache, mbedtls_ssl_cache_get, mbedtls_ssl_cache_set);
	}

	mbedtls_ssl_conf_ca_chain(conf, cert->next, NULL);
	if ((r = mbedtls_ssl_conf_own_cert(conf, cert, pkey)) != 0) {
		printf("Error: mbedtls_ssl_conf_own_cert returned %d\n", r);
		return WEBSOCKET_INIT_ERROR;
	}
	data->tls_conf = conf;

	printf("Ok\n");
	return WEBSOCKET_SUCCESS;
}

void websocket_tls_release(int param, mbedtls_ssl_config *conf, mbedtls_x509_crt *cert, mbedtls_pk_context *pkey, mbedtls_entropy_context *entropy, mbedtls_ctr_drbg_context *ctr_drbg, mbedtls_ssl_cache_context *cache)
{
	if (!param) {
		mbedtls_ssl_cache_free(cache);
	}
	mbedtls_ctr_drbg_free(ctr_drbg);
	mbedtls_entropy_free(entropy);
	mbedtls_pk_free(pkey);
	mbedtls_x509_crt_free(cert);
	mbedtls_ssl_config_free(conf);
}

/****************************************************************************
 * websocket_main
 ****************************************************************************/

/* receive packets from TCP socket */
ssize_t recv_cb(websocket_context_ptr ctx, uint8_t *buf, size_t len, int flags, void *user_data)
{
	ssize_t r;
	int fd;
	int retry_cnt = 3;
	struct websocket_info_t *info = user_data;

	fd = info->data->fd;
RECV_RETRY:
	if (info->data->tls_enabled) {
		r = mbedtls_ssl_read(info->data->tls_ssl, buf, len);
	} else {
		r = recv(fd, buf, len, 0);
	}

	if (r == 0) {
		websocket_set_error(info->data, WEBSOCKET_ERR_CALLBACK_FAILURE);
	} else if (r < 0) {
		printf("recv err : %d\n", errno);
		if (retry_cnt == 0) {
			websocket_set_error(info->data, WEBSOCKET_ERR_CALLBACK_FAILURE);
			return r;
		}
		retry_cnt--;
		goto RECV_RETRY;
	}

	return r;
}

/* send packets from TCP socket */
ssize_t send_cb(websocket_context_ptr ctx, const uint8_t *buf, size_t len, int flags, void *user_data)
{
	ssize_t r;
	int fd;
	int retry_cnt = 3;
	struct websocket_info_t *info = user_data;

	fd = info->data->fd;
SEND_RETRY:
	if (info->data->tls_enabled) {
		r = mbedtls_ssl_write(info->data->tls_ssl, buf, len);
	} else {
		r = send(fd, buf, len, flags);
	}

	if (r < 0) {
		printf("send err : %d\n", errno);
		if (retry_cnt == 0) {
			websocket_set_error(info->data, WEBSOCKET_ERR_CALLBACK_FAILURE);
			return r;
		}
		retry_cnt--;
		goto SEND_RETRY;
	}

	return r;
}

int genmask_cb(websocket_context_ptr ctx, uint8_t *buf, size_t len, void *user_data)
{
	memset(buf, rand(), len);
	return 0;
}

void print_on_msg_cb(websocket_context_ptr ctx, const websocket_on_msg_arg *arg, void *user_data)
{
	websocket_frame_t msgarg = {
		arg->opcode, arg->msg, arg->msg_length
	};

	/* Echo back non-closing message */
	if (WEBSOCKET_CHECK_NOT_CTRL_FRAME(arg->opcode)) {
		received_cnt++;			//to check communication is done.
		printf("on_msg length : %d print : %s, [#%d]\n", msgarg.msg_length, msgarg.msg, received_cnt);
	} else if (WEBSOCKET_CHECK_CTRL_PING(arg->opcode)) {
		printf("cli got ping\n");
	} else if (WEBSOCKET_CHECK_CTRL_PONG(arg->opcode)) {
		printf("cli got pong\n");
	}
}

void echoback_on_msg_cb(websocket_context_ptr ctx, const websocket_on_msg_arg *arg, void *user_data)
{
	struct websocket_info_t *info = user_data;
	websocket_frame_t msgarg = {
		arg->opcode, arg->msg, arg->msg_length
	};

	/* Echo back non-closing message */
	if (WEBSOCKET_CHECK_NOT_CTRL_FRAME(arg->opcode)) {
		websocket_queue_msg(info->data, &msgarg);
		//echo back the message
	} else if (WEBSOCKET_CHECK_CTRL_CLOSE(arg->opcode)) {
		printf("echoback_on_msg received close message\n");
	} else if (WEBSOCKET_CHECK_CTRL_PING(arg->opcode)) {
		printf("srv got ping\n");
	} else if (WEBSOCKET_CHECK_CTRL_PONG(arg->opcode)) {
		printf("srv got pong\n");
	}
}

/*
 *  recv_start_cb for example
 *  this cb will be invoked when a new frame starts to be received.
 */
void recv_start_cb(websocket_context_ptr ctx, const websocket_recv_start_arg *arg, void *user_data)
{
	if (arg->fin == 1) {
		//this frame is the last.
	} else if (arg->fin == 0) {
		//this frame is fragmented.
	}
	if (WEBSOCKET_CHECK_NOT_CTRL_FRAME(arg->opcode)) {
		//it's not a control frame. You can find opcodes from apps/include/netutils/websocket.h
	}
}

/*
 *  recv_chunk_cb for example
 *  this cb will be invoked when a chunk of frame payload is received.
 */
void recv_chunk_cb(websocket_context_ptr ctx, const websocket_recv_chunk_arg *arg, void *user_data)
{
	if (arg->data_length > 0) {
		//you can handle the payload (arg->data)
	}
}

/*
 *  recv_end_cb for example
 *  this cb will be invoked when a frame is completely received.
 */
void recv_end_cb(websocket_context_ptr ctx, void *user_data)
{
	printf("receive done\n");
	//receive done.
}

void on_connectivity_changes(websocket_context_ptr ctx, enum websocket_connection_state state, void *user_data)
{
	switch (state) {
	case WEBSOCKET_CLOSED:
		printf("Websocket closed\n");
		break;
	case WEBSOCKET_CONNECTED:
		printf("Websocket connected\n");
		break;
	}

}

/* websocket client prints received messages from a server using recv message cb */
int websocket_client(void *arg)
{
	int i;
	int r = WEBSOCKET_SUCCESS;
	char **argv = arg;
	char *addr = NULL;
	char *port = NULL;
	char *path = NULL;
	int tls = atoi(argv[3]);
	int size = atoi(argv[4]);
	int send_cnt = atoi(argv[5]);
	websocket_frame_t *tx_frame = NULL;
	websocket_t *websocket_cli = NULL;
	char *test_message = NULL;
	static struct websocket_cb_t cb = {
		recv_cb,				/* recv callback */
		send_cb,				/* send callback */
		genmask_cb,				/* gen mask callback */
		NULL,					/* recv frame start callback */
		NULL,					/* recv frame chunk callback */
		NULL,					/* recv frame end callback */
		print_on_msg_cb,			/* recv message callback */
		on_connectivity_changes			/* connectivity changes callback */
	};

	mbedtls_ssl_config conf;
	mbedtls_x509_crt cert;
	mbedtls_pk_context pkey;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_ssl_cache_context cache;

	if (size < 16) {
		printf("wrong size\n %s\n", WEBSOCKET_USAGE);
		return -1;
	}
	if (send_cnt < 1) {
		printf("wrong send count\n %s\n", WEBSOCKET_USAGE);
		return -1;
	}

	addr = malloc(strlen(argv[0]) + 1);
	if (addr == NULL) {
		printf("fail to allocate memory\n");
		goto WEB_CLI_EXIT;
	}
	port = malloc(strlen(argv[1]) + 1);
	if (port == NULL) {
		printf("fail to allocate memory\n");
		goto WEB_CLI_EXIT;
	}
	path = malloc(strlen(argv[2]) + 1);
	if (path == NULL) {
		printf("fail to allocate memory\n");
		goto WEB_CLI_EXIT;
	}

	memset(addr, 0, strlen(argv[0]) + 1);
	memset(port, 0, strlen(argv[1]) + 1);
	memset(path, 0, strlen(argv[2]) + 1);

	strncpy(addr, argv[0], strlen(argv[0]));
	strncpy(port, argv[1], strlen(argv[1]));
	strncpy(path, argv[2], strlen(argv[2]));

	received_cnt = 0;
	websocket_cli = malloc(sizeof(websocket_t));
	if (websocket_cli == NULL) {
		printf("fail to allocate memory\n");
		goto WEB_CLI_EXIT;
	}
	memset(websocket_cli, 0, sizeof(websocket_t));

	websocket_cli->fd = -1;
	websocket_cli->cb = &cb;
	websocket_cli->tls_enabled = tls;

	/* TLS init routine */
	if (tls) {
#ifdef MBEDTLS_DEBUG_C
		mbedtls_debug_set_threshold(MBEDTLS_DEBUG_LEVEL);
#endif
		if ((r = websocket_tls_init(1, websocket_cli, &conf, &cert, &pkey, &entropy, &ctr_drbg, &cache)) != WEBSOCKET_SUCCESS) {
			printf("fail to init TLS, error: %d\n", r);
			goto WEB_CLI_EXIT;
		}
		websocket_cli->tls_ssl = malloc(sizeof(mbedtls_ssl_context));
		if (websocket_cli->tls_ssl == NULL) {
			printf("fail to allocate memory\n");
			goto WEB_CLI_EXIT;
		}

		mbedtls_net_init(&(websocket_cli->tls_net));
		mbedtls_ssl_init(websocket_cli->tls_ssl);
	}

	printf("start opening websocket client to addr: %s, port: %s, Security: %s\n", addr, port, tls ? "enabled" : "disabled");

	r = websocket_client_open(websocket_cli, addr, port, path);
	if (r != WEBSOCKET_SUCCESS) {
		printf("websocket_client_open returned error: %d\n", r);
		goto WEB_CLI_EXIT;
	}

	test_message = malloc(size);
	if (test_message == NULL) {
		goto WEB_CLI_EXIT;
	}
	memset(test_message, '.', size);

	sprintf(test_message, "[%d] websocket", size);
	test_message[size - 1] = '\0';

	tx_frame = malloc(sizeof(websocket_frame_t));
	if (tx_frame == NULL) {
		printf("fail to allocate memory\n");
		goto WEB_CLI_EXIT;
	}
	memset(tx_frame, 0, sizeof(websocket_frame_t));

	tx_frame->opcode = WEBSOCKET_TEXT_FRAME;
	tx_frame->msg = (const uint8_t *)test_message;
	tx_frame->msg_length = size;

	/* send and receive small message */
	for (i = 1; i <= send_cnt; i++) {

		r = websocket_queue_msg(websocket_cli, tx_frame);
		if (r != WEBSOCKET_SUCCESS) {
			printf("websocket_queue_msg returned error : %d\n", r);
			goto WEB_CLI_EXIT;
		}

		/* wait for server echoes back */
		while (received_cnt != i) {
			if (websocket_cli->state == WEBSOCKET_STOP) {
				goto WEB_CLI_EXIT;
			}
			usleep(100000);
		}
	}

	/* wait until every message is tested. */
	while (websocket_cli->state == WEBSOCKET_RUNNING) {
		/* all echo back message received */
		if (received_cnt == send_cnt) {
			printf("all message was received well\n");
			break;
		}
		usleep(100000);
	};
WEB_CLI_EXIT:
	r = websocket_queue_close(websocket_cli, NULL);
	if (r != WEBSOCKET_SUCCESS) {
		printf("websocket_close returned error : %d\n", r);
	}
	printf("websocket client %s [%d]\n", (r ? "error" : "finished"), r);

	if (tx_frame) {
		free(tx_frame);
	}

	if (tls) {
		websocket_tls_release(1, &conf, &cert, &pkey, &entropy, &ctr_drbg, &cache);
		if (websocket_cli->tls_ssl) {
			mbedtls_ssl_free(websocket_cli->tls_ssl);
			free(websocket_cli->tls_ssl);
		}
	}

	if (websocket_cli) {
		free(websocket_cli);
	}
	if (addr) {
		free(addr);
	}
	if (port) {
		free(port);
	}
	if (path) {
		free(path);
	}
	if (test_message) {
		free(test_message);
	}

	return r;
}

/* websocket server echoes back messages from a client using recv message cb */
int websocket_server(void *arg)
{
	int r;
	char **argv = arg;
	int tls = atoi(argv[0]);
	static struct websocket_cb_t cb = {
		recv_cb,				/* recv callback */
		send_cb,				/* send callback */
		NULL,					/* gen mask callback */
		NULL,					/* recv frame start callback */
		NULL,					/* recv frame chunk callback */
		NULL,					/* recv frame end callback */
		echoback_on_msg_cb		/* recv message callback */
	};

	if (tls != 0 && tls != 1) {
		printf("wrong tls option\n %s\n", WEBSOCKET_USAGE);
		return WEBSOCKET_INIT_ERROR;
	}

	if (g_wsenabled) {
		printf("\nWebsocket server is already running\n");
		return WEBSOCKET_INIT_ERROR;
	} else {
		g_wsenabled = 1;
	}

	mbedtls_ssl_config conf;
	mbedtls_x509_crt cert;
	mbedtls_pk_context pkey;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_ssl_cache_context cache;

	websocket_t *websocket_srv = malloc(sizeof(websocket_t));
	if (websocket_srv == NULL) {
		printf("fail to allocate memory\n");
		g_wsenabled = 0;
		return WEBSOCKET_ALLOCATION_ERROR;
	}

	memset(websocket_srv, 0, sizeof(websocket_t));

	websocket_srv->fd = -1;
	websocket_srv->cb = &cb;
	websocket_srv->tls_enabled = tls;

	/* TLS init routine */
	if (tls) {
#ifdef MBEDTLS_DEBUG_C
		mbedtls_debug_set_threshold(MBEDTLS_DEBUG_LEVEL);
#endif
		if ((r = websocket_tls_init(0, websocket_srv, &conf, &cert, &pkey, &entropy, &ctr_drbg, &cache)) != WEBSOCKET_SUCCESS) {
			printf("fail to init TLS\n");
			goto WEB_SRV_EXIT;
		}
	}

	printf("websocket server is starting\n");

	/* Start Websocket server program */
	if ((r = websocket_server_open(websocket_srv)) != WEBSOCKET_SUCCESS) {
		printf("websocket_server_open returned error : %d\n", r);
		goto WEB_SRV_EXIT;
	}

WEB_SRV_EXIT:
	if (tls) {
		websocket_tls_release(0, &conf, &cert, &pkey, &entropy, &ctr_drbg, &cache);
	}

	if (websocket_srv) {
		free(websocket_srv);
	}

	g_wsenabled = 0;
	printf("websocket server is finished\n");
	return 0;
}

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int websocket_main(int argc, char *argv[])
#endif
{
	int status;
	pthread_attr_t attr;
	pthread_t tid;

	if ((status = pthread_attr_init(&attr)) != 0) {
		printf("fail to init thread\n");
		return -1;
	}
	pthread_attr_setstacksize(&attr, WEBSOCKET_EXAMPLE_STACKSIZE);
	pthread_attr_setschedpolicy(&attr, WEBSOCKET_SCHED_POLICY);

	if (memcmp(argv[1], "client", strlen("client")) == 0 && argc == 8) {
		if ((status = pthread_create(&tid, &attr, (pthread_startroutine_t)websocket_client, (void *)(argv + 2))) != 0) {
			printf("fail to create thread\n");
			return -1;
		}
		pthread_setname_np(tid, "websocket client");
		pthread_detach(tid);
	} else if (memcmp(argv[1], "server", strlen("server")) == 0 && argc == 3) {
		if ((status = pthread_create(&tid, &attr, (pthread_startroutine_t)websocket_server, (void *)(argv + 2))) != 0) {
			printf("fail to create thread\n");
			return -1;
		}
		pthread_setname_np(tid, "websocket server");
		pthread_detach(tid);
	} else {
		printf("\nwrong input parameter !!!\n %s\n", WEBSOCKET_USAGE);
		return -1;
	}
	return 0;
}
