/**
 *  SVRSKYBOT.C -- VOClient Mini-Daemon server methods for SkyBot queries.
 *
 *  @file       svrSkyBot.c
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


/**
 *  Public procedures
 */


/**
 *  Private procedures
 */


/*****************************************************************************/
/*****  PUBLIC METHODS 							******/
/*****************************************************************************/


/**
 *  SM_SKYBOT -- Method description.
 * 
 *  @brief   Method description.
 *  @fn      stat = sm_skybot (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_skybot (int objid, msgParam *pars, int npars)
{
    return ( (vocRes_t *) NULL );
}


/**
 *  SM_SBSTRATTR -- Method description.
 * 
 *  @brief   Method description.
 *  @fn      stat = sm_sbStrAttr (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_sbStrAttr (int objid, msgParam *pars, int npars)
{
    return ( (vocRes_t *) NULL );
}


/**
 *  SM_SBDBLATTR -- Method description.
 * 
 *  @brief   Method description.
 *  @fn      stat = sm_sbDblAttr (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_sbDblAttr (int objid, msgParam *pars, int npars)
{
    return ( (vocRes_t *) NULL );
}


/**
 *  SM_SBNObJS -- Method description.
 * 
 *  @brief   Method description.
 *  @fn      stat = sm_sbNObjs (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_sbNObjs (int objid, msgParam *pars, int npars)
{
    return ( (vocRes_t *) NULL );
}
