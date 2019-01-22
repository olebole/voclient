/**
 *  VOCLIENT LIBRARY --  Client interface library to the DAL Server
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
 *  cvo_add<type>Param() calls.  No action is taken until an execute of the
 *  query is performed, applications may get back a handle on the result and
 *  interrogate attributes directly, the raw VOTable or a CSV/TSV/ASCII
 *  representation of the result may also be returned.
 *
 *  High-Level Functions:
 *  ---------------------
 *
 *               cvo_initVOClient (config_opts)
 *              cvo_closeVOClient (shutdown_flag)
 *              cvo_abortVOClient (errcode, errmsg)
 * 
 *        string = cvo_coneCaller (url, ra, dec, sr, otype)
 *  status = cvo_coneCallerToFile (url, ra, dec, sr, otype, file)
 *        string = cvo_siapCaller (url, ra, dec, rsize, dsize, fmt, otype)
 *  status = cvo_siapCallerToFile (url, ra, dec, rsize, dsize, fmt, otype, file)
 *        string = cvo_ssapCaller (url, ra, dec, size, band, time, fmt)
 *  status = cvo_ssapCallerToFile (url, ra, dec, size, band, time, fmt, file)
 * 
 *         string = cvo_getRawURL (url, buflen)
 *
 *
 *  Main DAL Interface Procedures:
 *  ------------------------------
 *
 *       dal = cvo_openConnection (svc_url, type)
 *   dal = cvo_openConeConnection (svc_url, [version])	    # Utility aliases
 *   dal = cvo_openSiapConnection (svc_url, [version])
 *   dal = cvo_openSsapConnection (svc_url, [version])
 *            cvo_closeConnection (dal)
 * 
 *    count = cvo_getServiceCount (dal)
 *              cvo_addServiceURL (dal, svc_url)
 *        url = cvo_getServiceURL (dal, index)
 * 
 *           query = cvo_getQuery (dal, type)
 *       query = cvo_getConeQuery (dal, ra, dec, sr)
 *       query = cvo_getSiapQuery (dal, ra, dec, ra_size, dec_size, format)
 *       query = cvo_getSsapQuery (dal, ra, dec, size, band, time, format)
 *
 *         stat = cvo_addIntParam (query, pname, ival)
 *       stat = cvo_addFloatParam (query, pname, dval)
 *      stat = cvo_addStringParam (query, pname, str)
 * 
 *   url_str = cvo_getQueryString (query, type, index)
 *
 *          qr = cvo_executeQuery (query)
 *      qr = cvo_getQueryResponse (query)
 *      stat = cvo_executeQueryAs (query, fname, type)	  (Not Yet Implemented)
 *       csv_tab = cvo_executeCSV (query)
 *       tsv_tab = cvo_executeTSV (query)
 *       ascii = cvo_executeASCII (query)
 *   vot_str = cvo_executeVOTable (query)
 *
 *     count = cvo_getRecordCount (qr)
 *            rec = cvo_getRecord (qr, recnum)
 *         str = cvo_getFieldAttr (qr, fieldnum, attr)
 *
 *        attr = cvo_getAttribute (rec, char *attrname)
 *       count = cvo_getAttrCount (rec)                   
 *     list_str = cvo_getAttrList (rec)                   
 *
 *            ival = cvo_intValue (attr)
 *          dval = cvo_floatValue (attr)
 *          str = cvo_stringValue (attr)
 *
 *                 cvo_setIntAttr (rec, attrname, ival)   (Not Yet Implemented)
 *               cvo_setFloatAttr (rec, attrname, dval)   ( "   "      "      )
 *              cvo_setStringAttr (rec, attrname, str)    ( "   "      "      )
 *
 *          stat = cvo_getDataset (rec, acref, fname) 
 *
 *
 *  Sesame Name Resolver Interface:
 *  -------------------------------
 *
 *          sr = cvo_nameResolver (target)
 *      pos_str = cvo_resolverPos (sr)
 *         radeg = cvo_resolverRA (sr)
 *       decdeg = cvo_resolverDEC (sr)
 * 
 *
 *	Client programs may be written in any language that can interface to
 *  C code.  Sample programs using the interface are provided as is a SWIG
 *  interface definition file.
 *
 *
 *  @file       cvoDAL.c
 *  @author     Michael Fitzpatrick, NOAO
 *  @version    December 2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#define __USE_GNU
#define _GNU_SOURCE
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#define _VOCLIENT_LIB_
#include "VOClient.h"
#include "vocServer.h"
#include "dalclient.h"


/**
 *  Utility structures to map between the VOClient and libdalclient
 *  interpretation of the response.  VOClient refers to table columns as 
 *  'attributes' since the simple table data model is a collection of
 *  keyw=value pairs in a row.  In the dalclient, an 'attribute' is an
 *  XML element attr (e.g. the 'utype' in a <FIELD>) and columns/FIELDs
 *  are accessed by column index.  To resolve this we maintain a local
 *  structure so the application-facing handles behave as before with the
 *  Java daemon, but we retain enough information to call the dalcient
 *  interface as required.
 */
typedef struct {
    handle_t  dal_qr;			/* QResponse handle		*/
    handle_t  dal_rec;			/* QRecord handle		*/

    int	      recnum;			/* record (i.e. row) number	*/
    int	      rec_count;		/* record (i.e. col) count	*/
} dalRec, *dalRecP;


typedef struct {
    char      fieldName[SZ_FNAME];	/* FIELD column name		*/
    int       fieldIndex;		/* column number		*/

    dalRecP   recP;
    handle_t  dal_rec;
} dalAttr, *dalAttrP;


dalRec  *dal_rec = (dalRec *) NULL;
dalAttr *dal_attr = (dalAttr *) NULL;

extern VOClient *vo;			/* Interface runtime struct	*/

