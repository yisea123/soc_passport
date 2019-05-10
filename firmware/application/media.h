#ifndef __MEDIA_H__
#define __MEDIA_H__


/* media logical state definition */
enum media_state
{
	MEDIA_UNAVAILABLE = 0,	/* no media available */
	MEDIA_AT_FRONT = 1,	/* media is detected at front gate (neither aligned nor in the unit) */
	MEDIA_ALIGNED = 2,	/* media is aligned at the aligner (but not in the unit) */
	MEDIA_LOADED = 3,	/* media is loaded into paperpath (during scanning/printing/moving)*/
	MEDIA_IN_ACCEPTOR = 4,	/* media is in acceptor (not pushed) */
	MEDIA_ACCEPTED = 5,	/* media is accepted (properly pushed into acceptbox) */
	MEDIA_JAM = 15,		/* media is jammed in somewhere */
};


/* data structure of media control block */
typedef struct media_control_s
{
	int media_state;	/* media state */
	int media_length;	/* media length in millimeters (mm) */
	int media_width;	/* media width in millimeters (mm) */
} media_control_t;


#endif /* __MEDIA_H__ */
