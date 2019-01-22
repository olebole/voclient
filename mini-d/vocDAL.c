/**
 *
 *  VOCLient Library --  Client interface library to the DAL Server
 *  application.  This API allows non-Java programs to make use of the  DAL
 *  client interface by means of a remote procedure call between this
 *  interface and the DAL server.  State is maintained in the server and
 *  values passed back are language-neutral integer 'handles' on the remote
 *  objects, or simple int/double/string values.
 *
 *  All tasks must initialize the VO Client and establish a connection to
 *  the DAL server by calling voc_initVOClient() before making any service
 *  connections, thereafter new connections may be opened/closed at will.
 *  Convenience procedures allow for easy use of specific services, e.g. Cone
 *  or Siap.  Service-specific parameters may be added to a query using the
 *  voc_add<type>Param() calls.  No action is taken until an execute of the
 *  query is performed, applications may get back a handle on the result and
 *  interrogate attributes directly, the raw VOTable or a CSV/TSV/ASCII
 *  representation of the result may also be returned.
 *
 *  High-Level Functions:
 *  ---------------------
 *
 *               voc_initVOClient (config_opts)
 *              voc_closeVOClient (shutdown_flag)
 *              voc_abortVOClient (errcode, errmsg)
 * 
 *        string = voc_coneCaller (url, ra, dec, sr, otype)
 *  status = voc_coneCallerToFile (url, ra, dec, sr, otype, file)
 *        string = voc_siapCaller (url, ra, dec, rsize, dsize, fmt, otype)
 *  status = voc_siapCallerToFile (url, ra, dec, rsize, dsize, fmt, otype, file)
 *        string = voc_ssapCaller (url, ra, dec, size, band, time, fmt)
 *  status = voc_ssapCallerToFile (url, ra, dec, size, band, time, fmt, file)
 * 
 *         string = voc_getRawURL (url, buflen)
 *
 *
 *  Main DAL Interface Procedures:
 *  ------------------------------
 *
 *       dal = voc_openConnection (svc_url, type, [version])
 *   dal = voc_openConeConnection (svc_url, [version])	    # Utility aliases
 *   dal = voc_openSiapConnection (svc_url, [version])
 *   dal = voc_openSsapConnection (svc_url, [version])
 *            voc_closeConnection (dal)
 * 
 *    count = voc_getServiceCount (dal)
 *              voc_addServiceURL (dal, svc_url)
 *        url = voc_getServiceURL (dal, index)
 * 
 *           query = voc_getQuery (dal, type)
 *       query = voc_getConeQuery (dal, ra, dec, sr)
 *       query = voc_getSiapQuery (dal, ra, dec, ra_size, dec_size, format)
 *       query = voc_getSsapQuery (dal, ra, dec, size, band, time, format)
 *
 *         stat = voc_addIntParam (query, pname, ival)
 *       stat = voc_addFloatParam (query, pname, dval)
 *      stat = voc_addStringParam (query, pname, str)
 * 
 *   url_str = voc_getQueryString (query, type, index)
 *
 *          qr = voc_executeQuery (query)
 *      qr = voc_getQueryResponse (query)
 *      stat = voc_executeQueryAs (query, fname, type)	  (Not Yet Implemented)
 *       csv_tab = voc_executeCSV (query)
 *       tsv_tab = voc_executeTSV (query)
 *       ascii = voc_executeASCII (query)
 *   vot_str = voc_executeVOTable (query)
 *
 *     count = voc_getRecordCount (qr)
 *            rec = voc_getRecord (qr, recnum)
 *         str = voc_getFieldAttr (qr, fieldnum, attr)
 *
 *        attr = voc_getAttribute (rec, char *attrname)
 *       count = voc_getAttrCount (rec)                   
 *     list_str = voc_getAttrList (rec)                   
 *
 *            ival = voc_intValue (attr)
 *          dval = voc_floatValue (attr)
 *          str = voc_stringValue (attr)
 *
 *                 voc_setIntAttr (rec, attrname, ival)   (Not Yet Implemented)
 *               voc_setFloatAttr (rec, attrname, dval)   ( "   "      "      )
 *              voc_setStringAttr (rec, attrname, str)    ( "   "      "      )
 *
 *          stat = voc_getDataset (rec, acref, fname) 
 *
 *
 *  Sesame Name Resolver Interface:
 *  -------------------------------
 *
 *          sr = voc_nameResolver (target)
 *      pos_str = voc_resolverPos (sr)
 *         radeg = voc_resolverRA (sr)
 *       decdeg = voc_resolverDEC (sr)
 * 
 *
 *	Client programs may be written in any language that can interface to
 *  C code.  Sample programs using the interface are provided as is a SWIG
 *  interface definition file.
 *
 *
 *  @file       vocDAL.c
 *  @author     Michael Fitzpatrick, NOAO
 *  @version    December 2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#define _VOCLIENT_LIB_
#include "VOClient.h"
#include "vocServer.h"


extern VOClient *vo;			/* Interface runtime struct	*/

#define SZ_ERRMSG		256
static char errmsg[SZ_ERRMSG];




/******************************************************************************
***********         	    High Level Functions       		     **********
******************************************************************************/

/**
 *  VOC_CONECALLER -- Hi-level Cone Service Caller method.
 *
 *  @brief   Hi-level Cone Service Caller method.
 *  @fn      res = voc_coneCaller (char *url, double ra, double dec,
 *                      double sr, int otype)
 *
 *  @param   url        Base service URL
 *  @param   ra         RA position (ICRS degrees)
 *  @param   dec        Dec position (ICRS degrees)
 *  @param   sr         Search radius (degrees)
 *  @param   otype      Output result type (VOTable, CSV, etc)
 *  @returns            ERR or number of bytes in return msg
 *
 *  Simple all-in-one interface to call a Cone service and return the results
 *  as a text string of either the raw VOTable or a CSV/TSV/Ascii table.
 *
 *  Result string is allocated here and must be freed by the caller.
 */
char *
voc_coneCaller (char *url, double ra, double dec, double sr, int otype)
{
    char *res  = NULL;

    DAL       cone;                             /* DAL Connection        */
    Query     query;                            /* query handle          */


    /*  Initialize the VOClient code.  Error messages are printed by the
     *  interface so we just quit if there is a problem.
     */
    if (!vo && voc_initVOClient (NULL) == ERR)
        return ((char *)NULL);

    /*  Get a new connection to the named service.
     */
    cone = voc_openConeConnection (url);    	/* open a connection 	*/

    query = voc_getConeQuery (cone, ra, dec, sr); /* form a query 	*/

    if (otype == VOC_CSV)
        res = voc_executeCSV (query);
    else if (otype == VOC_TSV)
        res = voc_executeTSV (query);
    else if (otype == VOC_ASCII)
        res = voc_executeASCII (query);
    else if (otype == VOC_VOTABLE)
        res = voc_executeVOTable (query);

    voc_closeConnection (cone);                 /* close the cone connection */
    voc_closeVOClient (0);                      /* clean up and shutdown */

    return ((char *) res);
}


/**
 *  VOC_CONECALLERTOFILE -- Hi-level Cone Service Caller method.
 *
 *  @brief   Hi-level Cone Service Caller method.
 *  @fn      res = voc_coneCallerToFile (char *url, double ra, double dec,
 *                      double sr, int otype, char *file)
 *
 *  @param   url        Base service URL
 *  @param   ra         RA position (ICRS degrees)
 *  @param   dec        Dec position (ICRS degrees)
 *  @param   sr         Search radius (degrees)
 *  @param   otype      Output result type (VOTable, CSV, etc)
 *  @param   file       Output file name
 *  @returns            ERR or number of bytes in return msg
 *
 *  Simple all-in-one interface to call a Cone service and return the results
 *  as a text string of either the raw VOTable or a CSV/TSV/Ascii table.
 *
 *  Result string is allocated here and must be freed by the caller.
 */
