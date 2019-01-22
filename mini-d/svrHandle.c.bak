/**
 *  SVRHANDLE.C  -- Utility methods to convert struct pointers to user handles.
 *
 *  @brief      Utility methods to convert struct pointers to user handles.
 *
 *  @file       svrUtil.c
 *  @author     Mike Fitzpatrick
 *  @date       April 2013
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <ctype.h>


typedef  int 	handle_t;

#define	MAX_HANDLES	8192


int     numSvrHandles         = 0;
long    svrHandles[MAX_HANDLES];

/*  Public procedures
*/
void      svr_initHandles (void);
void      svr_cleanupHandles (void);
handle_t  svr_newHandle (void *ptr);
void      svr_freeHandle (handle_t handle);
handle_t  svr_P2H (void *ptr);
void     *svr_H2P (handle_t handle);



/**
 *  SVR_INITHANDLES -- Initialize the handles structure.
 * 
 *  @brief  Initialize the handles structure.
 *  @fn	    void svr_initHandles (void)
 *
 *  @return		nothing
 */
void
svr_initHandles (void)
{
    /*  Initialize the handle-to-ptr converter the first time we're called,
     *  or whenever we've restarted.
     */
    if (numSvrHandles == 0)
	memset (svrHandles, 0, sizeof (svrHandles));
}


/**
 *  SVR_CLEANUPHANDLES -- Cleanup the handles structure.
 * 
 *  @brief  Cleanup the handles structure.
 *  @fn	    void svr_cleanupHandles (void)
 *
 *  @return		nothing
 */
void
svr_cleanupHandles (void)
{
    register int i;

    for (i=numSvrHandles; i >= 0; i--) {
	if (svrHandles[i]) {
	    free ((void *) svrHandles[i]);
	    svrHandles[i] = 0;
 	}
    }
    numSvrHandles = 0;
}


/*  Utility routines for keep track of handles.
*/

/**
 *  SVR_NEWHANDLE -- Get an unused object handle.
 * 
 *  @brief  Get an unused object handle
 *  @fn	    handle_t svr_newHandle (void *ptr)
 *
 *  @param  ptr		pointer to object to be stored
 *  @return		new object handle
 */
handle_t
svr_newHandle (void *ptr)
{
    /*  Initialize the handle-to-ptr converter the first time we're called,
     *  or whenever we've restarted.
     */
    if (numSvrHandles == 0)
	memset (svrHandles, 0, sizeof (svrHandles));
    svrHandles[++numSvrHandles] = (long) ptr;

    return (numSvrHandles);
}


/**
 *  SVR_FREEHANDLE -- Free the handle for later re-use.
 *
 *  @brief  Free the handle for later re-use.
 *  @fn     svr_freeHandle (handle_t handle)
 *
 *  @param  handle	object handle
 *  @return 		nothing
 */
void
svr_freeHandle (handle_t handle)
{
    register int i, j;
    void *ptr = svr_H2P (handle);


    if (handle <= 0) {
	fprintf (stderr, "Error: Attempt to free zero handle!\n");
	return;
    }

    for (i=1; i < MAX_HANDLES; i++) {
	if ((void *) ptr == (void *) svrHandles[i]) {
	    for (j=i+1; j < MAX_HANDLES; j++) {
		if (svrHandles[j])
		    svrHandles[i++] = svrHandles[j];
		else
		    break;
	    }
	    numSvrHandles = ((numSvrHandles-1) >= 0 ? (numSvrHandles-1) : 0);
	    break;
 	}
    }
}


/**
 *  SVR_P2H -- Convert a pointer to a handle
 *
 *  @brief  Convert a pointer to a handle
 *  @fn     handle_t svr_P2H (void *ptr)
 *
 *  @param  ptr		pointer to object
 *  @return 		handle to object, < 0 on error
 */
handle_t	
svr_P2H (void *ptr)
{
    register int i;

    for (i=1; i < MAX_HANDLES; i++) {
	if ((void *) ptr == (void *) svrHandles[i])
	    return ((int) i);
    }

    return (-1);
}


/**
 *  SVR_H2P -- Convert a handle to a pointer
 *
 *  @brief  Convert a handle to a pointer
 *  @fn     void *svr_H2P (int handle) 
 *
 *  @param  handle	object handle
 *  @return 		pointer to object or NULL
 */
void *
svr_H2P (handle_t handle) 
{ 
    return ((void *) svrHandles[handle]); 
}
