/**
 *  CVO_REGISTRYQUERY -- Utility code to act as a client interface to
 *  the VO Registry service.
 *
 *
 *  RegistryQuery
 *  ----------------------
 * 
 *  High-Level Query:
 * 
 *           res = cvo_regSearch (sql, keywords, orValues)
 *   res =cvo_regSearchByService (svc, term, orValues)
 * 
 *  Programmatic Query:
 * 
 *          query = cvo_regQuery (term, orValues) 	// OR keyword list?
 * 
 *           cvo_regConstSvcType (query, svcType)	// search constraints
 *          cvo_regConstWaveband (query, waveband)
 *                cvo_regDALOnly (query, value)
 *                cvo_regSortRes (query, value)
 * 
 *          cvo_regAddSearchTerm (query, term, orValue)	// OR term w/ previous
 *       cvo_regRemoveSearchTerm (query, term)		// remove search term
 *     count = cvo_regGetSTCount (query)
 * 
 *   str = cvo_regGetQueryString (query)		// GET form of query
 * 
 *          res = cvo_regExecute (query)		// return result obj
 *       str = cvo_regExecuteRaw (query)		// return raw XML
 * 
 *  RegistryQueryResult
 * 
 *     count = cvo_resGetCount  (res)
 * 
 *         str = cvo_resGetStr  (res, attribute, index)
 *      dval = cvo_resGetFloat  (res, attribute, index)
 *        ival = cvo_resGetInt  (res, attribute, index)
 * 
 *     For this implementation, we've chose to use the NVO Registry at
 *  JHU/STScI, specifically the QueryRegistry() method which provides a
 *  'SimpleResource' form of the resource record.  Support for the newer
 *  IVOA standard will be added later, for now we can quickly access the most
 *  commonly used fields of a resource using both a keyword and SQL form of
 *  the search.
 * 
 *
 *  @file       cvoRegistry.c
 *  @author     Michael Fitzpatrick
 *  @version    January 2014
 *
 *************************************************************************
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#define __USE_GNU
#define _GNU_SOURCE
#include <string.h>
#include <errno.h>

#define _VOCLIENT_LIB_
#include "VOClient.h"
#include "vocServer.h"
#include "cvoRegistry.h"
#include "votParse.h"


/*  Public procedures
 */
extern handle_t  svr_newHandle (int context, void *p); 	/* server handles */
extern void     *svr_H2P (handle_t h);
extern handle_t  svr_P2H (void *p);


/*  Private procedures
 */
static regQueryRes *queryExecute (regQuery *query);
static regQueryRes *newVOTResourceArray (regQuery *query, char *response);

static void  addSearchTerm (regQuery *query, char *term, int orValues);
static void  removeSearchTerm (regQuery *query, char *term);
static void  printSearchTerms (regQuery *query);
static void  setResourceValue (VOTResource *vot, char *name, char *value);
static void  setArrayValue (char names[MAX_TAGS][SZ_STR], char *values);

static char *getQueryString (regQuery *query);
static char *formKeywSearchTerm (searchTerm *st);
static char *multiEncode (char names[MAX_TAGS][SZ_STR]);

static int   isKeywordSearch (char *s);


extern VOClient *vo;                    /* Interface runtime struct     */



/**
 *  CVO_REGSEARCH --  High-level procedure to form a query and execute it
 *  immediately.  We allow that 'term1' may be a complex SQL WHERE predicate,
 *  and that 'term2' (or vice versa) is a search-keyword list.  The
 *  'orValues' applies to the keyword list (if present), otherwise it applies
 *  to the two search term elements.  The default action if two terms are
 *  specified is to logically AND them.
 *
 *  The thinking here is that one might want SIAP services for Quasars.  This
 *  is easily expressed in an SQL form to get SIAP resources, however a
 *  Quasar may be known as a QSO, AGN, active-nuclei, etc and so we need a 
 *  easy way to OR the keywords but AND that result with the SQL predicate.
 *
 *  @brief  High-level Registry query interface
 *  @fn	    res = cvo_regSearch (char *term1, char *term2, int orValues)
 *
 *  @param  term1	first search term
 *  @param  term2	second search term
 *  @param  orValues	logically OR values?
 *  @returns		handle to Registry result object
 */
