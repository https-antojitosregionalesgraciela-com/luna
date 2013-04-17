#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "fast-tg.h"
#include "server.h"
#include "client.h"

/* valid command line options for getopt */
#define CLI_OPTS "sc:p:"



int main(int argc, char *argv[])
{
	struct addrinfo addrhints;
	addrhints.ai_family = AF_UNSPEC;
	addrhints.ai_socktype = SOCK_DGRAM;
	addrhints.ai_protocol = IPPROTO_UDP;
	//	addrhints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG;
	addrhints.ai_flags = AI_V4MAPPED;
	addrhints.ai_addrlen = 0;
	addrhints.ai_addr = NULL;
	addrhints.ai_canonname = NULL;
	addrhints.ai_next = NULL;

	int server = 0;
	int client = 0;
	/* port and host will be allocated by strdup, free'd below. */
	char *port = NULL;
	char *host = NULL;
	for (int opt = getopt(argc, argv, CLI_OPTS);
	     opt != -1;
	     opt = getopt(argc, argv, CLI_OPTS))
	{
		switch (opt)
		{
		case 'p':
			port = strdup(optarg);
			break;
		case 's': // act as server
			if (client != 0)
			{
				fprintf(stderr, "Select client or server mode, never both!\n");
				exit(EXIT_INVALID);
			}
			server = 1;
			addrhints.ai_family = AF_INET6;
			break;
		case 'c': // act as client
			if (server != 0)
			{
				fprintf(stderr, "Select client or server mode, never both!\n");
				exit(EXIT_INVALID);
			}
			host = strdup(optarg);
			client = 1;
			break;
		default:
			break;
		}
	}

	if (port == NULL)
	{
		port = malloc(DEFAULT_PORT_LEN);
		snprintf(port, DEFAULT_PORT_LEN, "%u", DEFAULT_PORT);
	}

	if (server)
		addrhints.ai_flags |= AI_PASSIVE;
	else
		if (host == NULL)
		{
			fprintf(stderr, "You must either use server mode or specify a server to send to (-c HOST)!\n");
			exit(EXIT_INVALID);
		}

	/* res will be allocated by getaddrinfo and free'd in
	 * client/server functions. */
	struct addrinfo *res = NULL;
	int error = 0;
	error = getaddrinfo(host, port, &addrhints, &res);
	if (error != 0)
	{
		fprintf(stderr, "Error in getaddrinfo for \"%s\": %s\n", host,
			gai_strerror(error));
	}

	free(host);
	free(port);

	if (client)
		return run_client(res, 1000, 4, 20);
	if (server)
		return run_server(res);
}
