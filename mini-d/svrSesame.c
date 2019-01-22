/**
 *  SVRSESAME.C -- VOClient Mini-Daemon server methods for Sesame queries.
 *
 *  @file       svrSesame.c
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


#define	MAX_ALIASES	16



/**
 *  Public procedures
 */
extern handle_t  svr_newHandle (void *p);
extern void     *svr_H2P (handle_t h);
extern handle_t  svr_P2H (void *p);




/*****************************************************************************/
/*****  PUBLIC METHODS 							******/
/*****************************************************************************/


/**
 *  SM_NAMERESOLVER -- Resolve a target name.
 * 
 *  @brief   Resolve a target name.
 *  @fn      stat = sm_nameResolver (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_nameResolver (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));


    result->status = OK; 			/* encode the result  	*/
    result->type   = TY_INT;
    result->nitems = 1;
    sprintf (result->value[0], "%d", cvo_nameResolver (pars[0].str));

    return ( (vocRes_t *) result );
}


/**
 *  SM_SRGETPOS -- Get a "POS" string for the resolved object.
 * 
 *  @brief   Get a "POS" string for the resolved object.esolve a target name.
 *  @fn      stat = sm_srGetPOS (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_srGetPOS (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK; 			/* encode the result  	*/
    result->type   = TY_STRING;
    result->nitems = 1;
    sprintf (result->value[0], "%s", cvo_resolverPos (objid));

    return ( (vocRes_t *) result );
}


/**
 *  SM_SRGETOTYPE -- Get a object type string for the resolved object.
 * 
 *  @brief   Get a object type string for the resolved object.
 *  @fn      stat = sm_srGetOType (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_srGetOtype (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK; 			/* encode the result  	*/
    result->type   = TY_STRING;
    result->nitems = 1;
    sprintf (result->value[0], "%s", cvo_resolverOtype (objid));

    return ( (vocRes_t *) result );
}


/**
 *  SM_SRGETRA -- Get the RA position for a resolved object name.
 * 
 *  @brief   Get the RA position for a resolved object name.
 *  @fn      stat = sm_srGetRA (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_srGetRA (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK; 			/* encode the result  	*/
    result->type   = TY_FLOAT;
    result->nitems = 1;
    sprintf (result->value[0], "%g", cvo_resolverRA (objid));

    return ( (vocRes_t *) result );
}


/**
 *  SM_SRGETRAERR -- Get the RA position error for a resolved object name.
 * 
 *  @brief   Get the RA position error for a resolved object name.
 *  @fn      stat = sm_srGetRAErr (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_srGetRAErr (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK; 			/* encode the result  	*/
    result->type   = TY_FLOAT;
    result->nitems = 1;
    sprintf (result->value[0], "%g", cvo_resolverRAErr (objid));

    return ( (vocRes_t *) result );
}


/**
 *  SM_SRGETDEC -- Get the Declination position for a resolved object name.
 * 
 *  @brief   Get the Declination position for a resolved object name.
 *  @fn      stat = sm_srGetDEC (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_srGetDEC (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK; 			/* encode the result  	*/
    result->type   = TY_FLOAT;
    result->nitems = 1;
    sprintf (result->value[0], "%g", cvo_resolverDEC (objid));

    return ( (vocRes_t *) result );
}


/**
 *  SM_SRGETDECERR -- Get the DEC position error for a resolved object name.
 * 
 *  @brief   Get the DEC position error for a resolved object name.
 *  @fn      stat = sm_srGetDECErr (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_srGetDECErr (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK; 			/* encode the result  	*/
    result->type   = TY_FLOAT;
    result->nitems = 1;
    sprintf (result->value[0], "%g", cvo_resolverDECErr (objid));

    return ( (vocRes_t *) result );
}