extern handle_t  svr_newHandle (int context, void *p);
extern void     *svr_H2P (handle_t h);
extern handle_t *svr_P2H (void *p);


#define	CVO_DEBUG		0

#define SZ_ERRMSG		256
static char errmsg[SZ_ERRMSG];




/******************************************************************************
***********         	    High Level Functions       		     **********
******************************************************************************/


/**
 *  CVO_INITVOCLIENT -- Initialize the VOClient for standalone use.
 *
 *  @brief   Initialize the VOClient for standalone use.
 *  @fn      vo = cvo_initVOClient (char *opts)
 *
 *  @param   opts	Initialization option string
 *  @returns            VO object handle 
 *
 *  When using the CVO interface we need to initialize the VOClient lib
 *  but we do not want to spawn the daemon.  In this case we simply add an
 *  option to the string an init normally (only the application does the
 *  name remapping).
 */
int
cvo_initVOClient (char *opts)
{
    char optbuf[1024];

    memset (optbuf, 0, 1024);
    if (opts && opts[0]) {
	strcpy (optbuf, opts);
	strcat (optbuf, ",mini-d");
    } else {
	strcpy (optbuf, "mini-d");
    }

    return ( voc_initVOClient (optbuf) );
}



/******************************************************************************
***********         	    High Level Functions       		     **********
******************************************************************************/

/**
 *  CVO_CONECALLER -- Hi-level Cone Service Caller method.
 *
 *  @brief   Hi-level Cone Service Caller method.
 *  @fn      res = cvo_coneCaller (char *url, double ra, double dec, 
 *			double sr, int otype)
 *
 *  @param   url	Base service URL
 *  @param   ra		RA position (ICRS degrees)
 *  @param   dec	Dec position (ICRS degrees)
 *  @param   sr		Search radius (degrees)
 *  @param   otype	Output result type (VOTable, CSV, etc)
 *  @returns            ERR or number of bytes in return msg
 *
 *  Simple all-in-one interface to call a Cone service and return the results
 *  as a text string of either the raw VOTable or a CSV/TSV/Ascii table.
 * 
 *  Result string is allocated here and must be freed by the caller.
 */
char *
cvo_coneCaller (char *url, double ra, double dec, double sr, int otype)
{
    char *res  = NULL, *retval = NULL;;

    DAL       cone;                             /* DAL Connection        */
    Query     query;                            /* query handle          */


    /*  Initialize the VOClient code.  Error messages are printed by the
     *  interface so we just quit if there is a problem.
     */
    if (!vo && voc_initVOClient (NULL) == ERR)
        return ((char *)NULL);

    /*  Get a new connection to the named service.
     */
    cone = dal_openConeConnection (url, "1"); 	   /* open a connection	*/

    query = dal_getConeQuery (cone, ra, dec, sr); /* form a query 	*/

    if (otype == VOC_CSV)
        res = dal_executeCSV (query);
    else if (otype == VOC_TSV)
        res = dal_executeTSV (query);
    else if (otype == VOC_ASCII)
        res = dal_executeASCII (query);
    else if (otype == VOC_VOTABLE)
        res = dal_executeVOTable (query);

    retval = (char *) strdup (res);		/* copy result string         */

    dal_closeQuery (query);                     /* close the query connection */
    dal_closeConnection (cone);                 /* close the cone connection  */
    voc_closeVOClient (0);                      /* clean up and shutdown      */

    /*  The DALClient library manages it's own strings, but we expect the
     *  caller to free our result so copy the string here.
     */
    return ((char *) retval );
}


/**
 *  CVO_CONECALLERTOFILE -- Hi-level Cone Service Caller method.
 *
 *  @brief   Hi-level Cone Service Caller method.
 *  @fn      res = cvo_coneCallerToFile (char *url, double ra, double dec, 
 *			double sr, int otype, char *file)
 *
 *  @param   url	Base service URL
 *  @param   ra		RA position (ICRS degrees)
 *  @param   dec	Dec position (ICRS degrees)
 *  @param   sr		Search radius (degrees)
 *  @param   otype	Output result type (VOTable, CSV, etc)
 *  @param   file       Output file name
 *  @returns            ERR or number of bytes in return msg
 *
 *  Simple all-in-one interface to call a Cone service and return the results
 *  as a text string of either the raw VOTable or a CSV/TSV/Ascii table.
 * 
 *  Result string is allocated here and must be freed by the caller.
 */
int
cvo_coneCallerToFile (char *url, double ra, double dec, double sr, int otype, 
	char *file)
{
    if (url) {
        char *res = (char *) NULL;
        int   fd = 0;

        if ((fd = open (file, O_CREAT|O_TRUNC|O_WRONLY)) < 0) {
            fprintf (stderr, "Error: cannot open file '%s'\n", file);
            return (ERR);
        }

        if ( (res = cvo_coneCaller (url, ra, dec, sr, otype)) ) {
            svr_fileWrite (fd, res, strlen (res));
            free ((void *) res);
        }
        close (fd);
    }

    return (OK);
}


/**
 *  CVO_SIAPCALLER -- Hi-level SIA Service Caller method.
 *
 *  @brief   Hi-level SIA Service Caller method.
 *  @fn      res = cvo_siapCaller (char *url, double ra, double dec, 
 *			double sr, int otype)
 *
 *  @param   url	Base service URL
 *  @param   ra		RA position (ICRS degrees)
 *  @param   dec	Dec position (ICRS degrees)
 *  @param   sr		Search radius (degrees)
 *  @param   otype	Output result type (VOTable, CSV, etc)
 *  @returns            ERR or number of bytes in return msg
 *
 *  Simple all-in-one interface to call an SIA service and return the results
 *  as a text string of either the raw VOTable or a CSV/TSV/Ascii table.
 * 
 *  Result string is allocated here and must be freed by the caller.
 */
