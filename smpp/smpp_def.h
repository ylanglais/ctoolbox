#ifndef _smpp_def_h_
#define _smpp_def_h_

typedef enum {
	bind_receiver 		= 0x00000001,
	bind_transmitter 	= 0x00000002,
	query_sm 			= 0x00000003,
	submit_sm 			= 0x00000004,
	deliver_sm 			= 0x00000005,
	unbind 				= 0x00000006,
	replace_sm 			= 0x00000007,
	cancel_sm 			= 0x00000008,
	bind_transceiver 	= 0x00000009,
	outbind 			= 0x0000000B,
	enquire_link 		= 0x00000015,
	submit_multi 		= 0x00000021,
	alert_notification 	= 0x00000102,
	data_sm 			= 0x00000103
} smpp_commands_e;

typedef enum { 
	smppRECEIVER     = bind_receiver, 
	smppTRANSMITTER  = bind_transmitter, 
	smppTRANSCIEVER  = bind_transceiver
} esme_type_e;

#define PDUSIZE 1000


#endif