int
voc_coneCallerToFile (char *url, double ra, double dec, double sr, int otype, 
	char *file)
{
    if (url) {
	char *res = (char *) NULL;
	int   fd = 0;

        if ((fd = open (file, O_CREAT|O_TRUNC|O_WRONLY, DEFFILEMODE)) < 0) {
	    fprintf (stderr, "Error: cannot open file '%s'\n", file);
	    return (ERR);
        }

	if ( (res = voc_coneCaller (url, ra, dec, sr, otype)) ) {
	    svr_fileWrite (fd, res, strlen (res));
	    free ((void *) res);
	}
	close (fd);
    }

    return (OK);
}


/**
 *  VOC_SIAPCALLER -- Hi-level SIA Service Caller method.
 *
 *  @brief   Hi-level SIA Service Caller method.
 *  @fn      res = voc_siapCaller (char *url, double ra, double dec,
 *                      double sr, int otype)
 *
 *  @param   url        Base service URL
 *  @param   ra         RA position (ICRS degrees)
 *  @param   dec        Dec position (ICRS degrees)
 *  @param   sr         Search radius (degrees)
 *  @param   otype      Output result type (VOTable, CSV, etc)
 *  @returns            ERR or number of bytes in return msg
 *
 *  Simple all-in-one interface to call a SIA service and return the results
 *  as a text string of either the raw VOTable or a CSV/TSV/Ascii table.
 *
 *  Result string is allocated here and must be freed by the caller.
 */
char *
voc_siapCaller (char *url, double ra, double dec, double rsize, double dsize, 
	char *fmt, int otype)
{
    char *res  = NULL;

    DAL       siap;                             /* DAL Connection        */
    Query     query;                            /* query handle          */


    /*  Initialize the VOClient code.  Error messages are printed by the
     *  interface so we just quit if there is a problem.
     */
    if (!vo && voc_initVOClient (NULL) == ERR)
        return ((char *) NULL);

    /*  Get a new connection to the named service.
     */
    siap = voc_openSiapConnection (url);    	/* open a connection */

    /*  Form a query.  Here we'll use the one search size we're given for
     *  both the RA,DEC sizes, and specify a null format.
     */
    query = voc_getSiapQuery (siap, ra, dec, rsize, dsize, fmt);

    if (otype == VOC_CSV)
        res = voc_executeCSV (query);
    else if (otype == VOC_TSV)
        res = voc_executeTSV (query);
    else if (otype == VOC_ASCII)
        res = voc_executeASCII (query);
    else if (otype == VOC_VOTABLE)
        res = voc_executeVOTable (query);

    voc_closeConnection (siap);                 /* close the siap connection */
    voc_closeVOClient (0);                      /* clean up and shutdown */

    return ((char *) res);
}


/**
 *  VOC_SIAPCALLERTOFILE -- Hi-level SIA Service Caller method.
 *
 *  @brief   Hi-level SIA Service Caller method.
 *  @fn      res = voc_siapCallerToFile (char *url, double ra, double dec,
 *                      double sr, int otype, char *file)
 *
 *  @param   url        Base service URL
 *  @param   ra         RA position (ICRS degrees)
 *  @param   dec        Dec position (ICRS degrees)
 *  @param   sr         Search radius (degrees)
 *  @param   otype      Output result type (VOTable, CSV, etc)
 *  @param   file       Output file name
 *  @returns            ERR or number of bytes in return msg
 *
 *  Simple all-in-one interface to call a SIA service and return the results
 *  as a text string of either the raw VOTable or a CSV/TSV/Ascii table.
 *
 *  Result string is allocated here and must be freed by the caller.
 */
int
voc_siapCallerToFile (char *url, double ra, double dec, double rsize, 
	double dsize, char *fmt, int otype, char *file)
{
    if (url) {
	char *res = (char *) NULL;
	int   fd = 0;

        if ((fd = open (file, O_CREAT|O_TRUNC|O_WRONLY, DEFFILEMODE)) < 0) {
	    fprintf (stderr, "Error: cannot open file '%s'\n", file);
	    return (ERR);
        }

	if ( (res = voc_siapCaller (url, ra, dec, rsize, dsize, fmt, otype)) ) {
	    svr_fileWrite (fd, res, strlen (res));
	    free ((void *) res);
	}
	close (fd);
    }

    return (OK);
}


/**
 *  VOC_SSAPCALLER -- Hi-level SSA Service Caller method.
 *
 *  @brief   Hi-level SSA Service Caller method.
 *  @fn      res = voc_ssapCaller (char *url, double ra, double dec,
 *                      double sr, int otype)
 *
 *  @param   url        Base service URL
 *  @param   ra         RA position (ICRS degrees)
 *  @param   dec        Dec position (ICRS degrees)
 *  @param   sr         Search radius (degrees)
 *  @param   otype      Output result type (VOTable, CSV, etc)
 *  @returns            ERR or number of bytes in return msg
 *
 *  Simple all-in-one interface to call a SSA service and return the results
 *  as a text string of either the raw VOTable or a CSV/TSV/Ascii table.
 *
 *  Result string is allocated here and must be freed by the caller.
 */
char *
voc_ssapCaller (char *url, double ra, double dec, double size, 
	char *band, char *time, char *fmt, int otype)
{
    char *res  = NULL;

    DAL       ssap;                             /* DAL Connection        */
    Query     query;                            /* query handle          */


    /*  Initialize the VOClient code.  Error messages are printed by the
     *  interface so we just quit if there is a problem.
     */
    if (!vo && voc_initVOClient (NULL) == ERR)
        return ((char *) NULL);

    /*  Get a new connection to the named service.
     */
    ssap = voc_openSsapConnection (url);    	/* open a connection */

    /*  Form a query.  Here we'll use the one search size we're given for
     *  both the RA,DEC sizes, and specify a null format.
     */
    query = voc_getSsapQuery (ssap, ra, dec, size, band, time, fmt);

    if (otype == VOC_CSV)
        res = voc_executeCSV (query);
    else if (otype == VOC_TSV)
        res = voc_executeTSV (query);
    else if (otype == VOC_ASCII)
        res = voc_executeASCII (query);
    else if (otype == VOC_VOTABLE)
        res = voc_executeVOTable (query);

    voc_closeConnection (ssap);                 /* close the ssap connection */
    voc_closeVOClient (0);                      /* clean up and shutdown */

    return ((char *) res);
}


/**
 *  VOC_SSAPCALLERTOFILE -- Hi-level SSA Service Caller method.
 *
 *  @brief   Hi-level SSA Service Caller method.
 *  @fn      res = voc_ssapCallerToFile (char *url, double ra, double dec,
 *                      double sr, int otype, char *file)
 *
 *  @param   url        Base service URL
 *  @param   ra         RA position (ICRS degrees)
 *  @param   dec        Dec position (ICRS degrees)
 *  @param   sr         Search radius (degrees)
 *  @param   otype      Output result type (VOTable, CSV, etc)
 *  @param   file       Output file name
 *  @returns            ERR or number of bytes in return msg
 *
 *  Simple all-in-one interface to call a SSA service and return the results
 *  as a text string of either the raw VOTable or a CSV/TSV/Ascii table.
 *
 *  Result string is allocated here and must be freed by the caller.
 */
int
voc_ssapCallerToFile (char *url, double ra, double dec, double size, 
	char *band, char *time, char *fmt, int otype, char *file)
{
    if (url) {
	char *res = (char *) NULL;
	int   fd = 0;

        if ((fd = open (file, O_CREAT|O_TRUNC|O_WRONLY, DEFFILEMODE)) < 0) {
	    fprintf (stderr, "Error: cannot open file '%s'\n", file);
	    return (ERR);
        }

	if ( (res = voc_ssapCaller (url, ra, dec, size, band, time, 
		fmt, otype)) ) {
	    	    svr_fileWrite (fd, res, strlen (res));
	    	    free ((void *) res);
	}
	close (fd);
    }

    return (OK);
}