RegResult 
cvo_regSearch (char *term1, char *term2, int orValues)
{
    RegResult res = (RegResult) VOC_NULL;
    regQuery *query = (regQuery *) VOC_NULL;
    regQueryRes *ra = (regQueryRes *) VOC_NULL;


    if (REG_DEBUG)
	fprintf (stderr, "in cvo_regSearch\n");

    if (!term1 && !term2)  				// NULL search terms.
	return ( (RegResult) res);

    query = (regQuery *) calloc (1, sizeof (regQuery));
    if (term1 && *term1) {				// keyword search tern
        addSearchTerm (query, term1, FALSE);
        strcpy (query->terms[0].term, term1);
    }
    if (term2 && *term2)  				// SQL constraint only
        addSearchTerm (query, term2, orValues);

    query->DALOnly = 0;
    query->sortRes = 1;


    if (REG_DEBUG) {
	printSearchTerms (query);
	fprintf (stderr, "qstring = '%s'\n", getQueryString (query)); 
    }

    /*  Execute the Registry query.
     */
    ra = queryExecute (query);

    /*  Create a handle for the result struct.
     */
    res = svr_newHandle (vo->handle_context, (void *) ra);

    return ((RegResult) res);
}


/**
 *  CVO_REGSEARCHBYSERVICE -- Search the Registry using a search term and
 *  constrain by service type. 
 *
 *  @brief  Search Registry using a search term and service constraint
 *  @fn	    res = cvo_regSearchByService (char *svc, char *term, int orValues)
 *
 *  @param  svc 	service type constraint
 *  @param  term	keyword search term
 *  @param  orValues	logically OR values?
 *  @returns		handle to Registry result object
 */
RegResult 
cvo_regSearchByService (char *svc,  char *term, int orValues)
{
    if (svc) {
	char sql[SZ_FNAME];

	memset (sql, 0, SZ_FNAME);
        sprintf (sql, "ResourceType like '%%%s%%'", svc);
	return ( (RegResult) cvo_regSearch (sql, term, orValues) );

    } else
        return ( (RegResult) cvo_regSearch (NULL, term, orValues) );
}


/**
 *  CVO_REGQUERY --  Create a RegistryQuery object.
 *
 *  @brief  Create a RegistryQuery object.
 *  @fn	    v = cvo_regQuery (char *term, int orValues)
 *
 *  @param  term	keyword search term
 *  @param  orValues	logically OR values?
 *  @returns		handle to Registry Query object
 */
RegQuery  
cvo_regQuery (char *term, int orValues)
{
    regQuery *query = (regQuery *) calloc (1, sizeof (regQuery));
    RegQuery qhandle = svr_newHandle (vo->handle_context, (void *) query);

    if (REG_DEBUG)
	fprintf (stderr, "cvo_regQuery:  term='%s'\n", term);

    addSearchTerm (query, term, orValues);

    return (qhandle);
}


/**
 *  CVO_REGADDSEARCHTERM -- Add a search term (sql predicate or keyword list)
 *  to the specified query.
 *
 *  @brief  Add a search term to the specified query
 *  @fn	    cvo_regAddSearchTerm (RegQuery query, char *term, int orValue)
 *
 *  @param  query	Registry query handle
 *  @param  term	keyword search term
 *  @param  orValues	logically OR values?
 *  @returns		nothing
 */
void
cvo_regAddSearchTerm (RegQuery query, char *term, int orValue)
{
    addSearchTerm ((regQuery *) svr_H2P(query), term, orValue);
}


/**
 *  CVO_REMOVESEARCHTERM -- Remove the search term from the query.
 *
 *  @brief  Remove a search term to the specified query
 *  @fn	    cvo_regRemoveSearchTerm (RegQuery query, char *term)
 *
 *  @param  query	Registry query handle
 *  @param  term	keyword search term
 *  @returns		nothing
 */
void
cvo_regRemoveSearchTerm (RegQuery query, char *term)
{
    removeSearchTerm ((regQuery *) svr_H2P(query), term);
}


/**
 *  CVO_REGCONSTWAVEBAND -- Constrain the Registry search by waveband.
 *
 *  @brief  Constrain the Registry search by waveband.
 *  @fn	    cvo_regConstWaveband (RegQuery query, char *waveband)
 *
 *  @param  query	Registry query handle
 *  @param  waveband	waveband string
 *  @returns		nothing
 */
void
cvo_regConstWaveband (RegQuery query, char *waveband)
{
    regQuery *q = svr_H2P(query);

    if (q)
        strcpy (q->waveband, waveband);
}


/**
 *   CVO_REGCONSTSVCTYPE -- Constraing the Registry search by service type.
 *
 *  @brief  Constrain the Registry search by service type.
 *  @fn	    cvo_regConstWaveband (RegQuery query, char *svcType)
 *
 *  @param  query	Registry query handle
 *  @param  svcType	service type string
 *  @returns		nothing
 */