char *
cvo_siapCaller (char *url, double ra, double dec, double rsize, double dsize, 
	char *fmt, int otype)
{
    char *res  = NULL, *retval = NULL;

    DAL       siap;                             /* DAL Connection        */
    Query     query;                            /* query handle          */


    /*  Initialize the VOClient code.  Error messages are printed by the
     *  interface so we just quit if there is a problem.
     */
    if (!vo && voc_initVOClient (NULL) == ERR)
        return ((char *) NULL);

    /*  Get a new connection to the named service.
     */
    siap = dal_openSiapConnection (url, "1");  	/* open a connection */

    /*  Form a query.  Here we'll use the one search size we're given for
     *  both the RA,DEC sizes, and specify a null format.
     */
    query = dal_getSiapQuery (siap, ra, dec, rsize, dsize, fmt);

    if (otype == VOC_CSV)
        res = dal_executeCSV (query);
    else if (otype == VOC_TSV)
        res = dal_executeTSV (query);
    else if (otype == VOC_ASCII)
        res = dal_executeASCII (query);
    else if (otype == VOC_VOTABLE)
        res = dal_executeVOTable (query);

    retval = (char *) strdup (res);		/* copy result string         */

    dal_closeQuery (query);                     /* close the query connection */
    dal_closeConnection (siap);                 /* close the siap connection  */
    voc_closeVOClient (0);                      /* clean up and shutdown      */

    return ((char *) res);
}


/**
 *  CVO_SIAPCALLERTOFILE -- Hi-level SIA Service Caller method.
 *
 *  @brief   Hi-level SIA Service Caller method.
 *  @fn      res = cvo_siapCallerToFile (char *url, double ra, double dec, 
 *			double sr, int otype, char *file)
 *
 *  @param   url	Base service URL
 *  @param   ra		RA position (ICRS degrees)
 *  @param   dec	Dec position (ICRS degrees)
 *  @param   sr		Search radius (degrees)
 *  @param   otype	Output result type (VOTable, CSV, etc)
 *  @param   file       Output file name
 *  @returns            ERR or number of bytes in return msg
 *
 *  Simple all-in-one interface to call a SIA service and return the results
 *  as a text string of either the raw VOTable or a CSV/TSV/Ascii table.
 * 
 *  Result string is allocated here and must be freed by the caller.
 */
int
cvo_siapCallerToFile (char *url, double ra, double dec, double rsize, 
	double dsize, char *fmt, int otype, char *file)
{
    if (url) {
        char *res = (char *) NULL;
        int   fd = 0;

        if ((fd = open (file, O_CREAT|O_TRUNC|O_WRONLY)) < 0) {
            fprintf (stderr, "Error: cannot open file '%s'\n", file);
            return (ERR);
        }

        if ( (res = cvo_siapCaller (url, ra, dec, rsize, dsize, fmt, otype)) ) {
            svr_fileWrite (fd, res, strlen (res));
            free ((void *) res);
        }
        close (fd);
    }

    return (OK);
}


/**
 *  CVO_SSAPCALLER -- Hi-level SSA Service Caller method.
 *
 *  @brief   Hi-level SSA Service Caller method.
 *  @fn      res = cvo_ssapCaller (char *url, double ra, double dec, 
 *			double sr, int otype)
 *
 *  @param   url	Base service URL
 *  @param   ra		RA position (ICRS degrees)
 *  @param   dec	Dec position (ICRS degrees)
 *  @param   sr		Search radius (degrees)
 *  @param   otype	Output result type (VOTable, CSV, etc)
 *  @returns            ERR or number of bytes in return msg
 *
 *  Simple all-in-one interface to call an SSA service and return the results
 *  as a text string of either the raw VOTable or a CSV/TSV/Ascii table.
 * 
 *  Result string is allocated here and must be freed by the caller.
 */
char *
cvo_ssapCaller (char *url, double ra, double dec, double size, 
	char *band, char *time, char *fmt, int otype)
{
    char *res  = NULL, *retval = NULL;

    DAL       ssap;                             /* DAL Connection        */
    Query     query;                            /* query handle          */


    /*  Initialize the VOClient code.  Error messages are printed by the
     *  interface so we just quit if there is a problem.
     */
    if (!vo && voc_initVOClient (NULL) == ERR)
        return ((char *) NULL);

    /*  Get a new connection to the named service.
     */
    ssap = dal_openSsapConnection (url, "1");   /* open a connection */

    /*  Form a query.  Here we'll use the one search size we're given for
     *  both the RA,DEC sizes, and specify a null format.
     */
    query = dal_getSsapQuery (ssap, ra, dec, size, band, time, fmt);

    if (otype == VOC_CSV)
        res = dal_executeCSV (query);
    else if (otype == VOC_TSV)
        res = dal_executeTSV (query);
    else if (otype == VOC_ASCII)
        res = dal_executeASCII (query);
    else if (otype == VOC_VOTABLE)
        res = dal_executeVOTable (query);

    retval = (char *) strdup (res);		/* copy result string         */

    dal_closeQuery (query);                     /* close the query connection */
    dal_closeConnection (ssap);                 /* close the ssap connection  */
    voc_closeVOClient (0);                      /* clean up and shutdown      */

    return ((char *) res);
}


/**
 *  CVO_SSAPCALLERTOFILE -- Hi-level SSA Service Caller method.
 *
 *  @brief   Hi-level SSA Service Caller method.
 *  @fn      res = cvo_ssapCallerToFile (char *url, double ra, double dec, 
 *			double sr, int otype, char *file)
 *
 *  @param   url	Base service URL
 *  @param   ra		RA position (ICRS degrees)
 *  @param   dec	Dec position (ICRS degrees)
 *  @param   sr		Search radius (degrees)
 *  @param   otype	Output result type (VOTable, CSV, etc)
 *  @param   file       Output file name
 *  @returns            ERR or number of bytes in return msg
 *
 *  Simple all-in-one interface to call a SSA service and return the results
 *  as a text string of either the raw VOTable or a CSV/TSV/Ascii table.
 * 
 *  Result string is allocated here and must be freed by the caller.
 */