/**
 *  VOC_GETRAWURL -- Get a raw URL to a returned string.
 *
 *  @brief   Get a raw URL to a returned string.
 *  @fn      raw = voc_getRawURL (char *url, int *nbytes)
 *
 *  @param   url        URL to retrieve
 *  @param   nbytes     number of bytes read
 *  @returns            Result string from URL
 *
 *  Note this is only for text files because of the assumed use of EOF
 *  that may actually be a NULL byte in a binary data stream.
 */
char *
voc_getRawURL (char *url, int *nbytes)
{
    vocRes_t *result = (vocRes_t *) NULL;
    vocMsg_t *msg = (vocMsg_t *) msg_newCallMsg (0, "getRawURL", 0);
    char      c, *ip, *buf = NULL, *raw = NULL;
    int       nb = -1;


    msg_addStringParam (msg, url);

    /* Send message and read result.
     */
    if (msg_resultStatus ((result = msg_sendMsg (vo->io_chan, msg))) == ERR) {
	if (!vo->quiet)
	    fprintf (stderr, "ERROR: getRawURL failed\n");
	memset (errmsg, 0, SZ_ERRMSG);
	strcpy (errmsg, "ERROR: getRawURL failed");
	*nbytes = -1;
    } else {
	raw = ip = msg_getBuffer (result);
	nb = result->buflen;

        /* Skip leading newlines	
        */
        while ((c = *ip) == '\n') {
	    ip++,  nb--;
        }
	buf = calloc (result->buflen+2, sizeof (char));
	memmove (buf, ip, nb);
        *nbytes = nb;
    }

    if (msg)    free ((void *)msg); 	/* free the pointers 		*/
    if (raw)    free ((void *)raw);
    if (result) free ((void *)result);

    return ((char *)buf);
}


/**
 *  VOC_OPENCONNECTION -- Open a new DAL context connection.
 *
 *  @brief   Open a new DAL context connection.
 *  @fn      dal = voc_openConnection (char *svc_url, int type, ...)
 *
 *  @param   svc_url    base service URL
 *  @param   type       DAL connection type
 *  @returns            Handle to new DAL connection object
 */
DAL
voc_openConnection (char *service_url, int type, ...)
{
    DAL	      dal = (int) VOC_NULL;
    vocRes_t *result = (vocRes_t *) NULL;
    vocMsg_t *msg = (vocMsg_t *) msg_newCallMsg (0, "newConnection", 0);
    char     *version = "1";
    va_list   argp;


    /*  Make sure we've been initialized properly first.
     */
    if (vo == (VOClient *) NULL) {
	if (voc_initVOClient (NULL) == ERR) {
	    if (!vo->quiet)
	        fprintf (stderr, "ERROR: Can't initialize VO Client....\n");
	    exit (1);
	} else if (VOC_DEBUG)
	    printf ("Warning: Initializing VO Client....\n");
    }

    switch (type) {
    case DAL_CONN:  	msg_addStringParam (msg, "DAL");  break;
    case CONE_CONN: 	msg_addStringParam (msg, "Cone"); break;
    case SIAP_CONN: 	msg_addStringParam (msg, "Siap"); break;
    case SSAP_CONN: 	msg_addStringParam (msg, "Ssap"); break;
    default:
	if (!vo->quiet)
	    fprintf (stderr, "ERROR: Invalid newConnection request type=%d\n",
		type);
        if (msg) free ((void *) msg); 	/* free the pointers 		*/
	return ((DAL) VOC_NULL);
    }


    /*  Get the optional version parameter.
     */
    va_start (argp, type);            		/* get a single string    */
    version = va_arg ((argp), char *);
    va_end (argp);

    msg_addStringParam (msg, version);  	/* only used for SIAv2 */
    

    /* Send message and read result.  */
    if (msg_resultStatus ((result = msg_sendMsg (vo->io_chan, msg))) == ERR) {
	if (!vo->quiet)
	    fprintf (stderr, "ERROR: cannot open connection\n");
    } else
	dal = msg_getIntResult (result, 0);


    /* If we were given a service_url add it as a param and send message.
     */
    if (service_url)
	voc_addServiceURL (dal, service_url);


    /* Free the message and return the object ID.
     */
    if (msg)    free ((void *)msg);
    if (result) free ((void *)result);

    return (dal);
}



/******************************************************************************
 *  Utility aliases for code readability.  We call the generic openConnection()
 *  procedure so that we can use the VOClient initialization.
 *****************************************************************************/

/**
 *  VOC_OPENCONECONNECTION -- Open a new DAL Cone connection.
 *
 *  @brief   Open a new DAL Cone connection.
 *  @fn      dal = voc_openConeConnection (char *svc_url, ...)
 *
 *  @param   svc_url    base service URL
 *  @param   type       DAL connection type
 *  @returns            Handle to new DAL connection object
 */
DAL voc_openConeConnection (char *service_url, ...)
{
    DAL   dal = (DAL) NULL;
    char *version = "1";
    va_list   argp;

    /*  Get the optional version parameter.
     */
    va_start (argp, service_url);
    version = va_arg ((argp), char *);
    va_end (argp);

    return ( (dal = voc_openConnection (service_url, CONE_CONN, version)) );
}


/**
 *  VOC_OPENSIAPCONNECTION -- Open a new DAL SIAP connection.
 *
 *  @brief   Open a new DAL SIAP connection.
 *  @fn      dal = voc_openSiapConnection (char *svc_url, ...)
 *
 *  @param   svc_url    base service URL
 *  @param   type       DAL connection type
 *  @returns            Handle to new DAL connection object
 */
DAL voc_openSiapConnection (char *service_url, ...)
{
    DAL   dal = (DAL) NULL;
    char *version = "1";
    va_list   argp;

    /*  Get the optional version parameter.
     */
    va_start (argp, service_url);
    version = va_arg ((argp), char *);
    va_end (argp);

    return ( (dal = voc_openConnection (service_url, SIAP_CONN, version)) );
}


/**
 *  VOC_OPENSSAPCONNECTION -- Open a new DAL SSAP connection.
 *
 *  @brief   Open a new DAL SSAP connection.
 *  @fn      dal = voc_openSsapConnection (char *svc_url, ...)
 *
 *  @param   svc_url    base service URL
 *  @param   type       DAL connection type
 *  @returns            Handle to new DAL connection object
 */
DAL voc_openSsapConnection (char *service_url, ...)
{
    DAL   dal = (DAL) NULL;
    char *version = "1";
    va_list   argp;

    /*  Get the optional version parameter.
     */
    va_start (argp, service_url);
    version = va_arg ((argp), char *);
    va_end (argp);

    return ( (dal = voc_openConnection (service_url, SSAP_CONN, version)) );
}


/**
 *  VOC_CLOSECONNETION -- Close the requested connection.
 *
 *  @brief   Close the requested connection.
 *  @fn      voc_closeConnection (DAL dal)
 *
 *  @param   dal        DAL connection object handle
 *  @returns            nothing
 */
void
voc_closeConnection (DAL dal)
{
    vocRes_t *result;
    vocMsg_t *msg = (vocMsg_t *) msg_newCallMsg (dal, "removeConnection", 0);

    if (msg_resultStatus ((result = msg_sendMsg (vo->io_chan, msg))) == ERR) {
	if (!vo->quiet)
	    fprintf (stderr, "ERROR: cannot close connection\n");
    }

    if (msg)    free ((void *)msg); 	/* free the pointers 		*/
    if (result) free ((void *)result);
}


/**
 *  VOC_GETSERVICECOUNT -- Get a count of the number of services associated
 *  with the given DAL connection context.
 *
 *  @brief   Get service count associated with the DAL context.
 *  @fn      count = voc_getServiceCount (DAL dal)
 *
 *  @param   dal        DAL connection object handle
 *  @returns            Number of services associated with the DAL handle
 */