void
cvo_regConstSvcType (RegQuery query, char *svcType)
{
    regQuery *q = svr_H2P(query);

    if (q)
        strcpy (q->svcType, svcType);
}


/**
 *  CVO_REGDALONLY -- Set the "DAL Only" flag.  If set, we expand a resource
 *  search to break out the individual DAL services into separate results.
 *
 *  @brief  Set the "DAL Only" flag
 *  @fn	    cvo_regDALOnly (RegQuery query, int value)
 *
 *  @param  query	Registry query handle
 *  @param  value	value of the DAL-only flag
 *  @returns		nothing
 */
void
cvo_regDALOnly (RegQuery query, int value)
{
    regQuery *q = svr_H2P(query);

    if (q)
        q->DALOnly = value;
}


/**
 *  CVO_REGSORTRES -- Set the resource "sort" flag.   If enabled, we try to
 *  order the resource table by some logical means.
 *
 *  @brief  Set the resource "sort" flag
 *  @fn	    cvo_regSortRes (RegQuery query, int value)
 *
 *  @param  query	Registry query handle
 *  @param  value	value of the sort flag
 *  @returns		nothing
 */
void
cvo_regSortRes (RegQuery query, int value)
{
    regQuery *q = svr_H2P(query);

    if (q)
        q->sortRes = value;
}


/**
 *  CVO_REGGETSTCOUNT -- Get the number of search terms in the current query.
 *
 *  @brief  Get the number of search terms in the current query.
 *  @fn	    count = cvo_regGetSTCount (RegQuery query)
 *
 *  @param  query	Registry query handle
 *  @returns		nothing
 */
int
cvo_regGetSTCount (RegQuery query)
{
    regQuery *q = svr_H2P(query);
    return ( (q ? q->nterms : 0) );
}


/**
 *  CVO_REGGETQUERYSTRING -- Get the current query as an http GET URL.
 *
 *  @brief  Get the current query as an http GET URL.
 *  @fn	    url = cvo_regGetQueryString (RegQuery query)
 *
 *  @param  query	Registry query handle
 *  @returns		query URL
 */
char *
cvo_regGetQueryString (RegQuery query)
{
    regQuery *q = (regQuery *) svr_H2P (query);
    return ( (q ? getQueryString (q) : NULL) );
}


/**
 *  CVO_REGEXECUTE -- Execute the specified query, returning a result object
 *  code or NULL.
 *
 *  @brief  Execute the specified query
 *  @fn	    res = cvo_regExecute (RegQuery query)
 *
 *  @param  query	Registry query handle
 *  @returns		registry result object handle
 */
RegResult
cvo_regExecute (RegQuery query)
{
    RegResult res = (RegResult) VOC_NULL;
    regQuery *q = (regQuery *) svr_H2P (query);

    if (q) {
        regQueryRes *ra = queryExecute (q);	/* execute the query 	   */
        res = svr_newHandle (vo->handle_context, (void *) ra);
    }
    return ((RegResult) res);
}


/**
 *  CVO_REGEXECUTERAW -- Execute the specified query and return the raw
 *  resulting XML string.
 *
 *  @brief  Execute the specified query and return raw result string
 *  @fn	    str = cvo_regExecuteRaw (RegQuery query)
 *
 *  @param  query	Registry query handle
 *  @returns		raw data return from data (caller must free)
 */
char *
cvo_regExecuteRaw (RegQuery query)
{
    regQuery *q = (regQuery *) svr_H2P (query);
    char  *qstring = NULL;

    if (q && (qstring = getQueryString (q)) )	/*  get the query URL 	*/
        return ( svr_getURL (qstring) );
    else
	return ( (char *) NULL );
}


/*****************************************************************************/
/*****		      RegistryQueryResult Methods  			******/
/*****************************************************************************/


/**
 *  CVO_RESGETCOUNT -- Return a count of the number of results records.
 *
 *  @brief  Return a count of the number of results records.
 *  @fn	    count = cvo_resGetCount (RegResult res)
 *
 *  @param  res 	Registry result handle
 *  @returns		number of result records
 */
int
cvo_resGetCount (RegResult res)
{
    regQueryRes *r = (regQueryRes *) svr_H2P (res);

    return ( (r ? r->nresources : 0) );
}


