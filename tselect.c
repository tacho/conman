/******************************************************************************\
 *  $Id: tselect.c,v 1.4 2001/10/11 07:48:47 dun Exp $
 *    by Chris Dunlap <cdunlap@llnl.gov>
 ****************************************************************************** 
 *  Based on the implementation in Jon C. Snader's
 *    "Effective TCP/IP Programming" (Tip #20).
 ****************************************************************************** 
 *  Refer to "tselect.h" for documentation on public functions.
\******************************************************************************/


#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>						/* xyzzy */
#include <unistd.h>
#include "tselect.h"


/*******************\
**  Out of Memory  **
\*******************/

#ifdef USE_OOMF
#  undef out_of_memory
   extern void * out_of_memory(void);
#else /* !USE_OOMF */
#  ifndef out_of_memory
#    define out_of_memory() (NULL)
#  endif /* !out_of_memory */
#endif /* USE_OOMF */


/***************\
**  Constants  **
\***************/

#define TIMER_ALLOC 10


/****************\
**  Data Types  **
\****************/

struct timer {
    int             id;			/* timer ID                           */
    CallBackF       fnc;		/* callback function                  */
    void           *arg;		/* callback function arg              */
    struct timeval  tv;			/* time at which timer expires        */
    struct timer   *next;		/* next timer in list                 */
};

typedef struct timer * Timer;


/****************\
**  Prototypes  **
\****************/

static Timer alloc_timer(void);


/***************\
**  Variables  **
\***************/

static Timer active = NULL;
static Timer inactive = NULL;


/***************\
**  Functions  **
\***************/

int tselect(int maxfdp1, fd_set *rset, fd_set *wset, fd_set *xset)
{
    fd_set rbak, wbak, xbak;
    struct timeval tvNow;
    struct timeval tvDelta;
    struct timeval *tvDeltaPtr;
    Timer t;
    int n;

    if (rset)
        rbak = *rset;
    if (wset)
        wbak = *wset;
    if (xset)
        xbak = *xset;

    for (;;) {

        if (gettimeofday(&tvNow, NULL) < 0) {
            perror("ERROR: gettimeofday() failed");
            exit(1);
        }

        /*  Dispatch timer events that have expired.
         */
        while (active && !timercmp(&tvNow, &active->tv, <)) {
            DPRINTF("TSELECT: dispatching timer %d for f:%p a:%p.\n",
                active->id, active->fnc, active->arg);		/* xyzzy */
            t = active;
            active = active->next;
            t->next = inactive;
            inactive = t;
            /*
             *  The timer must be removed from the active list before the
             *    callback function is invoked in case it calls untimeout().
             *  O/w, the current timer would be removed by untimeout(),
             *    and the next timer on the active list would be removed
             *    when the callback function returned.
             */
            t->fnc(t->arg);
        }

        if (active) {
            /*
             *  Calculate time until next timer event.
             */
            tvDelta.tv_sec = active->tv.tv_sec - tvNow.tv_sec;
            tvDelta.tv_usec = active->tv.tv_usec - tvNow.tv_usec;
            if (tvDelta.tv_usec < 0) {
                tvDelta.tv_usec += 1000000;
                tvDelta.tv_sec--;
            }
            tvDeltaPtr = &tvDelta;
        }
        else if (!rset && !wset && !xset) {
            /*
             *  No additional timers and no I/O events exist.
             */
            return(0);
        }
        else {
            /*
             *  No additional timers, but I/O events exist.
             *  Set the NULL ptr so select() will not timeout.
             */
            tvDeltaPtr = NULL;
        }

        /*  Temporary kludge to support mux_io() pthread brain-damage
         *    until ConMan has been thoroughly dethreaded.
         */
        tvDelta.tv_sec = 1;
        tvDelta.tv_usec = 0;
        tvDeltaPtr = &tvDelta;

        if ((n = select(maxfdp1, rset, wset, xset, tvDeltaPtr)) < 0)
            return(-1);
        if (n > 0)
            return(n);

        /*  Temporary kludge to support mux_io() pthread brain-damage
         *    until ConMan has been thoroughly dethreaded.
         */
        return(0);

        /*  One or more timer events have expired.  Because select()
         *    will have zeroed the fdsets, restore them before continuing
         *    the next loop where the expired timers will be dispatched.
         */
        if (rset)
            *rset = rbak;
        if (wset)
            *wset = wbak;
        if (xset)
            *xset = xbak;
    }
}


int timeout(CallBackF callback, void *arg, int ms)
{
    struct timeval tv;

    assert(callback);
    assert(ms >= 0);

    if (gettimeofday(&tv, NULL) < 0) {
        perror("ERROR: gettimeofday() failed");
        exit(1);
    }
    tv.tv_usec += ms * 1000;
    if (tv.tv_usec >= 1000000) {
        tv.tv_sec += tv.tv_usec / 1000000;
        tv.tv_usec %= 1000000;
    }
    return(abtimeout(callback, arg, &tv));
}


int abtimeout(CallBackF callback, void *arg, const struct timeval *tvp)
{
    static int id = 1;
    Timer t;
    Timer tCurr;
    Timer *tPrevPtr;

    assert(callback);
    assert(tvp);

    if (!(t = alloc_timer()))
        return(-1);

    t->id = id++;
    if (id <= 0)
        id = 1;

    t->fnc = callback;
    t->arg = arg;
    t->tv = *tvp;

    tPrevPtr = &active;
    tCurr = active;
    while (tCurr && !timercmp(&t->tv, &tCurr->tv, <)) {
        tPrevPtr = &tCurr->next;
        tCurr = tCurr->next;
    }
    *tPrevPtr = t;
    t->next = tCurr;

    DPRINTF("TSELECT: started timer %d for f:%p a:%p in %ld secs.\n",
        t->id, callback, arg, (tvp->tv_sec - time(NULL)));	/* xyzzy */
    return(t->id);
}


void untimeout(int timerid)
{
    Timer tCurr;
    Timer *tPrevPtr;

    if (timerid <= 0)
        return;

    tPrevPtr = &active;
    tCurr = active;
    while (tCurr && timerid != tCurr->id) {
        tPrevPtr = &tCurr->next;
        tCurr = tCurr->next;
    }

    if (!tCurr)				/* timer id not active */
        return;
    *tPrevPtr = tCurr->next;
    tCurr->next = inactive;
    inactive = tCurr;

    DPRINTF("TSELECT: canceled timer %d.\n", timerid);		/* xyzzy */
    return;
}


static Timer alloc_timer(void)
{
/*  Returns a timer, or out_of_memory() if memory allocation fails.
 */
    Timer t, tLast;

    if (!inactive) {
        if (!(inactive = (Timer) malloc(TIMER_ALLOC * sizeof(struct timer))))
            return(out_of_memory());
        tLast = inactive + TIMER_ALLOC - 1;
        for (t=inactive; t<tLast; t++)
            t->next = t + 1;
        tLast->next = NULL;
    }
    t = inactive;
    inactive = inactive->next;
    return(t);
}