int
voc_getServiceCount (DAL dal)
{
    vocMsg_t *msg = (vocMsg_t *) msg_newCallMsg (dal, "getServiceCount", 0);
    vocRes_t *result;
    int       count = -1;

    
    /* Read result and check for any faults.
     */
    if (msg_resultStatus ((result = msg_sendMsg (vo->io_chan, msg))) == ERR) {
	if (!vo->quiet)
	    fprintf (stderr, "ERROR: getServiceCount() failed\n");
    } else
	count = msg_getIntResult (result, 0);

    if (VOC_DEBUG) 
	fprintf (stderr, "svcCount: DAL=%ld count=%ld\n",(long)dal,(long)count);

    if (msg)    free ((void *)msg); /* free the pointers and return the objID */
    if (result) free ((void *)result);

    return (count);
}


/**
 *  VOC_ADDSERVICEURL --  Add a service URL to the specified connection.
 *
 *  @brief   Add a service URL to the specified connection.
 *  @fn      voc_addServiceURL (DAL dal, char *service_url)
 *
 *  @param   dal        DAL connection object handle
 *  @param   svc_url    base service url
 *  @returns            nothing
 */
void
voc_addServiceURL (DAL dal, char *service_url)
{
    vocRes_t *result = (vocRes_t *) NULL;
    vocMsg_t *msg = (vocMsg_t *) msg_newCallMsg (dal, "addServiceURL", 0);

    /* If we were given a service_url add it as a param and send message.
     */
    if (service_url) {
	msg_addStringParam (msg, service_url);
    
        /* Read result and check for any faults.
         */
        if (msg_resultStatus ((result = msg_sendMsg(vo->io_chan,msg))) == ERR) {
	    if (!vo->quiet)
	        fprintf (stderr, "ERROR: cannot add service URL: %s\n",
		    service_url);
	}

    } else if (!vo->quiet)
	fprintf (stderr, "ERROR: empty service URL\n");

    if (msg)    free ((void *)msg); 	/* free the pointers 		*/
    if (result) free ((void *)result);
}


/**
 *  VOC_GETSERVICEURL --  Get the requested service URL for the connection.
 *
 *  @brief   Get the requested service URL for the connection.
 *  @fn      str = voc_getServiceURL (DAL dal, int index)
 *
 *  @param   dal        DAL connection object handle
 *  @param   index      service index (0-based)
 *  @returns            requested service URL
 */
char *
voc_getServiceURL (DAL dal, int index)
{
    vocRes_t *result = (vocRes_t *) NULL;
    vocMsg_t *msg = (vocMsg_t *) msg_newCallMsg (dal, "getServiceURL", 0);
    char  *url = (char *) NULL;


    msg_addIntParam (msg, index);
    
    /* Send message and read result.
     */
    if (msg_resultStatus ((result = msg_sendMsg (vo->io_chan, msg))) == ERR) {
	if (!vo->quiet)
	    fprintf (stderr, "ERROR: getServiceURL() failed\n");
    } else
	url = msg_getStringResult (result, 0);


    if (msg)    free ((void *)msg); 	/* free the pointers 		*/
    if (result) free ((void *)result);

    return (url);
}


/**
 *  VOC_GETQUERY --  Get a generic Query context from the server.
 *
 *  @brief   Method description.
 *  @fn      stat = sm_method (int objid, msgParam *pars, int npars)
 *
 *  @param   dal        DAL connection object handle
 *  @param   type       query type (not used)
 *  @returns            Query object handle
 */
Query
voc_getQuery (DAL dal, int type)
{
    Query query = (Query) VOC_NULL;
    vocRes_t *result = (vocRes_t *) NULL;
    vocMsg_t *msg = (vocMsg_t *) msg_newCallMsg (dal, "getQuery", 0);

    switch (type) {
    case DAL_CONN:  	msg_addStringParam (msg, "DAL");  break;
    case CONE_CONN: 	msg_addStringParam (msg, "Cone"); break;
    case SIAP_CONN: 	msg_addStringParam (msg, "Siap"); break;
    case SSAP_CONN: 	msg_addStringParam (msg, "Ssap"); break;
    default:
	if (!vo->quiet)
	    fprintf (stderr, "ERROR: Invalid getQuery request type=%d\n", type);
        if (msg) free ((void *)msg); 	/* free the pointers 		*/
	return (query);
    }
    
    /* Send message and read result.
     */
    if (msg_resultStatus ((result = msg_sendMsg (vo->io_chan, msg))) == ERR) {
	if (!vo->quiet)
	    fprintf (stderr, "ERROR: getQuery() failed\n");
    } else
	query = msg_getIntResult (result, 0);

    if (msg)    free ((void *)msg); 	/* free the pointers 		*/
    if (result) free ((void *)result);

    return (query);
}


/**
 *  VOC_GETCONEQUERY --  Get a query for a Cone service.
 *
 *  We take the typical Cone arguments in the interface routine but break
 *  it down into a series of messages to get a general Query object and
 *  then add parameters.
 *
 *  @brief   Get a query for a Cone service.
 *  @fn      query = voc_getConeQuery (DAL dal, double ra, double dec,
 *                      double sr)
 *
 *  @param   dal        DAL connection object handle
 *  @param   ra         RA of position (ICRS degrees)
 *  @param   dec        Dec of position (ICRS degrees)
 *  @param   sr         Search radius (degrees)
 *  @returns            ERR or number of bytes in return msg
 */
Query
voc_getConeQuery (DAL dal, double ra, double dec, double sr)
{
    Query query = voc_getQuery (dal, CONE_CONN);

    if (query == (Query) VOC_NULL) {
	if (!vo->quiet)
	    fprintf (stderr, "ERROR: cannot get Query\n");
	return (query);
    }

    /* Allow for a negative search radius to inhibit adding the standard
    ** cone parameters.  This allows us to access the raw ServiceURL and
    ** in the case of Vizier, to access the entire table.
    */
    if (sr >= 0.0) {
        voc_addFloatParam (query, "RA", (double) ra);
        voc_addFloatParam (query, "DEC",(double) dec);
        voc_addFloatParam (query, "SR", (double) sr);
    }
    
    /*  Add a RUNID string to the query for logging.
     */
    if (vo->use_runid)
        voc_addStringParam (query, "RUNID", vo->runid);

    return (query);
}


/**
 *  VOC_GETSIAPQUERY --  Get a query for a SIAP service.
 *
 *  We take the typical SIA arguments in the interface routine but break
 *  it down into a series of messages to get a general Query object and
 *  then add parameters.
 *
 *  @brief   Get a query for a SIAP service.
 *  @fn      query = voc_getSiapQuery (DAL dal, double ra, double dec,
 *                      double ra_size, dec_size, char *format)
 *
 *  @param   dal        DAL connection object handle
 *  @param   ra         RA of position (ICRS degrees)
 *  @param   dec        Dec of position (ICRS degrees)
 *  @param   ra_size    RA search size (degrees)
 *  @param   dec_size   Dec search size (degrees)
 *  @param   format     Requested format
 *  @returns            ERR or number of bytes in return msg
 */
Query
voc_getSiapQuery (DAL dal, double ra, double dec, double ra_size, 
	double dec_size, char *format)
{
    Query query = voc_getQuery (dal, SIAP_CONN);
    char  pos[SZ_PBUF], size[SZ_PBUF], fmt[SZ_PBUF];

    if (query == (Query) VOC_NULL) {
	if (!vo->quiet)
	    fprintf (stderr, "ERROR: cannot get Query\n");
	return (query);
    }

    memset (pos,  0, SZ_PBUF);			/* clear arrays 	*/
    memset (size, 0, SZ_PBUF);
    memset (fmt,  0, SZ_PBUF);


    /*  Encode the RA,DEC as a POS string.
     */
    sprintf (pos, "%g,%g", ra, dec);
    voc_addStringParam (query, "POS", pos);


    /*  Encode the search box size, collapse to a single value of they're
     *  the same size. 
     */
    if (ra_size == dec_size)
        sprintf (size, "%g", ra_size);
    else
        sprintf (size, "%g,%g", ra_size,dec_size);

    voc_addStringParam (query, "SIZE", size);


    /*  Encode the format request if we're given one.
     */
    if (format)
        voc_addStringParam (query, "FORMAT", format);
    
    /*  Add a RUNID string to the query for logging.
     */
    if (vo->use_runid)
	voc_addStringParam (query, "RUNID", vo->runid);

    return (query);
}