/**
 *  CVO_GETSTR -- Get a string-valued attribute from the result resource
 *  record.  Currently recognized real-valued attributes include:
 *
 *	Title			Resource title (long version)
 *	ShortName		Short name of Resource
 *	ServiceURL		Service URL (if appropriate)
 *	ReferenceURL		URL to reference about Resource
 *	Description		Text description of resource
 *	Identifier		Standard ivo identifier of resource
 *	ServiceType		Service Type (Cone, Siap, etc)
 *	Type			Resource Type (catalog, survey, etc)
 *	CoverageSpatial		Spatial coverage (STC)
 *	CoverageTemporal	Temporal coverage of data
 *
 *	CoverageSpectral	Spectral coverage (csv list of bandpasses)
 *	ContentLevel		Content level (research, EPO, etc -- csv list)
 *
 *  Attribute strings are case-insensitive.
 *
 *  @brief  Get a string-valued attribute from the result resource record
 *  @fn	    str = cvo_resGetStr (RegResult res, char *attr, int index)
 *
 *  @param  res 	Registry result handle
 *  @param  attr 	record attribute
 *  @param  index 	record index
 *  @returns		string-valued attribute
 */
char *
cvo_resGetStr (RegResult res, char *attr, int index)
{
    regQueryRes *ra = (regQueryRes *) svr_H2P (res);
    VOTResource *vot = (VOTResource *) NULL;
    char  *str = NULL;


    if (!ra)
        return ( (char *) NULL );

    vot = (VOTResource *) &ra->resources[index];

    if (strcasecmp (attr, 		"shortName") == 0) {
	str = strdup (vot->shortName);
    } else if (strcasecmp (attr, 	"title") == 0 ||
	       strcasecmp (attr, 	"name") == 0) {
	str = strdup (vot->title);
    } else if (strcasecmp (attr, 	"description") == 0) {
	str = strdup (vot->description);
    } else if (strcasecmp (attr, 	"publisher") == 0 ||
	       strcasecmp (attr, 	"creator") == 0) {
	str = strdup (vot->publisher);
    } else if (strcasecmp (attr, 	"publisherID") == 0) {
	str = strdup (vot->publisherID);
    } else if (strcasecmp (attr, 	"identifier") == 0) {
	str = strdup (vot->identifier);
    } else if (strcasecmp (attr, 	"updated") == 0 ||
	       strcasecmp (attr, 	"date") == 0) {
	str = strdup (vot->updated);
    } else if (strcasecmp (attr, 	"referenceURL") == 0) {
	str = strdup (vot->referenceURL);
    } else if (strcasecmp (attr, 	"version") == 0) {
	str = strdup (vot->version);
    } else if (strcasecmp (attr, 	"resourceID") == 0) {
	str = strdup (vot->resourceID);

    } else if (strcasecmp (attr, 	"waveband") == 0 ||
	       strcasecmp (attr, 	"coverageSpectral") == 0) {
	str = multiEncode (vot->waveband);
    } else if (strcasecmp (attr, 	"subject") == 0) {
	str = multiEncode (vot->subject);
    } else if (strcasecmp (attr, 	"type") == 0) {
	str = multiEncode (vot->type);
    } else if (strcasecmp (attr, 	"contentLevel") == 0) {
	str = multiEncode (vot->contentLevel);
        
    } else if (strcasecmp (attr, 	"tags") == 0) {
	str = multiEncode (vot->tags);
    } else if (strcasecmp (attr, 	"capabilityName") == 0) {
	str = multiEncode (vot->capabilityName);
    } else if (strcasecmp (attr, 	"capabilityClass") == 0 ||
	       strcasecmp (attr, 	"resourceType") == 0 ||
	       strcasecmp (attr, 	"serviceType") == 0) {
	str = multiEncode (vot->capabilityClass);
    } else if (strcasecmp (attr, 	"capabilityStandardID") == 0) {
	str = multiEncode (vot->capabilityStandardID);
    } else if (strcasecmp (attr, 	"capabilityValidationLevel") == 0) {
	str = multiEncode (vot->capabilityValidationLevel);

    } else if (strcasecmp (attr, 	"interfaceClass") == 0) {
	str = multiEncode (vot->interfaceClass);
    } else if (strcasecmp (attr, 	"interfaceVersion") == 0) {
	str = multiEncode (vot->interfaceVersion);
    } else if (strcasecmp (attr, 	"interfaceRole") == 0) {
	str = multiEncode (vot->interfaceRole);

    } else if (strcasecmp (attr, 	"accessURL") == 0 ||
	       strcasecmp (attr, 	"serviceURL") == 0) {
	str = multiEncode (vot->accessURL);
    } else if (strcasecmp (attr, 	"supportedInputParam") == 0) {
	str = multiEncode (vot->supportedInputParam);
    } else if (strcasecmp (attr, 	"maxRadius") == 0) {
	str = multiEncode (vot->maxRadius);
    } else if (strcasecmp (attr, 	"maxRecords") == 0) {
	str = multiEncode (vot->maxRecords);

    } else
	return (NULL);

    return (str);
}