int
cvo_ssapCallerToFile (char *url, double ra, double dec, double size, 
	char *band, char *time, char *fmt, int otype, char *file)
{
    if (url) {
        char *res = (char *) NULL;
        int   fd = 0;

        if ((fd = open (file, O_CREAT|O_TRUNC|O_WRONLY)) < 0) {
            fprintf (stderr, "Error: cannot open file '%s'\n", file);
            return (ERR);
        }

        if ( (res = cvo_ssapCaller (url, ra, dec, size, band, time,
		fmt, otype)) ) {
            	    svr_fileWrite (fd, res, strlen (res));
            	    free ((void *) res);
        }
        close (fd);
    }

    return (OK);
}


/**
 *  CVO_GETRAWURL -- Get a raw URL to a returned string.  
 *
 *  @brief   Get a raw URL to a returned string.
 *  @fn      raw = cvo_getRawURL (char *url, int *nbytes)
 *
 *  @param   url       	URL to retrieve
 *  @param   nbytes   	number of bytes read
 *  @returns            Result string from URL
 *
 *  Note this is only for text files because of the assumed use of EOF 
 *  that may actually be a NULL byte in a binary data stream.
 */
char *
cvo_getRawURL (char *url, int *nbytes)
{
    char      *data = svr_getURL (url);

    *nbytes = (data ? strlen (data) : 0);
    return ((char *) data);
}


/**
 *  CVO_OPENCONNECTION -- Open a new DAL context connection.
 * 
 *  @brief   Open a new DAL context connection.
 *  @fn      dal = cvo_openConnection (char *svc_url, int type, ...)
 *
 *  @param   svc_url 	base service URL
 *  @param   type       DAL connection type
 *  @returns            Handle to new DAL connection object
 */
DAL
cvo_openConnection (char *service_url, int type, ...)
{
    DAL	      dal = (int) VOC_NULL;
    char     *version = "1", *service = "";
    va_list   argp;


    /*  Make sure we've been initialized properly first.
     */
    if (vo == (VOClient *) NULL) {
	if (voc_initVOClient (NULL) == ERR) {
	    if (!vo->quiet)
	        fprintf (stderr, "ERROR: Can't initialize VO Client....\n");
	    exit (1);
	} else if (CVO_DEBUG)
	    printf ("Warning: Initializing VO Client....\n");
    }

    switch (type) {
    case DAL_CONN:  	service = "DAL";  break;	/* FIXME   */
    case CONE_CONN: 	service = "scs";  break;
    case SIAP_CONN: 	service = "sia";  break;
    case SSAP_CONN: 	service = "ssa";  break;
    default:
	if (!vo->quiet)
	    fprintf (stderr, "ERROR: Invalid newConnection request type=%d\n",
		type);
	return ((DAL) VOC_NULL);
    }

    /*  Get the optional version parameter.
     */
    va_start (argp, type);                      /* get a single string    */
    version = va_arg ((argp), char *);
    va_end (argp);


    dal = dal_openConnection (service_url, service, (version ? version : "1"));
    

    /* If we were given a service_url add it as a param to the DAL object.
     */
    if (service_url)
	cvo_addServiceURL (dal, service_url);

    /* Return the object handle.
     */
    return (dal);
}


/******************************************************************************
 *  Utility aliases for code readability.  We call the generic openConnection()
 *  procedure so that we can use the VOClient initialization.
 *****************************************************************************/

/**
 *  CVO_OPENCONECONNECTION -- Open a new DAL Cone connection.
 * 
 *  @brief   Open a new DAL Cone connection.
 *  @fn      dal = cvo_openConeConnection (char *svc_url, ...)
 *
 *  @param   svc_url 	base service URL
 *  @param   type       DAL connection type
 *  @returns            Handle to new DAL connection object
 */
DAL cvo_openConeConnection (char *service_url, ...)
{
    DAL   dal = (DAL) NULL;
    char *version = "1";
    va_list   argp;

    /*  Get the optional version parameter.
     */
    va_start (argp, service_url);
    version = va_arg ((argp), char *);
    va_end (argp);

    return ( (dal = cvo_openConnection (service_url, CONE_CONN, version)) );
}


/**
 *  CVO_OPENSIAPCONNECTION -- Open a new DAL SIAP connection.
 * 
 *  @brief   Open a new DAL SIAP connection.
 *  @fn      dal = cvo_openSiapConnection (char *svc_url, ...)
 *
 *  @param   svc_url 	base service URL
 *  @param   type       DAL connection type
 *  @returns            Handle to new DAL connection object
 */
DAL cvo_openSiapConnection (char *service_url, ...)
{
    DAL   dal = (DAL) NULL;
    char *version = "1";
    va_list   argp;

    /*  Get the optional version parameter.
     */
    va_start (argp, service_url);
    version = va_arg ((argp), char *);
    va_end (argp);

    return ( (dal = cvo_openConnection (service_url, SIAP_CONN, version)) );
}


/**
 *  CVO_OPENSSAPCONNECTION -- Open a new DAL SSAP connection.
 * 
 *  @brief   Open a new DAL SSAP connection.
 *  @fn      dal = cvo_openSsapConnection (char *svc_url, ...)
 *
 *  @param   svc_url 	base service URL
 *  @param   type       DAL connection type
 *  @returns            Handle to new DAL connection object
 */