/**
 *  VOC_GETSSAPQUERY --  Get a query for a SSAP service.
 *
 *  We take the typical SSA arguments in the interface routine but break
 *  it down into a series of messages to get a general Query object and
 *  then add parameters.
 *
 *  @brief   Get a query for a SSAP service.
 *  @fn      query = voc_getSsapQuery (DAL dal, double ra, double dec,
 *                      double size, char *band, char *tim, char *format)
 *
 *  @param   dal        DAL connection object handle
 *  @param   ra         RA of position (ICRS degrees)
 *  @param   dec        Dec of position (ICRS degrees)
 *  @param   size       Search size (degrees)
 *  @param   band       BAND search string
 *  @param   tim        TIME search string
 *  @param   format     Requested format
 *  @returns            ERR or number of bytes in return msg
 */
Query
voc_getSsapQuery (DAL dal, double ra, double dec, double size, char *band, 
    char *tim, char *format)
{
    Query query = voc_getQuery (dal, SSAP_CONN);
    char  pos[SZ_PBUF], sz[SZ_PBUF];


    if (query == (Query) VOC_NULL) {
	if (!vo->quiet)
	    fprintf (stderr, "ERROR: cannot get Query\n");
	return (query);
    }

    memset (pos,  0, SZ_PBUF);			/* clear arrays 	*/
    memset (sz,   0, SZ_PBUF);


    /*  Encode the RA,DEC as a POS string, and the search size as POS.
     */
    sprintf (pos, "%g,%g", ra, dec);
    voc_addStringParam (query, "POS", pos);
    sprintf (sz, "%g", size);
    voc_addStringParam (query, "SIZE", sz);


    /*  Encode the band, time and format requests if we're given one.
     */
    if (band)
        voc_addStringParam (query, "BAND", band);
    if (tim)
        voc_addStringParam (query, "TIME", tim);
    if (format)
        voc_addStringParam (query, "FORMAT", format);
    
    /*  Add a RUNID string to the query for logging.
     */
    if (vo->use_runid)
	voc_addStringParam (query, "RUNID", vo->runid);

    return (query);
}


/**
 *  VOC_ADDINTPARAM --  Add an integer parameter to a Query string.
 *
 *  @brief   Add an integer parameter to a Query string.
 *  @fn      stat = voc_addIntParam (Query query, char *name, int ival)
 *
 *  @param   query       Query object handle
 *  @param   name        parameter name
 *  @param   ival        integer value
 *  @returns             status (OK or ERR)
 */
int
voc_addIntParam (Query query, char *name, int ival)
{
    char  buf[SZ_PBUF];

    memset (buf, 0, SZ_PBUF);
    sprintf (buf, "%d", ival);
    return ( voc_addStringParam (query, name, buf) );
}


/**
 *  VOC_ADDFLOATPARAM --  Add a floating-point parameter to a Query string.
 *
 *  @brief   Add a floating-point parameter to a Query string.
 *  @fn      stat = voc_addFloatParam (Query query, char *name, double dval)
 *
 *  @param   query       Query object handle
 *  @param   name        parameter name
 *  @param   dval        double-precision value
 *  @returns             status (OK or ERR)
 */
int
voc_addFloatParam (Query query, char *name, double dval)
{
    char  buf[SZ_PBUF];

    memset (buf, 0, SZ_PBUF);
    sprintf (buf, "%g", dval);
    return ( voc_addStringParam (query, name, buf) );
}


/**
 *  VOC_ADDSTRINGPARAM --  Add a String parameter to a Query string.
 *
 *  @brief   Add a String parameter to a Query string.
 *  @fn      stat = voc_addStringParam (Query query, char *name, char *str)
 *
 *  @param   query       Query object handle
 *  @param   name        parameter name
 *  @param   str         string parameter
 *  @returns             status (OK or ERR)
 */
int
voc_addStringParam (Query query, char *name, char *str)
{
    vocRes_t *result = (vocRes_t *) NULL;
    vocMsg_t *msg = (vocMsg_t *) msg_newCallMsg (query, "addParameter", 0);
    int status = OK;


    /* Add the parameters to the message. */
    msg_addStringParam (msg, name);
    msg_addStringParam (msg, str);

    /* send the message	*/
    status = msg_resultStatus ( (result = msg_sendMsg (vo->io_chan, msg)) );

    if (msg)    free ((void *) msg); 		/* free the pointers 	*/
    if (result) free ((void *) result);

    return (status);
}


/**
 *  VOC_GETQUERYSTRING -- Get the complete query string that will be
 *  executed.
 *
 *  @brief   Get the complete query string that will be executed.
 *  @fn      str = voc_getQueryString (Query query, int type, int index)
 *
 *  @param   query       Query object handle
 *  @param   type        query type (not used)
 *  @param   index       service index (not used)
 *  @returns             query URL to be executed
 */
char *
voc_getQueryString (Query query, int type, int index)
{
    vocRes_t *result = (vocRes_t *) NULL;
    vocMsg_t *msg = (vocMsg_t *) msg_newCallMsg (query, "getQueryString", 0);
    char  *qstring = (char *) NULL;

    switch (type) {
    case DAL_CONN:  	msg_addStringParam (msg, "DAL");  break;
    case CONE_CONN: 	msg_addStringParam (msg, "Cone"); break;
    case SIAP_CONN: 	msg_addStringParam (msg, "Siap"); break;
    case SSAP_CONN: 	msg_addStringParam (msg, "Ssap"); break;
    default:
	if (!vo->quiet)
	    fprintf (stderr, "ERROR: Invalid QueryString request type=%d\n",
		type);
        if (msg) free ((void *) msg); 	/* free the pointers 		*/
	return ((char *) NULL);
    }
    
    msg_addIntParam (msg, index);
    
    /* Send message and read result.
     */
    if (msg_resultStatus ((result = msg_sendMsg (vo->io_chan, msg))) == ERR) {
	if (!vo->quiet)
	    fprintf (stderr, "ERROR: cannot get query string URL\n");
    } else
	qstring = msg_getStringResult (result, 0);

    if (msg)    free ((void *) msg); 	/* free the pointers 		*/
    if (result) free ((void *) result);

    return (qstring);
}


/**
 *  VOC_EXECUTEQUERY --  Execute the specified query in the DAL server.
 *  the QResponse object handle.
 *
 *  @brief   Method description.
 *  @fn      qr = voc_executeQuery (Query query)
 *
 *  @param   query       Query object handle
 *  @returns             QResponse object handle
 */
QResponse
voc_executeQuery (Query query)
{
    vocRes_t *result = (vocRes_t *) NULL;
    vocMsg_t *msg = (vocMsg_t *) msg_newCallMsg (query, "execute", 0);
    QResponse qr  = (QResponse) VOC_NULL;

    /* Send message and read result.
     */
    if (msg_resultStatus ((result = msg_sendMsg (vo->io_chan, msg))) == ERR) {
	if (!vo->quiet)
	    fprintf (stderr, "ERROR: executeQuery failed\n");
	memset (errmsg, 0, SZ_ERRMSG);
	strcpy (errmsg, "executeQuery failed");
	qr = -1;
    } else
	qr = msg_getIntResult (result, 0);

    if (msg)    free ((void *)msg); 	/* free the pointers 		*/
    if (result) free ((void *)result);

    return (qr);
}


