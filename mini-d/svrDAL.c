/**
 *  SVRDAL.C -- VOClient Mini-Daemon server methods for DAL queries.
 *
 *  @file       svrDAL.c
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
 *  SM_NEWCONNECTION -- Create a new connection context.
 * 
 *  @brief   Create a new connection context.
 *  @fn      stat = sm_newConnection (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_newConnection (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

/*  FIXME
    type = DAL | Cone | Siap | Ssap
*/
    result->status = OK;                        /* encode the result    */
    result->type   = TY_INT;
    result->nitems = 1;
    sprintf (result->value[0], "%d", cvo_nameResolver (pars[0].str));

    return ( (vocRes_t *) result );
}


/**
 *  SM_GETRAWURL -- Download a URL and return the result.
 * 
 *  @brief   Download a URL and return the result.
 *  @fn      stat = sm_getRawURL (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_getRawURL (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

/*  FIXME
    char *baseURL 
*/
    result->status = OK;                        /* encode the result    */
    result->type   = TY_INT;
    result->nitems = 1;
    sprintf (result->value[0], "%d", cvo_nameResolver (pars[0].str));

    return ( (vocRes_t *) result );
}


/**
 *  SM_REMOVECONNECTION -- Close a client DAL connection context.
 * 
 *  @brief   Close a client DAL connection context.
 *  @fn      stat = sm_removeConnection (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_removeConnection (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_INT;
    result->nitems = 1;
    sprintf (result->value[0], "%d", OK);
    cvo_closeConnection (objid);

    return ( (vocRes_t *) result );
}


/**
 *  SM_GETSERVICECOUNT -- Method description.
 * 
 *  @brief   Method description.
 *  @fn      stat = sm_getServiceCount (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_getServiceCount (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_INT;
    result->nitems = 1;
    sprintf (result->value[0], "%d", cvo_getServiceCount (objid));

    return ( (vocRes_t *) result );
}


/**
 *  SM_ADDSERVICEURL -- Add a service URL to the specified connection.
 * 
 *  @brief   Add a service URL to the specified connection.
 *  @fn      stat = sm_addServiceURL (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_addServiceURL (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_INT;
    result->nitems = 1;
    sprintf (result->value[0], "%d", OK);
    cvo_addServiceURL (objid, pars[0].str);

    return ( (vocRes_t *) result );
}


/**
 *  SM_GETSERVICEURL -- Get the requested service URL for the connection.
 * 
 *  @brief   Get the requested service URL for the connection.
 *  @fn      stat = sm_getServiceURL (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_getServiceURL (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_STRING;
    result->nitems = 1;
    sprintf (result->value[0], "%s", cvo_getServiceURL (objid, pars[0].ival));

    return ( (vocRes_t *) result );
}


/**
 *  SM_GETQUERY -- Get a generic Query context from the server.
 * 
 *  @brief   Get a generic Query context from the server.
 *  @fn      stat = sm_getQuery (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_getQuery (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_STRING;
    result->nitems = 1;
    sprintf (result->value[0], "%s", cvo_getServiceURL (objid, pars[0].ival));

    return ( (vocRes_t *) result );
}


/**
 *  SM_ADDPARAMETER -- Add a string parameter to a Query string.
 * 
 *  @brief   Add a string parameter to a Query string.
 *  @fn      stat = sm_addParameter (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_addParameter (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_INT;
    result->nitems = 1;
    sprintf (result->value[0], "%d", 
	cvo_addStringParam (objid, pars[0].str, pars[1].str));

    return ( (vocRes_t *) result );
}


/**
 *  SM_GETQUERYSTRING -- Get the query string to be executed.
 * 
 *  @brief   Get the query string to be executed.
 *  @fn      stat = sm_getQueryString (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_getQueryString (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_INT;
    result->nitems = 1;
    sprintf (result->value[0], "%d", 
	cvo_addStringParam (objid, pars[0].str, pars[1].str));

    return ( (vocRes_t *) result );
}


/**
 *  SM_EXECUTE -- Execute the specified query in the DAL server.
 * 
 *  @brief   Execute the specified query in the DAL server.
 *  @fn      stat = sm_execute (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_execute (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_INT;
    result->nitems = 1;
    sprintf (result->value[0], "%d", cvo_executeQuery (objid));

    return ( (vocRes_t *) result );
}


/**
 *  SM_GETQRESPONSE -- Utility to get QResponse handle from a query.
 * 
 *  @brief   Utility to get QResponse handle from a query.
 *  @fn      stat = sm_getQResponse (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_getQResponse (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_INT;
    result->nitems = 1;
    sprintf (result->value[0], "%d", cvo_getQueryResponse (objid));

    return ( (vocRes_t *) result );
}


/**
 *  SM_EXECUTECSV -- Execute the query and return a CSV table.
 * 
 *  @brief   Execute the query and return a CSV table.
 *  @fn      stat = sm_executeCSV (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_executeCSV (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_STRING;
    result->nitems = 1;
    sprintf (result->value[0], "%s", cvo_executeCSV (objid));

    return ( (vocRes_t *) result );
}


/**
 *  SM_EXECUTETSV -- Execute the query and return a TSV table.
 * 
 *  @brief   Execute the query and return a TSV table.
 *  @fn      stat = sm_executeTSV (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_executeTSV (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_STRING;
    result->nitems = 1;
    sprintf (result->value[0], "%s", cvo_executeTSV (objid));

    return ( (vocRes_t *) result );
}


/**
 *  SM_EXECUTEASCII -- Execute the query and return a TSV table.
 * 
 *  @brief   Execute the query and return a TSV table.
 *  @fn      stat = sm_executeASCII (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_executeASCII (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_STRING;
    result->nitems = 1;
    sprintf (result->value[0], "%s", cvo_executeASCII (objid));

    return ( (vocRes_t *) result );
}


/**
 *  SM_EVECUTEVOTABLE -- Execute the query and return a TSV table.
 * 
 *  @brief   Execute the query and return a TSV table.
 *  @fn      stat = sm_evecuteVOTable (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_evecuteVOTable (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_STRING;
    result->nitems = 1;
    sprintf (result->value[0], "%s", cvo_executeVOTable (objid));

    return ( (vocRes_t *) result );
}


/**
 *  SM_GETRECORDCOUNT -- Get a count of records return by the QResponse.
 * 
 *  @brief   Get a count of records return by the QResponse.
 *  @fn      stat = sm_getRecordCount (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_getRecordCount (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_INT;
    result->nitems = 1;
    sprintf (result->value[0], "%d", cvo_getRecordCount (objid));

    return ( (vocRes_t *) result );
}


/**
 *  SM_GETRECORD -- Get the response record by Number.
 * 
 *  @brief   Get the response record by Number.
 *  @fn      stat = sm_getRecord (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_getRecord (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_INT;
    result->nitems = 1;
    sprintf (result->value[0], "%d", OK);
    cvo_getRecord (objid, pars[0].ival);

    return ( (vocRes_t *) result );
}


/**
 *  SM_GETATTRCOUNT -- Get the attribute record count.
 * 
 *  @brief   Get the attribute record count.
 *  @fn      stat = sm_getAttrCount (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_getAttrCount (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_INT;
    result->nitems = 1;
    sprintf (result->value[0], "%d", cvo_getAttrCount (objid));

    return ( (vocRes_t *) result );
}


/**
 *  SM_GETFIELDATTR -- Get the field attribute string.
 * 
 *  @brief   Get the field attribute string.
 *  @fn      stat = sm_getFieldAttr (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_getFieldAttr (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_STRING;
    result->nitems = 1;
    sprintf (result->value[0], "%s", 
	cvo_getFieldAttr (objid, pars[0].ival, pars[1].str));

    return ( (vocRes_t *) result );
}


/**
 *  SM_GETATTRLIST -- Get a list of attributes for the record.
 * 
 *  @brief   Get a list of attributes for the record.
 *  @fn      stat = sm_getAttrList (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_getAttrList (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_STRING;
    result->nitems = 1;
    sprintf (result->value[0], "%s", cvo_getAttrList (objid));

    return ( (vocRes_t *) result );
}


/**
 *  SM_GETATTRIBUTE -- Get a response record attribute object.
 * 
 *  @brief   Get a response record attribute object.
 *  @fn      stat = sm_getAttribute (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_getAttribute (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_INT;
    result->nitems = 1;
    sprintf (result->value[0], "%d", cvo_getAttribute (objid, pars[0].str));

    return ( (vocRes_t *) result );
}


/**
 *  SM_INTVALUE -- Set an integer value in a record.
 * 
 *  @brief   Set an integer value in a record.
 *  @fn      stat = sm_intValue (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_intValue (int objid, msgParam *pars, int npars)
{
    return ( (vocRes_t *) NULL );
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_INT;
    result->nitems = 1;
    sprintf (result->value[0], "%d", OK);
    cvo_setIntAttr (objid, pars[0].str, pars[1].ival);

    return ( (vocRes_t *) result );
}


/**
 *  SM_FLOATVALUE -- Set a floating-point value in a record.
 * 
 *  @brief   Set a floating-point value in a record.
 *  @fn      stat = sm_floatValue (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_floatValue (int objid, msgParam *pars, int npars)
{
    return ( (vocRes_t *) NULL );
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_INT;
    result->nitems = 1;
    sprintf (result->value[0], "%d", OK); 
    cvo_setFloatAttr (objid, pars[0].str, pars[1].dval);

    return ( (vocRes_t *) result );
}


/**
 *  SM_STRINGVALUE -- Set a string value in a record.
 * 
 *  @brief   Set a string value in a record.
 *  @fn      stat = sm_stringValue (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_stringValue (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_INT;
    result->nitems = 1;
    sprintf (result->value[0], "%d", OK);
    cvo_setStringAttr (objid, pars[0].str, pars[1].str);

    return ( (vocRes_t *) result );
}


/**
 *  SM_GETDATASET -- Download the specified dataset.
 * 
 *  @brief   Download the specified dataset.
 *  @fn      stat = sm_getDataset (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_getDataset (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_INT;
    result->nitems = 1;
    sprintf (result->value[0], "%d", 
	cvo_getDataset (objid, pars[0].str, pars[1].str));

    return ( (vocRes_t *) result );
}


/**
 *  SM_VALIDATEOBJECT -- Method description.
 * 
 *  @brief   Method description.
 *  @fn      stat = sm_validateObject (int objid, msgParam *pars, int npars)
 *
 *  @param   objid       object id handle
 *  @param   pars        message parameter struct
 *  @param   npars       number of parameters
 *  @returns             ERR or number of bytes in return msg
 */
vocRes_t *
sm_validateObject (int objid, msgParam *pars, int npars)
{
    vocRes_t *result = (vocRes_t *) calloc (1, sizeof (vocRes_t));

    result->status = OK;                        /* encode the result    */
    result->type   = TY_INT;
    result->nitems = 1;
    sprintf (result->value[0], "%d", 1);	/* always true		*/

    return ( (vocRes_t *) result );
}
