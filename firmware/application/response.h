#ifndef __RESPONSE_H__
#define __RESPONSE_H__


extern comm_packet_head_t reply_nodata;


#define SEND_RESP_SINGLE	0
#define SEND_RESP_MULTIPLE	1


extern int _host_send_response(int mode, comm_packet_head_t *replyhead, void *data, const int count);


/* function to send a response by a single calling. */
static inline int host_send_response(comm_packet_head_t *replyhead, void *data, const int count)
{
	return _host_send_response(SEND_RESP_SINGLE, replyhead, data, count);
}


/* 
   function to send data responses by multiple calling.
   Use it to send unpredictable length of data such as image data during scanning.
   After multiple calling of this function, a calling to "host_send_response"				   .
   should be followed to terminate current command/response session.
 */
static inline int host_send_response_multiple(comm_packet_head_t *replyhead, void *data, const int count)
{
	return _host_send_response(SEND_RESP_MULTIPLE, replyhead, data, count);
}


static inline void resp_packet_init(comm_packet_head_t *replyhead, const comm_packet_head_t *cmdhead)
{
	memset(replyhead, 0, sizeof(comm_packet_head_t));
	replyhead->deviceid = DEVICE_ID; 
	replyhead->command = cmdhead->command;
}


#endif /* __RESPONSE_H__ */
