/**
 * @file client.c
 * @author Philipp Auer <e1420446@student.tuwien.ac.at>
 * @date 31.10.2016
 *
 * @brief Client module.
 *
 * This is the client of the mastermind task of my OSUE course.
 * It implements basic communication via TCP sockets and a very basic algorithm
 * to solve a given mastermind game.
**/

/*
 * Copyright (c) 2016 Philipp Auer <e1420446@student.tuwien.ac.at>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <stdio.h>#include <stdlib.h>#include <string.h>#include <stdint.h>#include <unistd.h>#include <stdarg.h>#include <sys/types.h>#include <sys/socket.h>#include <netinet/in.h>#include <errno.h>#include <limits.h>#include <netdb.h>#include <stdbool.h>#include <assert.h>
/* === Constants === */
#define SLOTS (5)#define EXIT_PARITY_ERROR (2)#define EXIT_GAME_LOST (3)#define EXIT_MULTIPLE_ERRORS (4)#define MAX_PERM (32768)#define MAX_COLOR (8)
/* === Macros === */
#ifdef ENDEBUG
#define DEBUG(...) do { fprintf(stderr, __VA_ARGS__); } while(0)
#else
#define DEBUG(...)
#endif
/* === Type and structure definitions === */
 struct client_options{	char *port;	char *hostname;};struct client_protocol_args{	uint8_t color[5];};
/* === Prototypes === */

