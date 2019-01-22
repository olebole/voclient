/**
 *  SVRREGISTRY.C -- VOClient Mini-Daemon server methods for Registry queries.
 *
 *  @file       svrRegistry.c
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
#include "cvoRegistry.h"


#define	DEBUG			FALSE


/**
 *  Public procedures
 */
extern handle_t  svr_newHandle (void *p);
extern void     *svr_H2P (handle_t h);
extern handle_t  svr_P2H (void *p);

/**
 *  Private procedures
 */



/*****************************************************************************/
/*****  PUBLIC METHODS 							******/
/*****************************************************************************/


/**
 *  SM_REGSEARCH -- Method description.
 * 
 *  @brief   Method description.
 *  @fn      stat = sm_regSearch (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_regSearch (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_INT;
    result->nitems = 1;

    switch (npars) {
    case 1: 				// Only have one search term
        sprintf (result->value[0], "%d", 
	    cvo_regSearch (pars[0].str, NULL, TRUE));
	break;
    case 2: 				// One term and an OR value (unused)
        sprintf (result->value[0], "%d", 
	    cvo_regSearch (NULL, pars[0].str, pars[1].ival));
	break;
    case 3: 				// Two terms and an OR value
        sprintf (result->value[0], "%d", 
	    cvo_regSearch (pars[0].str, pars[1].str, pars[2].ival));
	break;
    }

    return ( (vocRes_t *) result );
}


/**
 *  SM_RESSEARCHBYSVC -- Method description.
 * 
 *  @brief   Method description.
 *  @fn      stat = sm_resSearchBySvc (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_resSearchBySvc (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_INT;
    result->nitems = 1;

    sprintf (result->value[0], "%d", 
	cvo_regSearchByService (pars[0].str, pars[1].str, pars[2].ival));

    return ( (vocRes_t *) result );
}


/**
 *  SM_REGQUERY -- Method description.
 * 
 *  @brief   Method description.
 *  @fn      stat = sm_regQuery (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_regQuery (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_INT;
    result->nitems = 1;

    sprintf (result->value[0], "%d", cvo_regQuery (pars[0].str, pars[1].ival));

    return ( (vocRes_t *) result );
}


/**
 *  SM_REGADDSEARCHTERM -- Method description.
 * 
 *  @brief   Method description.
 *  @fn      stat = sm_regAddSearchTerm (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_regAddSearchTerm (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_INT;
    result->nitems = 1;

    cvo_regAddSearchTerm (objid, pars[0].str, pars[1].ival);

    return ( (vocRes_t *) result );
}


/**
 *  SM_REMOVESEARCHTERM -- Method description.
 * 
 *  @brief   Method description.
 *  @fn      stat = sm_removeSearchTerm (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_removeSearchTerm (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_INT;
    result->nitems = 1;

    cvo_regRemoveSearchTerm (objid, pars[0].str);

    return ( (vocRes_t *) result );
}


/**
 *  SM_REGCONSTWAVEBAND -- Method description.
 * 
 *  @brief   Method description.
 *  @fn      stat = sm_regConstWaveband (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_regConstWaveband (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;
    result->type   = TY_INT;
    result->nitems = 1;

    cvo_regConstWaveband (objid, pars[0].str);

    return ( (vocRes_t *) result );
}


/**
 *  SM_REGCONSTSVCTYPE -- Method description.
 * 
 *  @brief   Method description.
 *  @fn      stat = sm_regConstSvcType (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_regConstSvcType (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;
    result->type   = TY_INT;
    result->nitems = 1;

    cvo_regConstSvcType (objid, pars[0].str);

    return ( (vocRes_t *) result );
}


/**
 *  SM_REGDALONLY -- Method description.
 * 
 *  @brief   Method description.
 *  @fn      stat = sm_regDALOnly (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_regDALOnly (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;
    result->type   = TY_INT;
    result->nitems = 1;

    cvo_regDALOnly (objid, pars[0].ival);

    return ( (vocRes_t *) result );
}


/**
 *  SM_REGSORTRES -- Method description.
 * 
 *  @brief   Method description.
 *  @fn      stat = sm_regSortRes (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_regSortRes (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;
    result->type   = TY_INT;
    result->nitems = 1;

    cvo_regSortRes (objid, pars[0].ival);

    return ( (vocRes_t *) result );
}


/**
 *  SM_REGGETSTCOUNT -- Method description.
 * 
 *  @brief   Method description.
 *  @fn      stat = sm_regGetSTCount (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_regGetSTCount (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;
    result->type   = TY_INT;
    result->nitems = 1;

    sprintf (result->value[0], "%d", cvo_regGetSTCount (objid));

    return ( (vocRes_t *) result );
}


/**
 *  SM_REGGETQUERYSTRING -- Method description.
 * 
 *  @brief   Method description.
 *  @fn      stat = sm_regGetQueryString (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_regGetQueryString (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_STRING;
    result->nitems = 1;
    sprintf (result->value[0], "%s", cvo_regGetQueryString (objid));

    return ( (vocRes_t *) result );
}


/**
 *  SM_REGEXECUTE -- Method description.
 * 
 *  @brief   Method description.
 *  @fn      stat = sm_regExecute (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_regExecute (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_INT;
    result->nitems = 1;

    sprintf (result->value[0], "%d", cvo_regExecute (objid));

    return ( (vocRes_t *) result );
}


/**
 *  SM_RESEXEVUTERAW -- Method description.
 * 
 *  @brief   Method description.
 *  @fn      stat = sm_regExecuteRaw (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_regExecuteRaw (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_STRING;
    result->nitems = 1;

    sprintf (result->value[0], "%s", cvo_regExecuteRaw (objid));

    return ( (vocRes_t *) result );
}


/**
 *  SM_RESGETCOUNT -- Method description.
 * 
 *  @brief   Method description.
 *  @fn      stat = sm_resGetCount (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_resGetCount (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;
    result->type   = TY_INT;
    result->nitems = 1;

    sprintf (result->value[0], "%d", cvo_resGetCount (objid));

    return ( (vocRes_t *) result );
}


/**
 *  SM_RESGETSTRING -- Method description.
 * 
 *  @brief   Method description.
 *  @fn      stat = sm_resGetStr (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_resGetStr (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_STRING;
    result->nitems = 1;

    sprintf (result->value[0], "%s", 
	cvo_resGetStr (objid, pars[0].str, pars[1].ival));

    return ( (vocRes_t *) result );
}


/**
 *  SM_RESGETFLOAT -- Method description.
 * 
 *  @brief   Method description.
 *  @fn      stat = sm_resGetFloat (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_resGetFloat (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_FLOAT;
    result->nitems = 1;

    sprintf (result->value[0], "%g", 
	cvo_resGetFloat (objid, pars[0].str, pars[1].ival));

    return ( (vocRes_t *) result );
}


/**
 *  SM_RESGETINT -- Method description.
 * 
 *  @brief   Method description.
 *  @fn      stat = sm_resGetInt (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_resGetInt (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_INT;
    result->nitems = 1;

    sprintf (result->value[0], "%d", 
	cvo_resGetInt (objid, pars[0].str, pars[1].ival));

    return ( (vocRes_t *) result );
}