/**
 *  CVO_GETFLOAT -- Get a real-valued attribute from the result resource
 *  record.  Currently recognized real-valued attributes include:
 *
 *	MaxSR			maximum search radius
 *
 *  Attribute string are case-insensitive.
 *
 *  @brief  Get a real-valued attribute from the result resource record
 *  @fn	    dval = cvo_resGetFloat (RegResult res, char *attr, int index)
 *
 *  @param  res 	Registry result handle
 *  @param  attr 	record attribute
 *  @param  index 	record index
 *  @returns		string-valued attribute
 */
double
cvo_resGetFloat (RegResult res, char *attr, int index)
{
    regQueryRes *ra = (regQueryRes *) svr_H2P (res);
    VOTResource *vot = (VOTResource *) NULL;

    if (!ra)
        return (0.0);

    vot = (VOTResource *) &ra->resources[index];

    if (strcasecmp (attr, "regionOfRegard") == 0)
	return (vot->regionOfRegard);
    else
	return ( (double) 0.0 );
}


/**
 *  CVO_GETINT -- Get a integer-valued attribute from the result resource
 *  record.  Currently recognized real-valued attributes include:
 *
 *	MaxRecords		maximum records returned by the service
 *
 *  Attribute string are case-insensitive.
 *
 *  @brief  Get an int-valued attribute from the result resource record
 *  @fn	    ival = cvo_resGetInt (RegResult res, char *attr, int index)
 *
 *  @param  res 	Registry result handle
 *  @param  attr 	record attribute
 *  @param  index 	record index
 *  @returns		string-valued attribute
 */
int
cvo_resGetInt (RegResult res, char *attr, int index)
{
    regQueryRes *ra = (regQueryRes *) svr_H2P (res);
    VOTResource *vot = (VOTResource *) NULL;

    if (!ra)
        return (0);

    vot = (VOTResource *) &ra->resources[index];

    if (strcasecmp (attr, 		"numCapabilities") == 0) {
	return (vot->numCapabilities);
    } else if (strcasecmp (attr, 	"numInterfaces") == 0) {
	return (vot->numInterfaces);
    } else if (strcasecmp (attr, 	"numStdCapabilities") == 0) {
	return (vot->numStdCapabilities);

    } else
	return ( 0 );
}



/***************************************************************************/
/*****   Private Procedures                                            *****/
/***************************************************************************/


/**
 *  QUERYEXECUTE -- Execute the specified query object, returning a
 *  VOTResource structure.
 */
static regQueryRes *
queryExecute (regQuery *query)
{
    char  *qstring  = (char *) NULL;
    char  *response = (char *) NULL;
    regQueryRes *vra = (regQueryRes *) NULL;
    extern int debug;


    if (!query)
	return ( (regQueryRes *) NULL );
    
    qstring = getQueryString (query); 		/*  get the query URL 	*/
    if (debug)
	fprintf (stderr, "qstring:\n\n%s\n\n", qstring);

    response = svr_getURL (qstring); 		/*  make the call 	*/
    if (debug)
	fprintf (stderr, "response:\n\n%s\n\n", response);

    /* Parse the response VOTable to create the VOTResource array.
     */
    vra = newVOTResourceArray (query, response);

    if (response)
	free ( (void *) response );

    return ( (regQueryRes *) vra );
}


/**
 *  NEWVOTRESOURCEARRAY -- Parse a response and create a resource array
 *  object.
 */