/**
 *  VOC_GETQUERYRESPONSE --  Utility to get QResponse handle from Query.
 *
 *  We use this when the query itself was executed by a routine that doesn't
 *  directly return it  (e.g. voc_executeCSV()).
 *
 *  @brief   Utility to get QResponse handle from Query.
 *  @fn      qr = voc_getQueryResponse (Query query)
 *
 *  @param   query       Query object handle
 *  @returns             QResponse object handle
 */
QResponse
voc_getQueryResponse (Query query)
{
    vocRes_t *result = (vocRes_t *) NULL;
    vocMsg_t *msg = (vocMsg_t *) msg_newCallMsg (query, "getQResponse", 0);
    QResponse qr  = (QResponse) VOC_NULL;

    /* Send message and read result.
     */
    if (msg_resultStatus ((result = msg_sendMsg (vo->io_chan, msg))) == ERR) {
	if (!vo->quiet)
	    fprintf (stderr, "ERROR: getQueryResponse failed\n");
	memset (errmsg, 0, SZ_ERRMSG);
	strcpy (errmsg, "getQueryResponse failed");
	qr = -1;
    } else
	qr = msg_getIntResult (result, 0);

    if (msg)    free ((void *)msg); 	/* free the pointers 		*/
    if (result) free ((void *)result);

    return (qr);
}


/**
 *  VOC_GETERRMSG -- Get the last error message.
 *
 *  @brief  Get the last error message.
 *  @fn     msg = voc_getErrMsg (void)
 *
 *  @returns            The last error message.
 */
char *
voc_getErrMsg ()
{
    return (errmsg);                    /* Not Yet Implemented  */
}


/**
 *  VOC_EXECUTECSV -- Execute the query and return a CSV table.
 *
 *  @brief  Execute the query and return a CSV table.
 *  @fn     result = voc_executeCSV (Query query)
 *
 *  @param  query       DAL query object
 *  @returns            The query result in CSV format
 */
char *
voc_executeCSV (Query query)
{
    vocRes_t *result = (vocRes_t *) NULL;
    vocMsg_t *msg = (vocMsg_t *) msg_newCallMsg (query, "executeCSV", 0);
    char      *raw = NULL, *csv = NULL, *rp = NULL, *buf = NULL;

    /* Send message and read result.
     */
    if (msg_resultStatus ((result = msg_sendMsg (vo->io_chan, msg))) == ERR) {
	if (!vo->quiet)
	    fprintf (stderr, "ERROR: executeCSV failed\n");
	memset (errmsg, 0, SZ_ERRMSG);
	buf = msg_getBuffer (result);
	if (buf)
	    strcpy (errmsg, (buf ? buf : "executeCSV failed"));
    } else
	raw = msg_getBuffer (result);

    if (raw) {
	for (rp=raw; *rp && isspace(*rp); rp++)
	    ;				/* skip leading whitespace	*/
	csv = strdup (rp);
    } else
	csv = NULL;

    if (msg)    free ((void *)msg); 	/* free the pointers 		*/
    if (raw)    free ((void *)raw);
    if (result) free ((void *)result);

    return (csv);
}


/**
 *  VOC_EXECUTETSV -- Execute the query and return a TSV table.
 *
 *  @brief  Execute the query and return a TSV table.
 *  @fn     result = voc_executeTSV (Query query)
 *
 *  @param  query       DAL query object
 *  @returns            The query result in TSV format
 */
char *
voc_executeTSV (Query query)
{
    vocRes_t *result = (vocRes_t *) NULL;
    vocMsg_t *msg = (vocMsg_t *) msg_newCallMsg (query, "executeTSV", 0);
    char      *raw = NULL, *tsv = NULL, *rp = NULL, *buf = NULL;

    /* Send message and read result.
     */
    if (msg_resultStatus ((result = msg_sendMsg (vo->io_chan, msg))) == ERR) {
	if (!vo->quiet)
	    fprintf (stderr, "ERROR: executeTSV failed\n");
	memset (errmsg, 0, SZ_ERRMSG);
	buf = msg_getBuffer (result);
	if (buf)
	    strcpy (errmsg, (buf ? buf : "executeTSV failed"));
    } else
	raw = msg_getBuffer (result);

    if (raw) {
	for (rp=raw; *rp && isspace(*rp); rp++)
	    ;				/* skip leading whitespace	*/
	tsv = strdup (rp);
    } else
	tsv = NULL;


    if (msg)    free ((void *)msg); 	/* free the pointers 		*/
    if (raw)    free ((void *)raw);
    if (result) free ((void *)result);

    return (tsv);
}


/**
 *  VOC_EXECUTEASCII -- Execute the query and return an ASCII table.
 *
 *  @brief  Execute the query and return an ASCII table.
 *  @fn     result = voc_executeASCII (Query query)
 *
 *  @param  query       DAL query object
 *  @returns            The query result in ASCII format
 */
char *
voc_executeASCII (Query query)
{
    vocRes_t *result = (vocRes_t *) NULL;
    vocMsg_t *msg = (vocMsg_t *) msg_newCallMsg (query, "executeASCII", 0);
    char      *raw = NULL, *ascii = NULL, *rp = NULL, *buf = NULL;

    /* Send message and read result.
     */
    if (msg_resultStatus ((result = msg_sendMsg (vo->io_chan, msg))) == ERR) {
	if (!vo->quiet)
	    fprintf (stderr, "ERROR: executeASCII failed\n");
	memset (errmsg, 0, SZ_ERRMSG);
	buf = msg_getBuffer (result);
	if (buf)
	    strcpy (errmsg, (buf ? buf : "executeASCII failed"));
    } else
	raw = msg_getBuffer (result);
    
    if (raw) {
	for (rp=raw; *rp && isspace(*rp); rp++)
	    ;				/* skip leading whitespace	*/
	ascii = strdup (rp);
    } else
	ascii = NULL;


    if (msg)    free ((void *)msg); 	/* free the pointers 		*/
    if (raw)    free ((void *)raw);
    if (result) free ((void *)result);

    return (ascii);
}


/**
 *  VOC_EXECUTEVOTABLE -- Execute the query and return a VOTable.
 *
 *  @brief  Execute the query and return a VOTable.
 *  @fn     result = voc_executeVOTable (Query query)
 *
 *  @param  query       DAL query object
 *  @returns            The query result in VOTable format
 */
char *
voc_executeVOTable (Query query)
{
    vocRes_t *result = (vocRes_t *) NULL;
    vocMsg_t *msg = (vocMsg_t *) msg_newCallMsg (query, "executeVOTable", 0);
    char      *raw = NULL, *vot = NULL, *rp = NULL, *buf = NULL;

    /* Send message and read result.
     */
    if (msg_resultStatus ((result = msg_sendMsg (vo->io_chan, msg))) == ERR) {
	if (!vo->quiet)
	    fprintf (stderr, "ERROR: executeVOTable failed\n");
	memset (errmsg, 0, SZ_ERRMSG);
	buf = msg_getBuffer (result);
	if (buf)
	    strcpy (errmsg, (buf ? buf : "executeVOTable failed"));
    } else
	raw = msg_getBuffer (result);

    if (raw) {
	for (rp=raw; *rp && isspace(*rp); rp++)
	    ;				/* skip leading whitespace	*/
	vot = strdup (rp);
    } else
	vot = NULL;

    if (msg)    free ((void *)msg); 	/* free the pointers 		*/
    if (raw)    free ((void *)raw);
    if (result) free ((void *)result);

    return (vot);
}


/**
 *  VOC_EXECUTEQUERYAS -- Execute the query and save requested type to file.
 *
 *  @brief  Execute the query and save requested type to file.
 *  @fn     stat = voc_executeQueryAs (Query query, char *fname, int type)
 *
 *  @param  query       DAL query object
 *  @param  fname       Saved-results file name
 *  @param  type        Output file type
 *  @returns            handle to Registry result object
 */
