/**
 *  VOCSERVER.C -- VOClient "Mini-Daemon" server.
 *
 *
 *  @file       vocServer.c
 *  @author     Michael Fitzpatrick
 *  @version    April 2013
 *
 *************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <errno.h>
#include <signal.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/uio.h>

#define _VOCLIENT_LIB_
#include "VOClient.h"
#include "vocServer.h"
#include "svrMethods.h"


#define MINI_DEBUG	(getenv("MINI_DBG")||access("/tmp/MINI_DBG",F_OK)==0)


/**
 *  Global variable declarations.
 */
msgParam  Params[MAX_PARAMS];


/**
 *  Public procedures
 */
vocRes_t  *svr_processMsg (vocMsg_t *msg);

extern vocRes_t  *svr_getOKResult (void);
extern vocRes_t  *svr_getERRResult (void);
extern void       svr_resetHandles (void);


/**
 *  Private procedures
 */
static int    svr_atoi (char *val);
static int    svr_getMType (char *msg);
static int    svr_getParams (msgParam pars[], int npars);

static double svr_atof (char *val);

static void   svr_printParams (msgParam pars[], int npars);

static char  *svr_typeStr (int code);
static char   to_hex (char code);

static vocRes_t *svr_callMethod (int objid, char *method, msgParam pars[], 
			int nparams);


/*****************************************************************************/
/*****  PUBLIC METHODS 							******/
/*****************************************************************************/

/**
 *  SVR_PROCESSMSG -- Process a server message.
 * 
 *  @brief   Process a server message.
 *  @fn      res = svr_processMsg (vocMsg_t *msg)
 *
 *  @param   msg         message struct
 *  @returns             a pointer to a vocRes_t struct or NULL
 */
vocRes_t *
svr_processMsg (vocMsg_t *msg)
{
    char  *method = NULL, *msgclass = NULL, *str = NULL;
    char   message[SZ_MSG];
    int    objid=0, nparams=0, status=0, type=0, nitems=0;
    int    mtype = 0;
    vocRes_t  *res = (vocRes_t *) NULL;


    if (!msg) {
	fprintf (stderr, "Error: svr_processMsg(): NULL message\n");
	return ( (vocRes_t *) NULL );
    }

    memset (Params, 0, sizeof (Params));
    memset (message, 0, SZ_MSG);
    strcpy (message, msg->message);		/* make working copy 	*/


    /* Get the message type and initialize the string tokenizer.
     */
    mtype =  svr_getMType (strtok (message, " "));
    (void) strtok (NULL, " ");			/* skip opening brace	*/

    switch (mtype) {
    case MSG_CALL:
	objid   = svr_atoi (strtok (NULL, " "));
	method  = strtok (NULL, " ");
	nparams = svr_atoi (strtok (NULL, " "));

	if (svr_getParams (Params, nparams) != nparams) {
	    fprintf (stderr, "svr_processMType: invalid parameter count.\n");
	    return ( (vocRes_t *) NULL );
	}
	if (MINI_DEBUG)  {
	    fprintf (stderr, "CALL:  objid=%d  method='%s'  nparams=%d\n",
		objid, method, nparams);
	    svr_printParams (Params, nparams);
	}

	/*  Call the requested method.  This will automatically set the
	 *  Result object when done.
	 */
	res = svr_callMethod (objid, method, Params, nparams);
	break;

    case MSG_RESULT:
	status = svr_atoi (strtok (NULL, " "));
	type   = svr_atoi (strtok (NULL, " "));
	nitems = svr_atoi (strtok (NULL, " "));

	if (MINI_DEBUG) 
	    fprintf (stderr, "RESULT:  status=%d  type='%s'  nitems=%d\n",
		status, svr_typeStr (type), nitems);
	break;

    case MSG_MESSAGE:
	msgclass = strtok (NULL, " ");		/* eat it	*/
	str      = strtok (NULL, " ");

	if (MINI_DEBUG) {
	    fprintf (stderr, "RESULT:  status=%d  type='%s'  nitems=%d\n",
		status, svr_typeStr (type), nitems);
	    fprintf (stderr, "RESULT:  class='%s'  str='%s'\n", msgclass, str);
	}
	break;

    case MSG_END:
	/*  Shut down processing in the server, free resources, etc.
	 */
	if (MINI_DEBUG) fprintf (stderr, "END ...\n");

	res = svr_getOKResult ();
	svr_resetHandles ();
	break;
    case MSG_QUIT:
	/*  Quit processing in the server, free resources, etc.
	 */
	if (MINI_DEBUG) fprintf (stderr, "QUIT ...\n");

	res = svr_getOKResult ();
	svr_resetHandles ();
	break;
    case MSG_ACK:					/* NO-OP	*/
	if (MINI_DEBUG) fprintf (stderr, "got ACK ...");
	res = svr_getOKResult ();
	break;
    case MSG_NOACK:					/* NO-OP	*/
	if (MINI_DEBUG) fprintf (stderr, "got NOACK ...");
	res = svr_getOKResult ();
	break;
    default:
	fprintf (stderr, "svr_processMsg:  unknown message type\n");
	return ( (vocRes_t *) NULL );
    }
    
    return ( res );
}