static regQueryRes *
newVOTResourceArray (regQuery *query, char *response)
{
    handle_t  vot = (handle_t) NULL;		/* votable parser	*/
    char  *id=NULL, *s=NULL;
    int    res, tab, data, tdata, field, tr, td;
    int    i, ncols=0, nrows=0, resnum=0, colnum=0;
    regQueryRes *va = (regQueryRes *) NULL;
    VOTResource *votres = (VOTResource *) NULL;


    vot_setWarnings (0);			/* disable messages	*/
    if ( (vot = vot_openVOTABLE (response) ) <= 0)
        return ( (regQueryRes *) NULL );


    /*  Get the needed handles.
     */
    if ( (res   = vot_getRESOURCE (vot)) ) {
        if ( (tab   = vot_getTABLE (res)) ) {
            if ( (data  = vot_getDATA (tab)) ) {
                if ( (tdata = vot_getTABLEDATA (data)) ) {
    		    nrows = vot_getNRows (tdata);    /* number of resources   */
    		    ncols = vot_getNCols (tdata);    /* number of attributes  */
		} else
	            return ( (regQueryRes *) NULL );
	    } else 
	        return ( (regQueryRes *) NULL );
	} else
	    return ( (regQueryRes *) NULL );
    } else
	return ( (regQueryRes *) NULL );

	
    if (REG_DEBUG)
	fprintf (stderr, "nrows = %d   ncols = %d\n", nrows, ncols);
	

    /*  Allocate the response struct, and the individual resources.
     */
    va = (regQueryRes *) calloc (1, sizeof (regQueryRes));
    va->resources = (VOTResource *) calloc (nrows, sizeof (VOTResource));
    va->nresources = nrows;


    /* Collect the column names.
    */
    for (field=vot_getFIELD (tab); field; field = vot_getNext (field)) {
        id   = vot_getAttr (field, "id");

	if (REG_DEBUG)
	    fprintf (stderr, "col %d = '%s'\n", colnum, id);
	if (id)
	    strcpy (va->colnames[colnum], id);
	colnum++;
    }


    /* Now dump the data.
    */
    for (tr=vot_getTR (tdata); tr; tr=vot_getNext (tr), resnum++) {
        votres = &va->resources[resnum];
        for (td=vot_getTD (tr), i=0; td; td=vot_getNext (td),i++) {
	    setResourceValue (votres, 
		va->colnames[i], ((s = vot_getValue (td)) ? s : "") );
        }
    }

    vot_closeVOTABLE (vot);			/* close the VOTable	*/
    return ( (regQueryRes *) va );
}


/**
 *  MULTIENCODE -- Encode a list of string values
 */
static char *
multiEncode (char names[MAX_TAGS][SZ_STR])
{
    char  buf[SZ_URL];
    int   i = 0;

    memset (buf, 0, SZ_URL);
    for (i=0; i < MAX_TAGS; i++) {
	if (names[i][0])
	    strcat (buf, names[i]);
	else
	    break;
	if (names[i+1][0])
	    strcat (buf, ",");
    }
    return ( strdup (buf) );
}


/**
 *  SETARRAYVALUE -- Break a multi-value string to a VOTResource array element.
 */
static void
setArrayValue (char names[MAX_TAGS][SZ_STR], char *values)
{
    int  i = 0;
    char  *ip, *op,  buf[SZ_STR];


    ip = values;
    if ( *ip == '#' ) {
        while (*ip) {
	    ip++;				/* skip the '#'		*/
	    memset (buf, 0, SZ_STR);
	    for (op=&buf[0]; *ip && *ip != '#'; ip++)
		*op++ = *ip;
 	    strcpy (names[i++], buf);
        }
	
    } else {
	/*  A single value, just copy it to the output.
	 */
 	strcpy (names[0], values);
    }
}


/**
 *  SETRESOURCEVALUE -- Save the VOTResource value in the column.
 */
