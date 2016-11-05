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

#include <stdio.h>
/* === Constants === */

/* === Macros === */

#define DEBUG(...) do { fprintf(stderr, __VA_ARGS__); } while(0)
#else
#define DEBUG(...)
#endif
/* === Type and structure definitions === */
 
/* === Prototypes === */

/**
 * @brief terminate program on error
 * @param exitcode exit code
 * @param fmt format string
**/
/**
 * @brief parse client arguments into options
 * @param argc argument count
 * @param argv argument vector
 * @param opts client options structure to store information in
**/
/**
 * @brief creates and connects a socket
 * @param host host address
 * @param service service / port
 * @return file descriptor
**/
/**
 * @brief reads a given number of bytes from server
 * @param fd file descriptor
 * @param buffer write buffer
 * @param n number of bytes to read
 * @return pointer to buffer on success, NULL otherwise
**/
/**
 * @brief calculates a unique permutation from its index
 * @param idx index
 * @return unique color permutation based on index
**/
/**
 * @brief calculates next choice based on elimination
 * @param last_guess the last played guess
 * @param master_answer_red number of red pins received
 * @param master_answer_white number of white pins received
 * @return next guess to play
**/
/**
 * @brief close connection and free resources
**/
/**
 * @brief compare two guesses based on the last answer
 * @param a first guess
 * @param b second guess
 * @param given_red red pins
 * @param given_white white pins
 * @return true if mastermind would give the same answer, false otherwise
**/
/**
 * @brief extracts single color from transmission structure
 * @param prot protocol received from server or generated
 * @param color_index whic index to extract (< 5)
 * @return stored color information
**/
/**
 * @brief build a transmission structure from color arguments
 * @param arguments array of 5 colors according to enum
 * @return transmission structure
**/
/**
 * @brief decrypt a server answer according to specification
 * @param answer given server answer byte
 * @param red pin out
 * @param white white pin out
 * @param parity_error parity error bit out
 * @param lost_error lost error bit out
**/
 * @brief program entry point
 * @param argc argument count
 * @param argv argument vector
 * @return EXIT_SUCCESS on win, EXIT_PARITY_ERROR on parity error,
 * EXIT_GAME_LOST on lose, EXIT_MULTIPLE_ERRORS on parity error and lose,
 * EXIT_FAILURE on generic error (see stderr)
**/
	/* create communication framework */
	/* loop until won or the server asks to terminate */
			/* In the first round play a suitible, but random guess */
			/* In all other rounds calculate a strategic guess based on what the mastermind tells us */
		/* Await server response and evaluate */
		/* Check for termination signals */
			/* EXIT_PARITY_ERROR */
			/* EXIT_GAME_LOST */
			/* Win condition */
	/* shift colors into the right spot */
	/* calculate parity */
	/* Loop through all permutation */
		/* if we eliminated our guess already, just skip it */
			/* check if our guess is still eligible and eliminate it otherwise */
				/* we select the last index that is still eligible */
				/* we could use a min/max strategy here to decrease our maximum rounds */
	/* close the socket, free the flag field */
	/* address hints in order to specify our endpoint */
	/* connect to server */
	/* we dont want the last bit, i.e. the parity bit to be set because it would create duplicates */
	/* just setting the variable to our index will guarantee a unique one for each index */
	uint16_t ret = idx;
	/* calculate the parity bit */
	/* just use bit operations to read the index */
	/* just use bit operations to read the index */
	/* use a similar algorithm to the one the mastermind uses to compare answers */
		/* count red pins */
		/* count white pins from the remaining colors */
	/* actual check for equality (class equality) */
	/* the port is a string, because it could specify a ai_socktype conform service name e.g. ssh*/