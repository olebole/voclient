/**
 *  CVO_REGISTRYQUERY.H -- Implementation detils for the CVO Registry code.
 *
 *  @file       cvoRegistry.h
 *  @author     Michael Fitzpatrick
 *  @version    January 2014
 *
 *************************************************************************
 */


#define _VOCLIENT_LIB_

#define REG_DEBUG  	(getenv("REG_DBG")||access("/tmp/REG_DBG",F_OK)==0)


#define	DEF_REGISTRY	"http://vao.stsci.edu/directory/NVORegInt.asmx/"
#define	OLD_REGISTRY	"http://nvo.stsci.edu/vor10/NVORegInt.asmx/"
#define	TEST_REGISTRY	"http://vaotest.stsci.edu/directory/NVORegInt.asmx/"
#define	DEF_METHOD	"VOTPredOpt"

#define MAX_SEARCH_TERMS	64
#define MAX_TAGS		10
#define MAX_ATTRS		40
#define SZ_STR			512
#define SZ_BIGSTR		2048


/**
 *  SEARCHTERM -- Struct defining a registry search term.
 */
typedef struct {
    char    term[SZ_FNAME];			/* search term 		      */
    int     orValue;				/* boolean OR?		      */
} searchTerm, *searchTermP;


/**
 *  VOTRESOURCE -- A VO Registry Resource record from the VOTable interface.
 */
typedef struct {
    char   tags[MAX_TAGS][SZ_STR];		/* resource tags	      */
    char   shortName[SZ_STR];			/* short name		      */
    char   title[SZ_STR];			/* resource title	      */
    char   description[SZ_BIGSTR];		/* description text	      */
    char   publisher[SZ_STR];			/* publisher name	      */
    char   publisherID[SZ_STR];			/* publisher ID		      */
    char   waveband[MAX_TAGS][SZ_STR];		/* wavebands		      */
    char   identifier[SZ_STR];			/* VO identifier	      */
    char   updated[SZ_STR];			/* record update time	      */
    char   subject[MAX_TAGS][SZ_STR];		/* subject keywords	      */
    char   type[MAX_TAGS][SZ_STR];		/* resource type	      */
    char   contentLevel[MAX_TAGS][SZ_STR];	/* content level	      */
    char   version[SZ_STR];			/* resource version	      */
    char   resourceID[SZ_STR];			/* resource ID string	      */

    char   capabilityName[MAX_TAGS][SZ_STR];	/* service name		      */
    char   capabilityClass[MAX_TAGS][SZ_STR];	/* service class	      */
    char   capabilityStandardID[MAX_TAGS][SZ_STR];/* service standard ID	      */
    char   capabilityValidationLevel[MAX_TAGS][SZ_STR];/* validation level    */

    char   interfaceClass[MAX_TAGS][SZ_STR];	/* interface class	      */
    char   interfaceVersion[MAX_TAGS][SZ_STR];	/* interface version	      */
    char   interfaceRole[MAX_TAGS][SZ_STR];	/* interface role	      */

    char   accessURL[MAX_TAGS][SZ_STR];		/* access URL		      */
    char   supportedInputParam[MAX_TAGS][SZ_STR];/* input params		      */
    char   maxRadius[MAX_TAGS][SZ_STR];		/* max search radius	      */
    char   maxRecords[MAX_TAGS][SZ_STR];	/* max return records	      */
    char   referenceURL[SZ_STR];		/* data reference URL	      */

    double regionOfRegard;			/* resolution metric	      */
    int    numCapabilities;			/* # of capabilities          */
    int    numInterfaces;			/* # of interfaces            */
    int    numStdCapabilities;			/* # of std capabilities      */

    int    index;				/* result index		      */
    int    rank;				/* sort rank		      */
    char   sterms[MAX_TAGS][SZ_STR];		/* search terms		      */

    void  *next;				/* linked list next ptr       */
} VOTResource, *VOTResourceP;


/**
 *  REGQUERY -- The main registry query struct containing search terms and
 *  constraints.
 */
typedef struct {
    char    svcType[SZ_FNAME];			/* service type constraint    */
    char    waveband[SZ_FNAME];			/* waveband constraint        */
    int     DALOnly;				/* limit to DAL services      */
    int     sortRes;				/* sort results?	      */

    char    queryURL[SZ_URL];			/* query URL		      */
    searchTerm  terms[MAX_SEARCH_TERMS];	/* search terms		      */
    int	    nterms;				/* number of search terms     */
} regQuery, *regQueryP;


/**
 *  REGQUERYRES -- The main registry query response structure.
 */
typedef struct {
    int	    nresources;				/* number of resources	      */
    VOTResource *resources;			/* Resource linked list       */

    char    colnames[MAX_ATTRS][SZ_FNAME];	/* FIELD names		      */
} regQueryRes, *regQueryResP;



/*  Public procedures
 */
RegResult cvo_regSearch (char *term1, char *term2, int orValues);
RegResult cvo_regSearchByService (char *svc,  char *term, int orValues);

RegQuery  cvo_regQuery (char *term, int orValues);
void      cvo_regAddSearchTerm (RegQuery query, char *term, int orValue);
void      cvo_regRemoveSearchTerm (RegQuery query, char *term);

void      cvo_regConstWaveband (RegQuery query, char *waveband);
void      cvo_regConstSvcType (RegQuery query, char *svcType);
void      cvo_regDALOnly (RegQuery query, int value);
void      cvo_regSortRes (RegQuery query, int value);
int       cvo_regGetSTCount (RegQuery query);
char     *cvo_regGetQueryString (RegQuery query);

RegResult cvo_regExecute (RegQuery query);
char     *cvo_regExecuteRaw (RegQuery query);

int       cvo_resGetCount (RegResult res);
char     *cvo_resGetStr (RegResult res, char *attr, int index);
double    cvo_resGetFloat (RegResult res, char *attr, int index);
int       cvo_resGetInt (RegResult res, char *attr, int index);

