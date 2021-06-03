#ifndef _msg_defs_h_
#define _msg_defs_h_

typedef enum {
	merr_NONE,
	merr_EMPTY,
	merr_NO_MEMORY,
	merr_REQUEUE,
	merr_BAD_QUEUE_SERVER,
	merr_BAD_QUEUE_CLIENT,
	merr_BAD_QUEUE,
	merr_BAD_QUEUE_ENTRY,
	merr_QUEUE_ERROR,
	merr_INVALID_CHECKSUM,
	merr_INVALID_PROTOCOL,
	merr_NULL_MESSAGE,
	merr_INCORRECT_MESSAGE,
	merr_BAD_MESSAGE,
	merr_PARTIAL_HEADER,
	merr_PARTIAL_MESSAGE,
	merr_MAX_RETRY,
	merr_UNKNOWN_COMMAND
} merr_t;

typedef enum {
	msg_OK,  /* Status OK                */
	msg_KO,  /* Status KO                */

	msg_BYE, /* Closing connexion        */

	msg_PUT, /* Put data                 */
	msg_GET, /* Get data                 */

	msg_PIG, /* Echo request             */
	msg_POG, /* Echo reply               */

	msg_DUP, /* Duplication request      */
	msg_SVG, /* Duplication              */
    msg_SYT, /* Sync request             */
    msg_SYN, /* Sync reply               */

	msg_RTT, /* Reroot request           */
	msg_RTN, /* Reroot notification      */

	msg_STT, /* Status request           */
	msg_STY, /* Status reply             */

	msg_TRT, /* Termination request      */
	msg_TRN, /* Termination notification */ 

	msg_LCK, /* Lock queue               */
	msg_ULK, /* Unlock queue             */

	msg_POZ, /* Pause queue              */
	msg_RUN, /* Rerun paused queue       */

	msg_BAD  /* BAD or INVALID message   */
} msg_command_t;

typedef enum {
	act_NONE,
	act_RETRY,
	act_EMPTY,
	act_BYE
} msg_ko_action_t; 

#endif