int
voc_executeQueryAs (Query query, char *fname, int type)
{
    char *res = (char *) NULL;
    int   fd = 0;

    /*  Execute the requested query.
     */
    switch (type) {
    case VOC_CSV:
	res = voc_executeCSV (query);
        break;
    case VOC_TSV:
	res = voc_executeTSV (query);
        break;
    case VOC_ASCII:
	res = voc_executeASCII (query);
        break;
    case VOC_RAW:
    case VOC_VOTABLE:
	res = voc_executeVOTable (query);
        break;
    }

    /*  Make sure we got a result.
     */
    if (!res) {
	fprintf (stderr, "Error: Cannot execute query.\n");
	return (ERR);
    }

    /* Save results to the named output file.
     */
    if ((fd = open (fname, O_CREAT|O_TRUNC|O_WRONLY, DEFFILEMODE)) < 0) {
        fprintf (stderr, "Error: cannot open file '%s'\n", fname);
        return (ERR);
    }
    svr_fileWrite (fd, res, strlen (res));
    close (fd);
    

    if (res)
	free ((void *) res);
    return (OK);
}


/**
 *  VOC_GETRECORDCOUNT --  Get a count of the records returned by the QResponse.
 *
 *  @brief   Get a count of the records returned by the QResponse.
 *  @fn      count = voc_getRecordCount (QResponse qr)
 *
 *  @param   qr         QResponse object handle
 *  @returns            count of records in the response
 */
int
voc_getRecordCount (QResponse qr)
{
    vocRes_t *result = (vocRes_t *) NULL;
    vocMsg_t *msg = (vocMsg_t *) msg_newCallMsg (qr, "getRecordCount", 0);
    int	      count = -1;


    /* Send message and read result.
     */
    if (msg_resultStatus ((result = msg_sendMsg (vo->io_chan, msg))) == ERR) {
	if (!vo->quiet)
	    fprintf (stderr, "ERROR: getRecordCount failed\n");
    } else
	count = msg_getIntResult (result, 0);

    if (msg)    free ((void *)msg); 	/* free the pointers 		*/
    if (result) free ((void *)result);

    return (count);
}


/******************************************************************************
 *  Access by dataset attribute methods.
 *****************************************************************************/

/**
 *  VOC_GETRECORD -- Get the response record by number.
 *
 *  @brief   Get the response record by number.
 *  @fn      rec = voc_getRecord (QResponse qr, int recnum)
 *
 *  @param   qr         QResponse object handle
 *  @returns            QRecord object handle
 */
QRecord
voc_getRecord (QResponse qr, int recnum)
{
    vocRes_t *result = (vocRes_t *) NULL;
    vocMsg_t *msg = (vocMsg_t *) msg_newCallMsg (qr, "getRecord", 0);
    int	      rec = -1;

    msg_addIntParam (msg, recnum);

    /* Send message and read result.
     */
    if (msg_resultStatus ((result = msg_sendMsg (vo->io_chan, msg))) == ERR) {
	if (!vo->quiet)
	    fprintf (stderr, "ERROR: getRecord failed\n");
    } else
	rec = msg_getIntResult (result, 0);

    if (msg)    free ((void *)msg); 	/* free the pointers 		*/
    if (result) free ((void *)result);

    return (rec);
}


/**
 *  VOC_GETATTRCOUNT -- Get attribute record count.
 *
 *  @brief   Get the attribute record count.
 *  @fn      count = voc_getAttrCount (QRecord rec)
 *
 *  @param   rec        QRecord object handle
 *  @returns            count of attributes
 */
int
voc_getAttrCount (QRecord rec)                   
{
    vocRes_t *result = (vocRes_t *) NULL;
    vocMsg_t *msg = (vocMsg_t *) msg_newCallMsg (rec, "getAttrCount", 0);
    int	     count = -1;

    /* Send message and read result.
     */
    if (msg_resultStatus ((result = msg_sendMsg (vo->io_chan, msg))) == ERR) {
	if (!vo->quiet)
	    fprintf (stderr, "ERROR: getAttrCount failed\n");
    } else
	count = msg_getIntResult (result, 0);

    if (msg)    free ((void *)msg); 	/* free the pointers 		*/
    if (result) free ((void *)result);

    return (count);
}


/**
 *  VOC_GETFIELDATTR -- Get the field attribute string.
 *
 *  @brief   Get the field attribute string.
 *  @fn      attr = voc_getFieldAttr (QResponse qr, int index, char *attr)
 *
 *  @param   qr         QResponse object handle
 *  @param   index      response index record number
 *  @param   attr       attribute to retrieve
 *  @returns            value of attribute
 */
char *
voc_getFieldAttr (QResponse qr, int index, char *attr)
{
    vocRes_t *result = (vocRes_t *) NULL;
    vocMsg_t *msg = (vocMsg_t *) msg_newCallMsg (qr, "getFieldAttr", 0);
    char     *id = (char *)NULL;

    msg_addIntParam (msg, index);
    msg_addStringParam (msg, attr);

    /* Send message and read result.
     */
    if (msg_resultStatus ((result = msg_sendMsg (vo->io_chan, msg))) == ERR) {
	if (!vo->quiet)
	    fprintf (stderr, "ERROR: getFieldAttr failed\n");
    } else
	id = msg_getStringResult (result, 0);

    if (msg)    free ((void *)msg); 	/* free the pointers 		*/
    if (result) free ((void *)result);

    return (id);
}


/**
 *  VOC_GETATTRLISST -- Get a list of attributes for the record.
 *
 *  @brief   Get the response record by number.
 *  @fn      str = voc_getAttrList (QRecord rec)
 *
 *  @param   qr         QResponse object handle
 *  @returns            QRecord object handle
 */
char *
voc_getAttrList (QRecord rec)                   
{
    vocRes_t *result = (vocRes_t *) NULL;
    vocMsg_t *msg = (vocMsg_t *) msg_newCallMsg (rec, "getAttrList", 0);
    char     *attr_list = (char *)NULL;


    /* Send message and read result.
     */
    if (msg_resultStatus ((result = msg_sendMsg (vo->io_chan, msg))) == ERR) {
	if (!vo->quiet)
	    fprintf (stderr, "ERROR: getAttrList failed\n");
    } else
	attr_list = msg_getStringResult (result, 0);

    if (msg)    free ((void *)msg); 	/* free the pointers 		*/
    if (result) free ((void *)result);

    return (attr_list);
}


/******************************************************************************
 *  Dataset Attribute Methods:
 *****************************************************************************/

/**
 *  VOC_GETATTRIBUTE -- Get a response record attribute object.
 *
 *  @brief   Get response record attribute object.
 *  @fn      attr = voc_getAttribute (QRecord rec, char *attrname)
 *
 *  @param   rec        QRecord object handle
 *  @param   attrname   attribute name
 *  @returns            QRAttribute object handle
 */
QRAttribute
voc_getAttribute (QRecord rec, char *attrname)
{
    vocRes_t *result = (vocRes_t *) NULL;
    vocMsg_t *msg = (vocMsg_t *) msg_newCallMsg (rec, "getAttribute", 0);
    int	     attr = -1;

    msg_addStringParam (msg, attrname);

    /* Send message and read result.
     */
    if (msg_resultStatus ((result = msg_sendMsg (vo->io_chan, msg))) == ERR) {
	if (!vo->quiet)
	    fprintf (stderr, "ERROR: getAttribute failed\n");
    } else
	attr = msg_getIntResult (result, 0);

    if (msg)    free ((void *)msg); 	/* free the pointers 		*/
    if (result) free ((void *)result);

    return (attr);
}


/******************************************************************************
 ***    NOT YET IMPLEMENTED
 *****************************************************************************/

/**
 *  VOC_SETINTATTR -- Set an integer attribute value in a record.
 *
 *  @brief   Set an integer attribute value in a record.
 *  @fn      voc_setIntAttr (QRecord rec, char *attrname, int ival)
 *
 *  @param   rec        QRecord object handle
 *  @param   attrname   attribute name
 *  @param   ival       integer value to set
 *  @returns            nothing
 */
void 
voc_setIntAttr (QRecord rec, char *attrname, int ival)
{
}