static void
setResourceValue (VOTResource *vot, char *name, char *value)
{
    if (!value)
	return;

    if (strcasecmp (name, 		"shortName") == 0)
	strcpy (vot->shortName, value);
    else if (strcasecmp (name, 		"title") == 0)
	strcpy (vot->title, value);
    else if (strcasecmp (name, 		"description") == 0)
	strcpy (vot->description, value);
    else if (strcasecmp (name, 		"publisher") == 0)
	strcpy (vot->publisher, value);
    else if (strcasecmp (name, 		"publisherID") == 0)
	strcpy (vot->publisherID, value);
    else if (strcasecmp (name, 		"identifier") == 0)
	strcpy (vot->identifier, value);
    else if (strcasecmp (name, 		"updated") == 0)
	strcpy (vot->updated, value);

    else if (strcasecmp (name, 		"tags") == 0)
	setArrayValue (vot->tags, value);
    else if (strcasecmp (name, 		"waveband") == 0)
	setArrayValue (vot->waveband, value);
    else if (strcasecmp (name, 		"subject") == 0)
	setArrayValue (vot->subject, value);
    else if (strcasecmp (name, 		"type") == 0)
	setArrayValue (vot->type, value);
    else if (strcasecmp (name, 		"contentLevel") == 0)
	setArrayValue (vot->contentLevel, value);

    else if (strcasecmp (name, 		"capabilityName") == 0)
	setArrayValue (vot->capabilityName, value);
    else if (strcasecmp (name, 		"capabilityClass") == 0)
	setArrayValue (vot->capabilityClass, value);
    else if (strcasecmp (name, 		"capabilityStandardID") == 0)
	if (strncmp (value, "ivo://ivoa.net/std/", 19) == 0)
	    setArrayValue (vot->capabilityStandardID, &value[19]);
	else
	    setArrayValue (vot->capabilityStandardID, value);
    else if (strcasecmp (name, 		"capabilityValidationLevel") == 0)
	setArrayValue (vot->capabilityValidationLevel, value);

    else if (strcasecmp (name, 		"interfaceClass") == 0)
	setArrayValue (vot->interfaceClass, value);
    else if (strcasecmp (name, 		"interfaceVersion") == 0)
	setArrayValue (vot->interfaceVersion, value);
    else if (strcasecmp (name, 		"interfaceRole") == 0)
	setArrayValue (vot->interfaceRole, value);

    else if (strcasecmp (name, 		"accessURL") == 0)
	setArrayValue (vot->accessURL, value);
    else if (strcasecmp (name, 		"supportedInputParam") == 0)
	setArrayValue (vot->supportedInputParam, value);
    else if (strcasecmp (name, 		"maxRadius") == 0)
	setArrayValue (vot->maxRadius, value);
    else if (strcasecmp (name, 		"maxRecords") == 0)
	setArrayValue (vot->maxRecords, value);

    else if (strcasecmp (name, 		"referenceURL") == 0)
	strcpy (vot->referenceURL, value);
    else if (strcasecmp (name, 		"version") == 0)
	strcpy (vot->version, value);
    else if (strcasecmp (name, 		"resourceID") == 0)
	strcpy (vot->resourceID, value);
        
    else if (strcasecmp (name, 		"regionOfRegard") == 0)
	vot->regionOfRegard = atof (value);
    else if (strcasecmp (name, 		"numCapabilities") == 0)
	vot->numCapabilities = atoi (value);
    else if (strcasecmp (name, 		"numInterfaces") == 0)
	vot->numInterfaces = atoi (value);
    else if (strcasecmp (name, 		"numStdCapabilities") == 0)
	vot->numStdCapabilities = atoi (value);

    else
	fprintf (stderr, "invalid VOTResource name '%s'\n", name);
}


/**
 *  ADDSEARCHTERM -- Add a search term to a query object.
 */
static void
addSearchTerm (regQuery *query, char *term, int orValues)
{
    if (query) {
        int  tnum = 0;

	tnum = query->nterms;
        strcpy (query->terms[tnum].term, term);
        query->terms[tnum].orValue = orValues;
	query->nterms++;
    }
}


/**
 *  REMOVESEARCHTERM -- Remove a search term from a query object.
 */
static void
removeSearchTerm (regQuery *query, char *term)
{
    if (query) {
        int  i, j;

	for (i=0; i < query->nterms; i++) {
	    if (strcasecmp (query->terms[i].term, term) == 0) {
		/*  Down shift the rest of the terms. 
		 */
		for (j=i+1; j < query->nterms; j++) {
        	    strcpy (query->terms[j-1].term, query->terms[j].term);
        	    query->terms[j-1].orValue = query->terms[j].orValue;
		}
		query->nterms--;
	    }
	}
    }
}


/**
 *  PRINTSEARCHTERM -- Print a search term to a query object.
 */
static void
printSearchTerms (regQuery *query)
{
    register int i;

    if (query) {
	for (i=0; i < query->nterms; i++) 
	    printf ("term[%d] = '%s'   or = %d\n", 
		query->nterms, query->terms[i].term, query->terms[i].orValue);
    }
}


/**
 *  GETQUERYSTRING -- Form a query URL to the VOTable Registry service.
 */