/**
 *  SVR_URLENCODE -- Returns a url-encoded version of str.  Call must free()
 *  the pointer that is returned.
 */
char *
svr_urlEncode (char *str)
{
    char *pstr = str, *buf = malloc(strlen(str) * 3 + 1), *pbuf = buf;

    while (*pstr) {
        if (isalnum(*pstr) ||
            *pstr == '-' ||
            *pstr == '_' ||
            *pstr == '.' ||
            *pstr == '~')
                *pbuf++ = *pstr;
        else if (*pstr == ' ')
            *pbuf++ = '+';
        else {
            *pbuf++ = '%';
            *pbuf++ = to_hex (*pstr >> 4);
            *pbuf++ = to_hex (*pstr & 15);
        }
        pstr++;
    }
    *pbuf = '\0';
    return buf;
}



/*****************************************************************************/
/*****  PRIVATE METHODS 						******/
/*****************************************************************************/


/**
 *  SVR_CALLMETHOD -- Call a requested server method.
 */
static vocRes_t *
svr_callMethod (int objid, char *method, msgParam pars[], int nparams)
{
    register int i;
    vocRes_t  *res = (vocRes_t *) NULL;

    for (i=0; vocMethods[i].name; i++) {
	if (strcmp (vocMethods[i].name, method) == 0) {
	    res = (*vocMethods[i].func) (objid, pars, nparams);
	    break;
	}
    }

    return (res);
}


/**
 *  SVR_GETPARAMS -- Get the parameters from the message.
 */
static int
svr_getParams (msgParam pars[], int npars)
{
    register int i, len, nfound = 0;
    char   *tok, *str = NULL;;

    for (i=0; i < npars; i++) {
	tok = strtok (NULL, " ");	/* get the param type	*/
	if (*tok == '}') {
	    fprintf (stderr, "svr_getParams:  missing type token.\n");
	    break;
	}

	switch ( (pars[i].type = svr_atoi(tok)) ) {
	case TY_INT: 	
	    pars[i].ival = svr_atoi (strtok (NULL, " "));  	
	    break;
	case TY_FLOAT: 	
	    pars[i].dval = svr_atof (strtok (NULL, " "));  	
	    break;
	case TY_STRING: 
	    str = strtok (NULL, " ");  	/* strip quotes around strings */
	    len = strlen (str);
	    str[len-1] = '\0';
	    pars[i].str = &str[1];
	    break;
	}
	nfound++;
    }

    return (nfound);
}


/**
 *  SVR_PRINTPARAMS -- Print a parameter struct.
 */
static void
svr_printParams (msgParam pars[], int npars)
{
    register int i;

    for (i=0; i < npars; i++) {
	switch (pars[i].type) {
	case TY_INT: 	fprintf (stderr, "\tpar[%d] = %d\n",   i, pars[i].ival);
	case TY_FLOAT: 	fprintf (stderr, "\tpar[%d] = %g\n",   i, pars[i].dval);
	case TY_STRING: fprintf (stderr, "\tpar[%d] = '%s'\n", i, pars[i].str);
	}
    }
}


/**
 *  SVR_GETMTYPE -- Convert a message type string to an int code.
 */
static int
svr_getMType (char *mtype)
{
    if (strcmp (mtype, "CALL") == 0) 		return (MSG_CALL);
    else if (strcmp (mtype, "RESULT") == 0) 	return (MSG_RESULT);
    else if (strcmp (mtype, "MSG") == 0) 	return (MSG_MESSAGE);
    else if (strcmp (mtype, "END") == 0) 	return (MSG_END);
    else if (strcmp (mtype, "QUIT") == 0) 	return (MSG_QUIT);
    else if (strcmp (mtype, "ACK") == 0) 	return (MSG_ACK);
    else if (strcmp (mtype, "NOACK") == 0) 	return (MSG_NOACK);

    return (-1);
}


/**
 *  SVR_TYPESTR -- Convert a type code to a string for debug printing.
 */
static char *
svr_typeStr (int code)
{
    switch (code) {
    case TY_INT:		return ("TY_INT");
    case TY_FLOAT:		return ("TY_FLOAT");
    case TY_STRING:		return ("TY_STRING");
    case TY_BULK:		return ("TY_BULK");
    default:			break;
    }
    return (NULL);
}


/**
 *  SVR_ATOI -- System atoi() with lexical argument checking.
 */
static int
svr_atoi (char *val)
{
    char *ip;

    for (ip = val; *ip; ip++) {
	if (isalpha ((int) *ip)) {
	    fprintf (stderr, "Warning: value '%s' is not an integer\n", val);
	    break;
	}
    }
    return (atoi (val));
}


/**
 *  SVR_ATOF -- System atoi() with lexical argument checking.
 */
static double
svr_atof (char *val)
{
    char *ip;

    for (ip = val; *ip; ip++) {
	if (isalpha ((int) *ip)) {
	    fprintf (stderr, "Warning: value '%s' is not an integer\n", val);
	    break;
	}
    }
    return (atof (val));
}


/**
 *  TO_HEX -- Converts an integer value to its hex character
 */
static char
to_hex (char code)
{
    static char hex[] = "0123456789abcdef";
    return (hex[code & 15]);
}