/**
 *  VOC_SETFLOATATTR -- Set a floating-point attribute value in a record.
 *
 *  @brief   Set a floating-point attribute value in a record.
 *  @fn      voc_setFloatAttr (QRecord rec, char *attrname, double dval)
 *
 *  @param   rec        QRecord object handle
 *  @param   attrname   attribute name
 *  @param   dval       double-precision value to set
 *  @returns            nothing
 */
void 
voc_setFloatAttr (QRecord rec, char *attrname, double dval)
{
}


/**
 *  VOC_SETSTRINGATTR -- Set a string attribute value in a record.
 *
 *  @brief   Set a string attribute value in a record.
 *  @fn      voc_setStringAttr (QRecord rec, char *attrname, char *str)
 *
 *  @param   rec        QRecord object handle
 *  @param   attrname   attribute name
 *  @param   str        string value to set
 *  @returns            nothing
 */
void 
voc_setStringAttr (QRecord rec, char *attrname, char *str)
{
}



/*****************************************************************************
 * Utility aliases for the messaging commands below.
 *****************************************************************************/

/**
 *  VOC_GETINTATTR -- Get an integer attribute value from a record.
 *
 *  @brief   Get an integer attribute value from a record.
 *  @fn      ival = voc_getIntAttr (QRecord rec, char *attrname)
 *
 *  @param   rec        QRecord object handle
 *  @param   attrname   attribute name
 *  @returns            integer value
 */
int voc_getIntAttr (QRecord rec, char *attrname) {
    return ( voc_intValue ( voc_getAttribute (rec, attrname)) );
}


/**
 *  VOC_GETFLOATATTR -- Get a floating-point attribute value from a record.
 *
 *  @brief   Get a floating-point attribute value from a record.
 *  @fn      dval = voc_getFloatAttr (QRecord rec, char *attrname)
 *
 *  @param   rec        QRecord object handle
 *  @param   attrname   attribute name
 *  @returns            double-precision value
 */
double voc_getFloatAttr (QRecord rec, char *attrname) {
    return ( voc_floatValue ( voc_getAttribute (rec, attrname)) );
}


/**
 *  VOC_GETSTRINGATTR -- Get a string attribute value from a record.
 *
 *  @brief   Get a string attribute value from a record.
 *  @fn      str = voc_getStringAttr (QRecord rec, char *attrname)
 *
 *  @param   rec        QRecord object handle
 *  @param   attrname   attribute name
 *  @returns            string value
 */
char * voc_getStringAttr (QRecord rec, char *attrname) {
    return ( voc_stringValue ( voc_getAttribute (rec, attrname)) );
}



/**
 *  VOC_INTVALUE -- Get an integer value from an attribute.
 *
 *  @brief   Get an integer value from an attribute.
 *  @fn      ival = voc_intValue (QRAttribute v)
 *
 *  @param   v          QRAttribute object handle
 *  @returns            integer value
 */
int
voc_intValue (QRAttribute v)
{
    vocRes_t *result = (vocRes_t *) NULL;
    vocMsg_t *msg = (vocMsg_t *) NULL;
    int      ival = 0;

    if (v > 0) {
        msg = (vocMsg_t *) msg_newCallMsg (v, "intValue", 0);

        /* Send message and read result.
         */
        if (msg_resultStatus ((result=msg_sendMsg(vo->io_chan,msg))) == ERR) {
	    if (!vo->quiet)
	        fprintf (stderr, "ERROR: intValue() failed\n");
        } else
	    ival = msg_getIntResult (result, 0);

        if (msg)    free ((void *)msg);		/* free the pointers	*/
        if (result) free ((void *)result);
    } else if (!vo->quiet)
	fprintf (stderr, "ERROR: Null attribute to intValue\n");

    return (ival);
}


/**
 *  VOC_FLOATVALUE -- Get a floating-point value from an attribute.
 *
 *  @brief   Get a floating-point value from an attribute.
 *  @fn      dval = voc_floatValue (QRAttribute v)
 *
 *  @param   rec        QRAttribute object handle
 *  @returns            double-precision value
 */
double
voc_floatValue (QRAttribute v)
{
    vocRes_t *result = (vocRes_t *) NULL;
    vocMsg_t *msg = (vocMsg_t *) NULL;
    double   dval = (double) 0.0;

    if (v > 0) {
        msg = (vocMsg_t *) msg_newCallMsg (v, "floatValue", 0);

        /* Send message and read result.
         */
        if (msg_resultStatus ((result=msg_sendMsg(vo->io_chan,msg))) == ERR) {
	    if (!vo->quiet)
	        fprintf (stderr, "ERROR: floatValue() failed\n");
        } else
	    dval = msg_getFloatResult (result, 0);

        if (msg)    free ((void *)msg); 	/* free the pointers	*/
        if (result) free ((void *)result);
    } else if (!vo->quiet)
	fprintf (stderr, "ERROR: Null attribute to floatValue\n");

    return (dval);
}


/**
 *  VOC_STRINGVALUE -- Get a string value from an attribute.
 *
 *  @brief   Get a string value from an attribute.
 *  @fn      str = voc_stringValue (QRAttribute v)
 *
 *  @param   v         QRAttribute object handle
 *  @returns           string value
 */
char *
voc_stringValue (QRAttribute v)
{
    vocRes_t *result = (vocRes_t *) NULL;
    vocMsg_t *msg = (vocMsg_t *) NULL;
    char     *val = NULL, *str = NULL;
    int       len = 0;

    if (v > 0) {
        msg = (vocMsg_t *) msg_newCallMsg (v, "stringValue", 0);

        /* Send message and read result.
         */
        if (msg_resultStatus ((result=msg_sendMsg(vo->io_chan,msg))) == ERR) {
	    if (!vo->quiet)
	        fprintf (stderr, "ERROR: stringValue() failed\n");
        } else {
	    val = msg_getStringResult (result, 0);
	    str = calloc (1, (len = strlen(val)));
	    strncpy (str, val, len);
	}

        if (msg)    free ((void *)msg); 	/* free the pointers 	*/
        if (val)    free ((void *)val);
        if (result) free ((void *)result);
    }

    return (str);
}


/**
 *  VOC_GETDATASET -- Download the AccessReference dataset object to the
 *  named
 *  file.  If fname is NULL, a temp file will be created and the fname
 *  pointer allocated with a string containing the name.
 *
 *  @brief   Download the AccessReference dataset.
 *  @fn      stat = voc_getDataset (QRecord rec, char *acref, char *fname)
 *
 *  @param   rec        QRecord object handle
 *  @param   acref      access reference URL
 *  @param   fname      saved output filename
 *  @returns            ERR or number of bytes in return msg
 */
int 
voc_getDataset (QRecord rec, char *acref, char *fname) 
{
    vocRes_t *result = (vocRes_t *) NULL;
    vocMsg_t *msg = (vocMsg_t *) msg_newCallMsg (rec, "getDataset", 0);
    int       fd, status = OK;
    char      name[SZ_FNAME];


    msg_addStringParam (msg, acref);

    /* Create a temp file if no name is supplied.
     */
    memset (name, 0, SZ_FNAME);
    if (fname == (char *)NULL) {
	strcpy (name, "dataXXXXXX");
	if ((fd = mkstemp (name)) > 0)
	    close (fd);
	fname = calloc (1, strlen (name) + 1);
	strcpy (fname, name);
    }
    strncpy (name, fname, strlen(fname));

    /* Send message and read result.
     */
    if (msg_sendRawMsg (vo->io_chan, msg) != ERR) {
        result = msg_getResultToFile (vo->io_chan, name, TRUE);

        if (msg_resultStatus (result) == ERR) {
	    if (!vo->quiet)
		fprintf (stderr, "ERROR: getDataset failed\n");
            status = ERR;
        }
    }

    if (msg)    free ((void *)msg); 	/* free the pointers 		*/
    if (result) free ((void *)result);

    return (status);
}