DAL cvo_openSsapConnection (char *service_url, ...)
{
    DAL   dal = (DAL) NULL;
    char *version = "1";
    va_list   argp;

    /*  Get the optional version parameter.
     */
    va_start (argp, service_url);
    version = va_arg ((argp), char *);
    va_end (argp);

    return ( (dal = cvo_openConnection (service_url, SSAP_CONN, version)) );
}


/**
 *  CVO_CLOSECONNETION -- Close the requested connection.
 * 
 *  @brief   Close the requested connection.
 *  @fn      cvo_closeConnection (DAL dal)
 *
 *  @param   dal       	DAL connection object handle
 *  @returns            nothing
 */
void
cvo_closeConnection (DAL dal)
{
    dal_closeConnection (dal);
}


/**
 *  CVO_GETSERVICECOUNT -- Get a count of the number of services associated 
 *  with the given DAL connection context.
 * 
 *  @brief   Get service count associated with the DAL context.
 *  @fn      count = cvo_getServiceCount (DAL dal)
 *
 *  @param   dal       	DAL connection object handle
 *  @returns            Number of services associated with the DAL handle
 */
int
cvo_getServiceCount (DAL dal)
{
    int       count = 1; 		/**  NOT YET IMPLEMENTED  **/

    return (count);
}


/**
 *  CVO_ADDSERVICEURL --  Add a service URL to the specified connection.
 * 
 *  @brief   Add a service URL to the specified connection.
 *  @fn      cvo_addServiceURL (DAL dal, char *service_url)
 *
 *  @param   dal       	DAL connection object handle
 *  @param   svc_url    base service url
 *  @returns            nothing
 */
void
cvo_addServiceURL (DAL dal, char *service_url)
{
    dal_setBaseUrl (dal, service_url);
}


/**
 *  CVO_GETSERVICEURL --  Get the requested service URL for the connection.
 * 
 *  @brief   Get the requested service URL for the connection.
 *  @fn      str = cvo_getServiceURL (DAL dal, int index)
 *
 *  @param   dal       	DAL connection object handle
 *  @param   index      service index (0-based)
 *  @returns            requested service URL
 */
char *
cvo_getServiceURL (DAL dal, int index)
{
    char *service_url = dal_getBaseUrl (dal);
    return (service_url);
}


/**
 *  CVO_GETQUERY --  Get a generic Query context from the server.
 * 
 *  @brief   Method description.
 *  @fn      stat = sm_method (int objid, msgParam *pars, int npars)
 *
 *  @param   dal       	DAL connection object handle
 *  @param   type       query type (not used)
 *  @returns            Query object handle
 */
Query
cvo_getQuery (DAL dal, int type)
{
    return ( dal_getQuery (dal) ); 	/* the 'type' is ignored   */
}


/**
 *  CVO_GETCONEQUERY --  Get a query for a Cone service.  
 *
 *  We take the typical Cone arguments in the interface routine but break 
 *  it down into a series of messages to get a general Query object and 
 *  then add parameters.
 * 
 *  @brief   Get a query for a Cone service.
 *  @fn      query = cvo_getConeQuery (DAL dal, double ra, double dec, 
 *			double sr)
 *
 *  @param   dal       	DAL connection object handle
 *  @param   ra         RA of position (ICRS degrees)
 *  @param   dec        Dec of position (ICRS degrees)
 *  @param   sr         Search radius (degrees)
 *  @returns            ERR or number of bytes in return msg
 */
Query
cvo_getConeQuery (DAL dal, double ra, double dec, double sr)
{
    Query query = dal_getConeQuery (dal, ra, dec, sr);

    /*  Add a RUNID string to the query for logging.
     */
    if (vo->use_runid)
	cvo_addStringParam (query, "RUNID", vo->runid);

    return ( query );
}


/**
 *  CVO_GETSIAPQUERY --  Get a query for a SIAP service.  
 *
 *  We take the typical SIA arguments in the interface routine but break 
 *  it down into a series of messages to get a general Query object and 
 *  then add parameters.
 * 
 *  @brief   Get a query for a SIAP service.
 *  @fn      query = cvo_getSiapQuery (DAL dal, double ra, double dec, 
 *			double ra_size, dec_size, char *format)
 *
 *  @param   dal       	DAL connection object handle
 *  @param   ra         RA of position (ICRS degrees)
 *  @param   dec        Dec of position (ICRS degrees)
 *  @param   ra_size    RA search size (degrees)
 *  @param   dec_size   Dec search size (degrees)
 *  @param   format     Requested format
 *  @returns            ERR or number of bytes in return msg
 */
Query
cvo_getSiapQuery (DAL dal, double ra, double dec, double ra_size, 
	double dec_size, char *format)
{
    Query query = dal_getSiapQuery (dal, ra, dec, ra_size, dec_size, format);
    
    /*  Add a RUNID string to the query for logging.
     */
    if (vo->use_runid)
	cvo_addStringParam (query, "RUNID", vo->runid);

    return ( query );
}


/**
 *  CVO_GETSSAPQUERY --  Get a query for a SSAP service.
 *
 *  We take the typical SSA arguments in the interface routine but break
 *  it down into a series of messages to get a general Query object and
 *  then add parameters.
 *
 *  @brief   Get a query for a SSAP service.
 *  @fn      query = cvo_getSsapQuery (DAL dal, double ra, double dec,
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
cvo_getSsapQuery (DAL dal, double ra, double dec, double size, char *band, 
    char *tim, char *format)
{
    Query query = dal_getSsapQuery (dal, ra, dec, size, band, tim, format);
    
    
    /*  Add a RUNID string to the query for logging.
     */
    if (vo->use_runid)
	cvo_addStringParam (query, "RUNID", vo->runid);

    return (query);
}