static char *
getQueryString (regQuery *query)
{
    register int i = 0;
    static char url[SZ_URL];
    char   searchKeys[SZ_URL], value[SZ_URL], keyTerms[SZ_URL], *s;
    char  *andKeys  = "TRUE";
    int    keywOnly = TRUE;


    /* Initialize the URL.
     */
    memset (url, 0, SZ_URL);
    memset (searchKeys, 0, SZ_URL);
    memset (keyTerms, 0, SZ_URL);

    if (getenv ("OLD_REGISTRY"))
	sprintf (url, "%s", OLD_REGISTRY);
    else if (getenv ("VOC_REGTEST"))
	sprintf (url, "%s", TEST_REGISTRY);
    else
	sprintf (url, "%s", DEF_REGISTRY);
    if (REG_DEBUG)
	fprintf (stderr, "getQueryString:  url='%s'\n", url);

    /*  Turn the search terms into query parameters.
     */
    for (i=0; i < query->nterms; i++) {
	s = query->terms[i].term;

        memset (value, 0, SZ_URL);
	if (isKeywordSearch (s)) {
	    sprintf (value, "(%s)", formKeywSearchTerm (&query->terms[i]));
	} else {
	    sprintf (value, "(%s)", s);			// SQL search term
	    keywOnly = FALSE;
	}

	strcat (keyTerms, s);
	if (i < (query->nterms - 1))
	    strcat (keyTerms, " ");

	strcat (searchKeys, svr_urlEncode (value));
	if (i < (query->nterms - 1)) {
	    if (query->terms[i].orValue) {
		//strcat (searchKeys, "+OR+");
		strcat (searchKeys, "+ZZZZZ+");
		andKeys = "FALSE";
	    } else
		strcat (searchKeys, "+AND+");
	}
    }


    /*  Form the final query URL.
     */
    if (query->svcType[0] && query->waveband[0]) {
	strcat (url, "VOTCapBandPredOpt");

        strcat (url, "?predicate="); 		strcat (url, searchKeys);
        strcat (url, "&capability="); 		strcat (url, query->svcType);
        strcat (url, "&waveband="); 		strcat (url, query->waveband);
        strcat (url, "&VOTStyleOption=2");

    } else if (query->svcType[0]) {
	strcat (url, "VOTCapabilityPredOpt");

        strcat (url, "?predicate="); 		strcat (url, searchKeys);
        strcat (url, "&capability="); 		strcat (url, query->svcType);
        strcat (url, "&VOTStyleOption=2");

    } else if (keywOnly) {
	strcat (url, "VOTKeyOpt");

        strcat (url, "?keywords="); 		strcat (url, 
						    svr_urlEncode (keyTerms));
        strcat (url, "&andKeys=");		strcat (url, andKeys);
        strcat (url, "&VOTStyleOption=2");

    } else {
	strcat (url, "VOTPredOpt");

        strcat (url, "?predicate="); 		strcat (url, searchKeys);
        strcat (url, "&VOTStyleOption=2");
    }

    if (REG_DEBUG)
	fprintf (stderr, "getQueryString:  final url='%s'\n", url);
    return (url);
}


/**
 *  ISKEYWORDSEARCH -- Test if the search term indicates an SQL string or not.
 */
static int
isKeywordSearch (char *s)
{
    int  value = FALSE;
    extern char *strcasestr(const char *haystack, const char *needle);


    if (s && *s) {
        value = (! (strcasestr (s, "like") || 
		    strcasestr (s, ">")    ||
		    strcasestr (s, "<")    || 
		    strcasestr (s, "=")) );
    }
    return ( value );
}


/**
 *  FORMKEYWSEARCHTERM -- Form an SQL statement for a keyword search term.
 */
static char *
formKeywSearchTerm (searchTerm *st)
{
    static char qstring[SZ_URL];
    char sql[SZ_URL];
    char q[SZ_URL];
    char *s;


    if (!st)
	return (NULL);

    memset (qstring, 0, SZ_URL);
    memset (q, 0, SZ_URL);
    memset (sql, 0, SZ_URL);

    s = st->term;

    strcat(q,"(Title like '%");              strcat (q,s);strcat (q,"%' OR ");
    strcat(q,"ShortName like '%");           strcat (q,s);strcat (q,"%' OR ");
    strcat(q,"Identifier like '%");          strcat (q,s);strcat (q,"%' OR ");
    strcat(q,"[content/subject] like '%");   strcat (q,s);strcat (q,"%' OR ");
    strcat(q,"[curation/publisher] like '%");strcat (q,s);strcat (q,"%' OR ");
    strcat(q,"[content/description] like '%");strcat (q, s);strcat (q, "%')");

    /*  FIXME -- Need to support multiple search term tokens
     */
    sprintf (qstring, "( %s )", q);

    return ( qstring );
}


/*  Debug breakpoint.
 */
void cvo_break (void) { int i = 0; i++; }
