/* who3.c - who with buffered reads
 *	  - surpresses empty records
 *	  - formats time nicely
 *	  - buffers input (using utmplib)
 */
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <utmp.h>
#include <stdlib.h>

#define	SHOWHOST
#define NRECS   16
#define NULLUT  ((struct utmp *)NULL)
#define UTSIZE  (sizeof(struct utmp))

static  char    utmpbuf[NRECS * UTSIZE];                /* storage      */
static  int     num_recs;                               /* num stored   */
static  int     cur_rec;                                /* next to go   */
static  int     fd_utmp = -1;                           /* read from    */

void show_info(struct utmp *);
void showtime(time_t);
int utmp_open(char *);
struct utmp *utmp_next();
int utmp_reload();
void utmp_close();


int main()
{
	struct utmp	*utbufp,	/* holds pointer to next rec	*/
			*utmp_next();	/* returns pointer to next	*/

	if(utmp_open( UTMP_FILE ) == -1)
	{
		perror(UTMP_FILE);
		exit(1);
	}
	while(( utbufp = utmp_next() ) != ((struct utmp *) NULL))
		show_info( utbufp );
	utmp_close();
	return 0;
}
/*
 *	show info()
 *			displays the contents of the utmp struct
 *			in human readable form
 *			* displays nothing if record has no user name
 */
void show_info( struct utmp *utbufp )
{
	if ( utbufp->ut_type != USER_PROCESS )
		return;

	printf("%-8.8s", utbufp->ut_name);		/* the logname	*/
	printf(" ");					/* a space	*/
	printf("%-8.8s", utbufp->ut_line);		/* the tty	*/
	printf(" ");					/* a space	*/
	showtime( utbufp->ut_time );			/* display time	*/
#ifdef SHOWHOST
	if ( utbufp->ut_host[0] != '\0' )
		printf(" (%s)", utbufp->ut_host);	/* the host	*/
#endif
	printf("\n");					/* newline	*/
}

void showtime( time_t timeval )
/*
 *	displays time in a format fit for human consumption
 *	uses ctime to build a string then picks parts out of it
 *      Note: %12.12s prints a string 12 chars wide and LIMITS
 *      it to 12chars.
 */
{
	char	*ctime();		/* convert long to ascii	*/
	char	*cp;			/* to hold address of time	*/

	cp = ctime( &timeval );		/* convert time to string	*/
					/* string looks like		*/
					/* Mon Feb  4 00:46:40 EST 1991 */
					/* 0123456789012345.		*/
	printf("%12.12s", cp+4 );	/* pick 12 chars from pos 4	*/
}


int utmp_open( char *filename )
{
        fd_utmp = open( filename, O_RDONLY );           /* open it      */
        cur_rec = num_recs = 0;                         /* no recs yet  */
        return fd_utmp;                                 /* report       */
}

struct utmp *utmp_next()
{
        struct utmp *recp;

        if ( fd_utmp == -1 )                            /* error ?      */
                return NULLUT;
        if ( cur_rec==num_recs && utmp_reload()==0 )    /* any more ?   */
                return NULLUT;
                                        /* get address of next record    */
        recp = ( struct utmp *) &utmpbuf[cur_rec * UTSIZE];
        cur_rec++;
        return recp;
}

int utmp_reload()
/*
 *      read next bunch of records into buffer
 */
{
        int     amt_read;

                                                /* read them in         */
        amt_read = read( fd_utmp , utmpbuf, NRECS * UTSIZE );   

                                                /* how many did we get? */
        num_recs = amt_read/UTSIZE;
                                                /* reset pointer        */
        cur_rec  = 0;
        return num_recs;
}

void utmp_close()
{
    if ( fd_utmp != -1 )                    /* don't close if not   */
		close( fd_utmp );               /* open                 */
}