/**
 *  CVO_ADDINTPARAM --  Add an integer parameter to a Query string.
 * 
 *  @brief   Add an integer parameter to a Query string.
 *  @fn      stat = cvo_addIntParam (Query query, char *name, int ival)
 *
 *  @param   query       Query object handle
 *  @param   name        parameter name
 *  @param   ival        integer value
 *  @returns             status (OK or ERR)
 */
int
cvo_addIntParam (Query query, char *name, int ival)
{
    return ( dal_addIntParam (query, name, (long) ival) );
}
              

/**
 *  CVO_ADDFLOATPARAM --  Add a floating-point parameter to a Query string.
 * 
 *  @brief   Add a floating-point parameter to a Query string.
 *  @fn      stat = cvo_addFloatParam (Query query, char *name, double dval)
 *
 *  @param   query       Query object handle
 *  @param   name        parameter name
 *  @param   dval        double-precision value
 *  @returns             status (OK or ERR)
 */
int
cvo_addFloatParam (Query query, char *name, double dval)
{
    return ( dal_addFloatParam (query, name, dval) );
}
             

/**
 *  CVO_ADDSTRINGPARAM --  Add a String parameter to a Query string.
 * 
 *  @brief   Add a String parameter to a Query string.
 *  @fn      stat = cvo_addStringParam (Query query, char *name, char *str)
 *
 *  @param   query       Query object handle
 *  @param   name        parameter name
 *  @param   str         string parameter
 *  @returns             status (OK or ERR)
 */
int
cvo_addStringParam (Query query, char *name, char *str)
{
    return ( dal_addStringParam (query, name, str) );
}


/**
 *  CVO_GETQUERYSTRING -- Get the complete query string that will be executed.
 * 
 *  @brief   Get the complete query string that will be executed.
 *  @fn      str = cvo_getQueryString (Query query, int type, int index)
 *
 *  @param   query       Query object handle
 *  @param   type        query type (not used)
 *  @param   index       service index (not used)
 *  @returns             query URL to be executed
 */
char *
cvo_getQueryString (Query query, int type, int index)
{
    return ( dal_getQueryURL (query) );
}


/**
 *  CVO_EXECUTEQUERY --  Execute the specified query in the DAL server.
 *  the QResponse object handle.
 * 
 *  @brief   Method description.
 *  @fn      qr = cvo_executeQuery (Query query)
 *
 *  @param   query       Query object handle
 *  @returns             QResponse object handle
 */
QResponse
cvo_executeQuery (Query query)
{
    return ( dal_executeQuery (query) );
}


/**
 *  CVO_GETQUERYRESPONSE --  Utility to get QResponse handle from Query.
 *
 *  We use this when the query itself was executed by a routine that doesn't
 *  directly return it  (e.g. cvo_executeCSV()).
 * 
 *  @brief   Utility to get QResponse handle from Query.
 *  @fn      qr = cvo_getQueryResponse (Query query)
 *
 *  @param   query       Query object handle
 *  @returns             QResponse object handle
 */
QResponse
cvo_getQueryResponse (Query query)
{
    return ( dal_getQueryResponse (query) );
}


/**
 *  CVO_GETERRMSG -- Get the last error message.
 *
 *  @brief  Get the last error message.
 *  @fn     msg = cvo_getErrMsg (void)
 *
 *  @returns            The last error message.
 */
char *
cvo_getErrMsg ()
{
    return (errmsg);			/* Not Yet Implemented	*/
}
             

/**
 *  CVO_EXECUTECSV -- Execute the query and return a CSV table.
 *
 *  @brief  Execute the query and return a CSV table.
 *  @fn     result = cvo_executeCSV (Query query)
 *
 *  @param  query       DAL query object
 *  @returns            The query result in CSV format
 */
char *
cvo_executeCSV (Query query)
{
    return ( dal_executeCSV (query) );
}


/**
 *  CVO_EXECUTETSV -- Execute the query and return a TSV table.
 *
 *  @brief  Execute the query and return a TSV table.
 *  @fn     result = cvo_executeTSV (Query query)
 *
 *  @param  query       DAL query object
 *  @returns            The query result in TSV format
 */
char *
cvo_executeTSV (Query query)
{
    return ( dal_executeTSV (query) );
}


/**
 *  CVO_EXECUTEASCII -- Execute the query and return an ASCII table.
 *
 *  @brief  Execute the query and return an ASCII table.
 *  @fn     result = cvo_executeASCII (Query query)
 *
 *  @param  query       DAL query object
 *  @returns            The query result in ASCII format
 */
char *
cvo_executeASCII (Query query)
{
    return ( dal_executeASCII (query) );
}


/**
 *  CVO_EXECUTEVOTABLE -- Execute the query and return a VOTable.
 *
 *  @brief  Execute the query and return a VOTable.
 *  @fn     result = cvo_executeVOTable (Query query)
 *
 *  @param  query       DAL query object
 *  @returns            The query result in VOTable format
 */
char *
cvo_executeVOTable (Query query)
{
    return ( dal_executeVOTable (query) );
}


/**
 *  CVO_EXECUTEQUERYAS -- Execute the query and save requested type to file.
 *
 *  @brief  Execute the query and save requested type to file.
 *  @fn     stat = cvo_executeQueryAs (Query query, char *fname, int type)
 *
 *  @param  query       DAL query object
 *  @param  fname       Saved-results file name
 *  @param  type        Output file type
 *  @returns            handle to Registry result object
 */
int
cvo_executeQueryAs (Query query, char *fname, int type)
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
 *  CVO_GETRECORDCOUNT --  Get a count of the records returned by the QResponse.
 * 
 *  @brief   Get a count of the records returned by the QResponse.
 *  @fn      count = cvo_getRecordCount (QResponse qr)
 *
 *  @param   qr       	QResponse object handle
 *  @returns            count of records in the response
 */
int
cvo_getRecordCount (QResponse qr)
{
    return ( dal_getRecordCount (qr) );
}


