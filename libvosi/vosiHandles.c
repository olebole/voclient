/**
 *  VOSIHANDLE.C  -- Utility methods to convert struct pointers to user handles.
 *
 *  @brief      Utility methods to convert struct pointers to user handles.
 *
 *  @file       vosiHandle.c
 *  @authors    Mike Fitzpatrick, Doug Tody
 *  @date       January 2014
 *
 *  This version has been modified to avoid having a single large (eg 8192
 *  element) array of service handles.  In particular, if freeHandle deletes
 *  a handle and shifts the remaining elements of the array left, then (aside
 *  from the obvious inefficiency issue) index values change and the
 *  array[index] -> ptr relationship is no longer valid.
 *
 *  This could be fixed by replacing the simple index array with a hash table
 *  as in the Java version, however it would be nice to have a simpler scheme.
 *  The scheme provided here is instead context or subsystem-oriented.
 *  When a module is initialized, opens a connection, or initiates some other
 *  module-defined context, it opens a new context with vosiHandle.  Resource
 *  handles are then handed out sequentially within the connection context.
 *  If subsequently freed, the slot is merely left unused.  When subsequently
 *  the connection context is freed, all resource handles created within that
 *  context are destroyed, and the connection number within vosiHandle is
 *  available for re-use.  Since connections are associated with dynamic
 *  activity this should make it much less likely to run out of space.
 *  (In DALClient for example, each connection to a remote service is the
 *  basis for all subsequent activity, and all resource handles created
 *  during execution are invalid anyway after closing the service connection).
 *
 *		 vosi_initHandles ()
 *		vosi_resetHandles ()
 *   hcon = vosi_newHandleContext (contextName, size)
 *        vosi_closeHandleContext (hcon)
 *	  handle = vosi_newHandle (hcon, ptr)
 *		  vosi_freeHandle (handle)
 *		   ptr = vosi_H2P (handle)
 *
 *  We still have a simple global space of integer resource handles at the
 *  top level.  Each integer handle consists of a connection number plus a
 *  resource number (array index within the connection context) packed as a
 *  single integer.  [The P2H routine was omitted above as it is hard to see
 *  where this would be needed in well-designed class code where we are
 *  already operating in the space where we have resource pointers].
 *  D.Tody January 2014
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


/* Internal Definitions.
 */

#define MAX_CONTEXTS	256	/* max active contexts */
#define	MAX_HANDLES	4096	/* max handles per context */
#define	SZ_HCNAME	64	/* context name */

#define	H_ENCODE(c,i)	(((i)<<8)|(c))
#define	H_CONTEXT(h)	((h)&0x77)
#define	H_SLOT(h)	((h)>>8)

typedef int handle_t;

typedef struct vosi_hContext_t {
    char name[SZ_HCNAME];	/* handle context name (info only) */
    int nHandles;		/* number of handles in use */
    long index[MAX_HANDLES];	/* handle storage */
} vosi_hContext_t;

int vosi_numHContexts = 0;
int vosi_HContextOverflow = 0;
int vosi_HContextPoolOverflow = 0;
vosi_hContext_t *vosiHContexts[MAX_CONTEXTS];	/* context descriptors */


/*  Public procedures
*/
void vosi_initHandles (void);
int vosi_newHandleContext (char *name, int size);
void vosi_closeHandleContext (int context);
handle_t vosi_newHandle (int context, void *ptr);
void vosi_freeHandle (handle_t handle);
void *vosi_H2P (handle_t handle);


/**
 *  VOSI_INITHANDLES -- Initialize the handles structure.
 * 
 *  @brief  Initialize the handles structure.
 *  @fn	    void vosi_initHandles (void)
 *
 *  @return		nothing
 */
void vosi_initHandles (void)
{
    /* Initialize the handle-to-ptr converter the first time we're called. */
    if (vosi_numHContexts == 0)
	memset (vosiHContexts, 0, sizeof (vosiHContexts));
}


/**
 *  VOSI_RESETANDLES -- Reset the handles structure.
 * 
 *  @brief  Reset the handles structure.
 *  @fn	    void vosi_resetHandles (void)
 *
 *  @return		nothing
 *
 *  The handles structure is reset to its initial state, freeing all
 *  resources (for example when closing down the module).
 */
void vosi_resetHandles (void)
{
    int i;

    /* If not in use, do nothing. */
    if (vosi_numHContexts == 0)
	return;

    /* Free any context descriptors. */
    for (i = 0; i < vosi_numHContexts; i++)
	if (vosiHContexts[i] != NULL)
	    free ((void *) vosiHContexts[i]);

    vosi_numHContexts = 0;
    memset (vosiHContexts, 0, sizeof (vosiHContexts));
}


/**
 *  VOSI_NEWHANDLECONTEXT -- Get a new handle context.
 * 
 *  @brief  Get a new handle context
 *  @fn	    int vosi_newHandleContext (char *name, int size)
 *
 *  @param  name	Context name (informational only)
 *  @param  size	Max number of handles (not currently implemented)
 *  @return		Context number allocated
 *
 *  The size argument is not currently implemented and should be set to
 *  zero, which would cause a context to be created with the default size.
 *
 *  A context number is a small integer, e.g., like a file descriptor.
 *  The handle mechanism permits only a limited number of simultaneously
 *  active handler contexts (no need to make it more complex than that).
 */