/**
 * @brief terminate program on error
 * @param exitcode exit code
 * @param fmt format string
**/static void bail_out(int exitcode, const char *fmt, ...);
/**
 * @brief parse client arguments into options
 * @param argc argument count
 * @param argv argument vector
 * @param opts client options structure to store information in
**/static void parse_client_arguments(int argc, char *argv[],struct client_options *opts);
/**
 * @brief creates and connects a socket
 * @param host host address
 * @param service service / port
 * @return file descriptor
**/static int create_client_socket(const char *host, const char *service);
/**
 * @brief reads a given number of bytes from server
 * @param fd file descriptor
 * @param buffer write buffer
 * @param n number of bytes to read
 * @return pointer to buffer on success, NULL otherwise
**/static uint8_t *read_from_server(int fd, uint8_t *buffer, size_t n);
/**
 * @brief calculates a unique permutation from its index
 * @param idx index
 * @return unique color permutation based on index
**/static uint16_t index_to_answer(uint16_t idx);
/**
 * @brief calculates next choice based on elimination
 * @param last_guess the last played guess
 * @param master_answer_red number of red pins received
 * @param master_answer_white number of white pins received
 * @return next guess to play
**/static uint16_t build_intelligent_answer(uint16_t last_guess,	uint8_t master_answer_red,	uint8_t master_answer_white);
/**
 * @brief close connection and free resources
**/static void close_connection(void);
/**
 * @brief compare two guesses based on the last answer
 * @param a first guess
 * @param b second guess
 * @param given_red red pins
 * @param given_white white pins
 * @return true if mastermind would give the same answer, false otherwise
**/static bool compare_answer(uint16_t a, uint16_t b, int given_red,	int given_white);
/**
 * @brief extracts single color from transmission structure
 * @param prot protocol received from server or generated
 * @param color_index whic index to extract (< 5)
 * @return stored color information
**/static uint8_t color_from_client_protocol(uint16_t prot, uint8_t color_index);
/**
 * @brief build a transmission structure from color arguments
 * @param arguments array of 5 colors according to enum
 * @return transmission structure
**/static uint16_t build_client_protocol(uint8_t *arguments);
/**
 * @brief decrypt a server answer according to specification
 * @param answer given server answer byte
 * @param red pin out
 * @param white white pin out
 * @param parity_error parity error bit out
 * @param lost_error lost error bit out
**/static void decrypt_master_answer(uint8_t answer, uint8_t * red,	uint8_t * white, bool * parity_error,	bool * lost_error);static const char *progname = "client";      /* default name */static int sockfd = -1;						 /* communication file descriptor */static bool *invalid_flags;					 /* flag field for elimination strategy *//**
 * @brief program entry point
 * @param argc argument count
 * @param argv argument vector
 * @return EXIT_SUCCESS on win, EXIT_PARITY_ERROR on parity error,
 * EXIT_GAME_LOST on lose, EXIT_MULTIPLE_ERRORS on parity error and lose,
 * EXIT_FAILURE on generic error (see stderr)
**/int main(int argc, char *argv[]){	int rounds = 0;	struct client_options opts;	parse_client_arguments(argc, argv, &opts);
	/* create communication framework */	sockfd = create_client_socket(opts.hostname, opts.port);	invalid_flags = malloc(MAX_PERM);	(void)memset(invalid_flags, 0, MAX_PERM);	bool quit = false;	int exit_code = EXIT_SUCCESS;	uint16_t current_guess = 0;	uint8_t last_red = 0;	uint8_t last_white = 0;	bool last_parity = 0;	bool last_lost_game = 0;
	/* loop until won or the server asks to terminate */	while (!quit)	{		rounds++;		if (rounds == 1)		{
			/* In the first round play a suitible, but random guess */			enum { beige, darkblue, green, orange, red, black, violet, white };			uint8_t colors[5] = {				beige, darkblue, green, orange, red			};			current_guess = build_client_protocol(colors);		}		else		{
			/* In all other rounds calculate a strategic guess based on what the mastermind tells us */			uint16_t last_guess = current_guess;			current_guess =				build_intelligent_answer(last_guess, last_red, last_white);		}		(void)send(sockfd, &(current_guess), 2, 0);		uint8_t response[1];
		/* Await server response and evaluate */		if (read_from_server(sockfd, response, 1) == NULL)			bail_out(EXIT_FAILURE, "Could not read server response");		uint8_t server_answer = response[0];		decrypt_master_answer(server_answer, &last_red, &last_white,			&last_parity, &last_lost_game);		DEBUG("round #%d\n", rounds);		DEBUG("red: %d; white: %d\n", last_red, last_white);
		/* Check for termination signals */		if (last_parity)		{
			/* EXIT_PARITY_ERROR */			(void)fprintf(stderr, "Parity error\n");			exit_code = EXIT_PARITY_ERROR;			quit = true;		}		if (last_lost_game > 0)		{
			/* EXIT_GAME_LOST */			(void)fprintf(stderr, "Game lost\n");			if (last_parity)				exit_code = EXIT_MULTIPLE_ERRORS;			else				exit_code = EXIT_GAME_LOST;			quit = true;		}		if ((last_red >= SLOTS) && exit_code == EXIT_SUCCESS)		{
			/* Win condition */			quit = true;			(void)printf("Game won after %d rounds\n", rounds);		}	}	close_connection();	return exit_code;}static uint16_t build_client_protocol(uint8_t * arguments){	uint16_t protocol = 0;
	/* shift colors into the right spot */	for (int i = 0; i < 5; ++i)	{		protocol |= arguments[i] << 3 * i;	}	uint8_t parity = 0;
	/* calculate parity */	for (int i = 0; i < 15; ++i)	{		parity ^= (((1 << i) & protocol) > 0 ? 1 : 0);	}	return protocol | (parity << 15);}static uint16_t build_intelligent_answer(uint16_t last_guess, uint8_t master_answer_red,	uint8_t master_answer_white){	int ret_index = -1;	int count_exclude = 0;
	/* Loop through all permutation */	for (int i = 0; i < MAX_PERM; ++i)	{
		/* if we eliminated our guess already, just skip it */		if (!invalid_flags[i])		{
			/* check if our guess is still eligible and eliminate it otherwise */			if (!compare_answer				(last_guess, index_to_answer(i), master_answer_red,					master_answer_white))			{				invalid_flags[i] = true;				count_exclude++;			}			else			{
				/* we select the last index that is still eligible */
				/* we could use a min/max strategy here to decrease our maximum rounds */				ret_index = i;			}		}	}	if (ret_index == -1)	{		/* we should always find a guess since we enumerate all of them */		assert(0);	}	DEBUG("Guess: %d;%d;%d;%d;%d [EXLUDE: %d]\n\n",		color_from_client_protocol(last_guess, 0),		color_from_client_protocol(last_guess, 1),		color_from_client_protocol(last_guess, 2),		color_from_client_protocol(last_guess, 3),		color_from_client_protocol(last_guess, 4),		count_exclude);	return index_to_answer(ret_index);}static uint8_t *read_from_server(int fd, uint8_t * buffer, size_t n){	size_t bytes_recv = 0;	do {		ssize_t r;		r = recv(fd, buffer + bytes_recv, n - bytes_recv, 0);		if (r <= 0) {			return NULL;		}		bytes_recv += r;	} while (bytes_recv < n);	if (bytes_recv < n) {		return NULL;	}	return buffer;}static void close_connection(void){
	/* close the socket, free the flag field */	if (sockfd > 0)		(void) close(sockfd);	free(invalid_flags);	invalid_flags = NULL;}static int create_client_socket(const char *host, const char *service){	struct addrinfo hints;	struct addrinfo *ai, *aip;	int fd, res;
	/* address hints in order to specify our endpoint */	(void)memset(&hints, 0, sizeof(hints));	hints.ai_family = AF_INET;	hints.ai_socktype = SOCK_STREAM;	hints.ai_protocol = IPPROTO_TCP;	hints.ai_flags = AI_PASSIVE;	res = getaddrinfo(host, service, &hints, &ai);	if (res != 0)		bail_out(EXIT_FAILURE, gai_strerror(res));	if (ai == NULL)		bail_out(EXIT_FAILURE, "Could not resolve host %s", host);	aip = ai;	fd = socket(aip->ai_family, aip->ai_socktype, aip->ai_protocol);	if (fd < 0)		bail_out(EXIT_FAILURE, "Could not create socket");
	/* connect to server */	if (connect(fd, aip->ai_addr, aip->ai_addrlen) < 0)	{		freeaddrinfo(ai);		bail_out(EXIT_FAILURE, "Connection failed");	}	freeaddrinfo(ai);	return fd;}static uint16_t index_to_answer(uint16_t idx){
	/* we dont want the last bit, i.e. the parity bit to be set because it would create duplicates */	if (idx > MAX_PERM)		bail_out(EXIT_FAILURE,			"Tried to create client answer from index greater than maximum number of permutations.");	
	/* just setting the variable to our index will guarantee a unique one for each index */
	uint16_t ret = idx;	uint8_t parity = 0;
	/* calculate the parity bit */	for (int i = 0; i <= 14; ++i)	{		parity ^= ((1 << i) & ret ? 1 : 0);	}	return ret | (parity << 15);}static void decrypt_master_answer(uint8_t answer, uint8_t * red, uint8_t * white,	bool * parity_error, bool * lost_error){
	/* just use bit operations to read the index */	*red = answer & 0x7;	*white = (answer & 0x38) >> 3;	*parity_error = (answer & 0x40) > 0;	*lost_error = (answer & 0x80) > 0;}static uint8_t color_from_client_protocol(uint16_t prot, uint8_t color_index){
	/* just use bit operations to read the index */	return ((prot & (7 << 3 * color_index)) >> 3 * color_index);}static bool compare_answer(uint16_t a, uint16_t b, int given_red, int given_white){	uint8_t colors_left[MAX_COLOR];	(void)memset(&colors_left[0], 0, sizeof(colors_left));	int red, white;	red = white = 0;
	/* use a similar algorithm to the one the mastermind uses to compare answers */	for (int i = 0; i < SLOTS; ++i)	{
		/* count red pins */		if (color_from_client_protocol(a, i) == color_from_client_protocol(b, i))			red++;		else			colors_left[color_from_client_protocol(a, i)]++;	}	for (int i = 0; i < SLOTS; ++i)	{
		/* count white pins from the remaining colors */		if (color_from_client_protocol(a, i) != color_from_client_protocol(b, i))		{			if (colors_left[color_from_client_protocol(b, i)] > 0)			{				white++;				colors_left[color_from_client_protocol(b, i)]--;			}		}	}
	/* actual check for equality (class equality) */	if (given_red == red && given_white == white)		return true;	return false;}static void parse_client_arguments(int argc, char *argv[], struct client_options *opts){	if (argc > 0)		progname = argv[0];	if (argc != 3)		bail_out(EXIT_FAILURE, "Usage: %s <server-hostname> <server-port>",		progname);	opts->hostname = argv[1];
	/* the port is a string, because it could specify a ai_socktype conform service name e.g. ssh*/	opts->port = argv[2];}static void bail_out(int exitcode, const char *fmt, ...){	va_list ap;	(void)fprintf(stderr, "%s: ", progname);	if (fmt != NULL) {		va_start(ap, fmt);		(void)vfprintf(stderr, fmt, ap);		va_end(ap);	}	if (errno != 0) {		(void)fprintf(stderr, ": %s", strerror(errno));	}	(void)fprintf(stderr, "\n");	close_connection();	exit(exitcode);}