/******************************************************************************
 * Access by dataset attribute methods.
 *****************************************************************************/

/**
 *  CVO_GETRECORD -- Get the response record by number.
 * 
 *  @brief   Get the response record by number.
 *  @fn      rec = cvo_getRecord (QResponse qr, int recnum)
 *
 *  @param   qr         QResponse object handle
 *  @returns            QRecord object handle
 */
QRecord
cvo_getRecord (QResponse qr, int recnum)
{
    QRecord rec = dal_getRecord (qr, recnum);
    handle_t handle;


    if (! dal_rec)
	dal_rec = (dalRec *) calloc (1, sizeof (dalRec));

    dal_rec->dal_qr = qr;
    dal_rec->dal_rec = rec;			// dalClient QRecord
    dal_rec->recnum = recnum;
    dal_rec->rec_count = dal_getRecordCount (qr);

    if (CVO_DEBUG)
	fprintf (stderr, "getRecord: qr=%d  rec=%d  recnum=%d  count=%d\n",
	    qr, rec, recnum, dal_rec->rec_count);

    handle = svr_newHandle (vo->handle_context, (void *) dal_rec);

    return ( handle );
}


/**
 *  CVO_GETATTRCOUNT -- Get attribute (i.e. column) record count.
 * 
 *  @brief   Get the attribute i.e. column) count.
 *  @fn      count = cvo_getAttrCount (QRecord rec)                   
 *
 *  @param   rec        QRecord object handle
 *  @returns            count of attributes
 */
int
cvo_getAttrCount (QRecord rec)                   
{
    dalRec *r = svr_H2P (rec);

    return ( dal_getFieldCount (r->dal_qr) );
}


/**
 *  CVO_GETFIELDATTR -- Get the field attribute string.
 * 
 *  @brief   Get the field attribute string.
 *  @fn      attr = cvo_getFieldAttr (QResponse qr, int index, char *attr)
 *
 *  @param   qr         QResponse object handle
 *  @param   index      response index record number
 *  @param   attr       attribute to retrieve
 *  @returns            value of attribute
 */
char *
cvo_getFieldAttr (QResponse qr, int index, char *attr)
{
    return (NULL);			// Not Yet Implemented
}


/**
 *  CVO_GETATTRLIST -- Get a list of attributes for the record.
 * 
 *  @brief   Get the response record by number.
 *  @fn      str = cvo_getAttrList (QRecord rec)                   
 *
 *  @param   qr         QResponse object handle
 *  @returns            QRecord object handle
 */
char *
cvo_getAttrList (QRecord rec)                   
{
    dalRec *r = svr_H2P (rec);
    register int i, nrec =  cvo_getAttrCount (rec);
    char  buf[8192], col[32], *str = NULL;


    memset (buf, 0, 8192);
    for (i=0; i < nrec; i++) {
	if (!(str = dal_getFieldAttr (r->dal_qr, i, DAL_UCD))) {
	    if (!(str = dal_getFieldAttr (r->dal_qr, i, DAL_NAME))) {
		if (!(str = dal_getFieldAttr (r->dal_qr, i, DAL_ID))) {
    		    memset (col, 0, 32);
		    sprintf (col, "col%03d", i);
		    str = col;
		}
	    }
	}
	if (i == 0)
	    strncpy (buf, str, strlen (str));
	else
	    strncat (buf, str, strlen (str));
	strcat (buf, (i < (nrec-1) ? " " : "\0"));
    }

    if (CVO_DEBUG)
	fprintf (stderr, "getAttrList[%d]: '%s'\n", nrec, buf);

    return ( strdup (buf) );
}


/******************************************************************************
 *  Dataset Attribute Methods:
 *****************************************************************************/

/**
 *  CVO_GETATTRIBUTE -- Get a response record attribute object.
 *
 *  @brief   Get response record attribute object.
 *  @fn      attr = cvo_getAttribute (QRecord rec, char *attrname)
 *
 *  @param   rec        QRecord object handle
 *  @param   attrname   attribute name
 *  @returns            QRAttribute object handle
 */
QRAttribute
cvo_getAttribute (QRecord rec, char *attrname)
{
    dalRec *r = svr_H2P (rec);
    int    index = 0;
    char  *ip = NULL, *aname = NULL, *colname = NULL;
    char  *attrList = cvo_getAttrList (rec);
    handle_t  handle;


    ip = attrList;			/* find attrname as a substring */
    for (aname=attrList; *ip; ip++) {
	if (!(*ip) || *ip == ' ') {
	    *ip = '\0';
	    if (strcasestr (aname, attrname)) {
	        colname = dal_getFieldAttr (r->dal_qr, index, DAL_NAME);
		break;
	    }
	    aname = (++ip);
	    index++;
	}
    }
    free ((void *) attrList);


    if (! dal_attr)
	dal_attr = (dalAttr *) calloc (1, sizeof (dalAttr));

    dal_attr->recP = r;
    dal_attr->dal_rec = r->dal_rec;
    dal_attr->fieldIndex = index;

    memset (dal_attr->fieldName, 0, SZ_FNAME);	// ensure empty string
    strcpy (dal_attr->fieldName, colname);

    return ( (handle = svr_newHandle (vo->handle_context, (void *) dal_attr)) );
}


/*****************************************************************************
 * Utility aliases for the messaging commands.
 *****************************************************************************/

/**
 *  CVO_STRINGVALUE -- Get a string value from an attribute.
 *
 *  @brief   Get a string value from an attribute.
 *  @fn      str = cvo_stringValue (QRAttribute v)
 *
 *  @param   v         QRAttribute object handle
 *  @returns           string value
 */