int vosi_newHandleContext (char *name, int size)
{
    vosi_hContext_t *hc = NULL;
    int i, context = -1;

    /* Initialize the handle-to-ptr converter the first time we're called.  */
    if (vosi_numHContexts == 0)
	memset (vosiHContexts, 0, sizeof (vosiHContexts));

    /* Reuse an empty context if available.  Skip context zero so that a
     * legal handle cannot be zero.
     */
    for (i = 1; i < vosi_numHContexts; i++)
	if (vosiHContexts[i] == NULL) {
	    context = i;
	    break;
	}

    /* Add a new context descriptor to the active pool. */
    if (context < 0)
	context = ++vosi_numHContexts;

    /* Allocate a new Context descriptor. */
    hc = (vosi_hContext_t *) calloc ((size_t) 1, sizeof (vosi_hContext_t));

    /* Overflow should not be possible or we will need a smarter error
     * handling mechanism here.
     */
    if (vosi_numHContexts > MAX_CONTEXTS || (hc == NULL)) {
	fprintf (stderr, "Error: ran out of voclient handle contexts!\n");
	if (hc != NULL) {
	    vosi_HContextPoolOverflow++;
	    free ((void *) hc);
	}
	return (-1);
    }

    /* Initialize the context descriptor. */
    vosiHContexts[context] = hc;
    strncpy (hc->name, name, SZ_HCNAME - 1);
    hc->nHandles = 0;

    return (context);
}


/**
 *  VOSI_CLOSEHANDLECONTEXT -- Close a handle context.
 *
 *  @brief   Close a handle context
 *  @fn	     vosi_closeHandleContext (int context)
 *
 *  @param   context		Handle context
 *
 *  Any pointers to external objects are simply deleted.  It is up to the
 *  caller to manage any external objects.
 */
void vosi_closeHandleContext (int context)
{
    free ((void *) vosiHContexts[context]);
    vosiHContexts[context] = NULL;
}


/**
 *  VOSI_NEWHANDLE -- Get an unused object handle.
 * 
 *  @brief  Get an unused object handle
 *  @fn	    handle_t vosi_newHandle (void *ptr)
 * *  @param  context	context number
 *  @param  ptr		pointer to object to be stored
 *  @return		new object handle
 *
 *  A new object handle is allocated within the specified handle context.
 *  The context must have been allocated earlier.  This implementation
 *  merely allocates handle slots sequentially, assuming that there are
 *  a limited number of active resource pointers for a context.
 */
handle_t vosi_newHandle (int context, void *ptr)
{
    vosi_hContext_t *hc = NULL;
    int slot;

    /* Get the already allocated context. */
    if (context < 0 || context >= MAX_CONTEXTS)
	return (-1);
    else {
	hc = vosiHContexts[context];
	if (hc == NULL)
	    return (-1);
    }

    /* Index zero is not used here. */
    slot = ++hc->nHandles;
    hc->index[slot] = (long) ptr;

    /* Wrap around if we run out of slots.  This should not happen, but
     * could if the context is very active and handles are not freed.
     */
    if (hc->nHandles >= MAX_HANDLES) {
	vosi_HContextOverflow++;
	hc->nHandles = 0;
    }

    return (H_ENCODE (context, slot));
}


/**
 *  VOSI_FREEHANDLE -- Free the handle for later re-use.
 *
 *  @brief  Free the handle for later re-use.
 *  @fn     vosi_freeHandle (handle_t handle)
 *
 *  @param  handle	object handle
 *  @return 		nothing
 *
 *  In this implementation we don't actually re-use a handle unless it
 *  is at the end of the list, but we mark all freed handles as NULL in any
 *  case so that the handle index remains valid.
 */
void vosi_freeHandle (handle_t handle)
{
    vosi_hContext_t *hc = NULL;
    int context = H_CONTEXT (handle);
    int slot = H_SLOT (handle);

    /* Get the already allocated context. */
    if (context < 0 || context >= MAX_CONTEXTS)
	return;
    else {
	hc = vosiHContexts[context];
	if (hc == NULL)
	    return;
    }

    /* Free the handle. */
    hc->index[slot] = 0;
    if (slot > 0 && slot == hc->nHandles)
	--hc->nHandles;
}


/**
 *  VOSI_H2P -- Convert a handle to a pointer
 *
 *  @brief  Convert a handle to a pointer
 *  @fn     void *vosi_H2P (int handle) 
 *
 *  @param  handle	object handle
 *  @return 		pointer to object or NULL
 */
void *vosi_H2P (handle_t handle)
{
    /* Check for an invalid handle. */
    if (handle <= 0)
	return (NULL);

    vosi_hContext_t *hc = NULL;
    int context = H_CONTEXT (handle);
    int slot = H_SLOT (handle);

    /* Get the already allocated context. */
    if (context < 0 || context >= MAX_CONTEXTS)
	return (NULL);
    else {
	hc = vosiHContexts[context];
	if (hc == NULL)
	    return (NULL);
    }

    return ((void *) hc->index[slot]);
}