char *
cvo_stringValue (QRAttribute v)
{
    char     *str = NULL;
    dalAttr  *attr = svr_H2P (v);

    str = dal_getStringField (attr->dal_rec, attr->fieldIndex);

    if (CVO_DEBUG)
	fprintf (stderr, "cvo_strValue: name='%s' indx=%d  rec=%ld str='%s'\n",
	    attr->fieldName, attr->fieldIndex, (long)attr->dal_rec, str);

    return (str);
}


/**
 *  CVO_INTVALUE -- Get an integer value from an attribute.
 *
 *  @brief   Get an integer value from an attribute.
 *  @fn      ival = cvo_intValue (QRAttribute v)
 *
 *  @param   v          QRAttribute object handle
 *  @returns            integer value
 */
int
cvo_intValue (QRAttribute v)
{
    int      ival = 0;
    dalAttr  *attr = svr_H2P (v);

    ival = dal_getIntProperty (attr->dal_rec, attr->fieldName);

    if (CVO_DEBUG)
	fprintf (stderr, "cvo_intValue: name='%s' indx=%d  rec=%ld ival=%d\n",
	    attr->fieldName, attr->fieldIndex, (long)attr->dal_rec, ival);

    return (ival);
}


/**
 *  CVO_FLOATVALUE -- Get a floating-point value from an attribute.
 *
 *  @brief   Get a floating-point value from an attribute.
 *  @fn      dval = cvo_floatValue (QRAttribute v)
 *
 *  @param   rec        QRAttribute object handle
 *  @returns            double-precision value
 */
double
cvo_floatValue (QRAttribute v)
{
    double   dval = (double) 0.0;
    dalAttr  *attr = svr_H2P (v);

    dval = dal_getFloatProperty (attr->dal_rec, attr->fieldName);

    if (CVO_DEBUG)
	fprintf (stderr, "cvo_intValue: name='%s' indx=%d  rec=%ld dval=%g\n",
	    attr->fieldName, attr->fieldIndex, (long)attr->dal_rec, dval);

    return (dval);
}


/**
 *  CVO_GETINTATTR -- Get an integer attribute value from a record.
 *
 *  @brief   Get an integer attribute value from a record.
 *  @fn      ival = cvo_getIntAttr (QRecord rec, char *attrname)
 *
 *  @param   rec        QRecord object handle
 *  @param   attrname   attribute name
 *  @returns            integer value
 */
int
cvo_getIntAttr (QRecord rec, char *attrname) 
{
    return ( cvo_intValue ( cvo_getAttribute (rec, attrname)) );
}


/**
 *  CVO_GETFLOATATTR -- Get a floating-point attribute value from a record.
 *
 *  @brief   Get a floating-point attribute value from a record.
 *  @fn      dval = cvo_getFloatAttr (QRecord rec, char *attrname)
 *
 *  @param   rec        QRecord object handle
 *  @param   attrname   attribute name
 *  @returns            double-precision value
 */
double
cvo_getFloatAttr (QRecord rec, char *attrname)
{
    return ( cvo_floatValue ( cvo_getAttribute (rec, attrname)) );
}


/**
 *  CVO_GETSTRINGATTR -- Get a string attribute value from a record.
 *
 *  @brief   Get a string attribute value from a record.
 *  @fn      str = cvo_getStringAttr (QRecord rec, char *attrname)
 *
 *  @param   rec        QRecord object handle
 *  @param   attrname   attribute name
 *  @returns            string value
 */
char *
cvo_getStringAttr (QRecord rec, char *attrname) 
{
    return ( cvo_stringValue ( cvo_getAttribute (rec, attrname)) );
}


/**
 *  CVO_GETDATASET -- Download the AccessReference dataset object to the named 
 *  file.  If fname is NULL, a temp file will be created and the fname
 *  pointer allocated with a string containing the name.
 * 
 *  @brief   Download the AccessReference dataset.
 *  @fn      stat = cvo_getDataset (QRecord rec, char *acref, char *fname) 
 *
 *  @param   rec        QRecord object handle
 *  @param   acref      access reference URL
 *  @param   fname      saved output filename
 *  @returns            ERR or number of bytes in return msg
 */
int 
cvo_getDataset (QRecord rec, char *acref, char *fname) 
{
    dalRec *r = svr_H2P (rec);

    return ( dal_getDataset (r->dal_rec, acref, fname) );
}


/******************************************************************************
 ***    NOT YET IMPLEMENTED
 *****************************************************************************/

/**
 *  CVO_SETINTATTR -- Set an integer attribute value in a record.
 *
 *  @brief   Set an integer attribute value in a record.
 *  @fn      cvo_setIntAttr (QRecord rec, char *attrname, int ival)
 *
 *  @param   rec        QRecord object handle
 *  @param   attrname   attribute name
 *  @param   ival       integer value to set
 *  @returns            nothing
 */
void 
cvo_setIntAttr (QRecord rec, char *attrname, int ival)
{
}


/**
 *  CVO_SETFLOATATTR -- Set a floating-point attribute value in a record.
 *
 *  @brief   Set a floating-point attribute value in a record.
 *  @fn      cvo_setFloatAttr (QRecord rec, char *attrname, double dval)
 *
 *  @param   rec        QRecord object handle
 *  @param   attrname   attribute name
 *  @param   dval       double-precision value to set
 *  @returns            nothing
 */
void 
cvo_setFloatAttr (QRecord rec, char *attrname, double dval)
{
}


/**
 *  CVO_SETSTRINGATTR -- Set a string attribute value in a record.
 *
 *  @brief   Set a string attribute value in a record.
 *  @fn      cvo_setStringAttr (QRecord rec, char *attrname, char *str)
 *
 *  @param   rec        QRecord object handle
 *  @param   attrname   attribute name
 *  @param   str        string value to set
 *  @returns            nothing
 */
void 
cvo_setStringAttr (QRecord rec, char *attrname, char *str)
{
}
