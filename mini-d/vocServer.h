/**
 *  VOCSERVER.H -- Include file for the "Mini Daemon" server.
 *
 *  @file       vocServer.h
 *  @author     Michael Fitzpatrick
 *  @version    April 2013
 *
 *************************************************************************
 */


#ifdef  MAX_PARAMS
#undef  MAX_PARAM
#endif
#define MAX_PARAMS      16

#ifdef  SZ_MSG
#undef  SZ_MSG
#endif
#define SZ_MSG          512


typedef  int    handle_t;


/*  Debug and verbose flags.
 */
#define SVR_DEBUG  (getenv("SVR_DBG")||access("/tmp/SVR_DBG",F_OK)==0)
#define SVR_VERB   (getenv("SVR_VERB")||access("/tmp/SVR_VERB",F_OK)==0)



/**
 *  Method parameter struct.
 */
typedef struct {
    int      type;                              /* parameter type 	*/
    int      ival;                              /* integer param 	*/
    double   dval;                              /* floating point param */
    char    *str;                               /* string-valued param 	*/
    void    *buf;                               /* bulk buffer 		*/
} msgParam, *msgParamP;


/**
 *  Method parameter struct.
 */
typedef struct {
    int      status; 				/* result type 		*/
} msgResult, *msgResultP;


/**
 *  Server method structure.
 */
#define  _SVR_METHOD_SIG_	(int objid, msgParam *pars, int npars);

typedef struct {
   char     *name;                         	/* method name 		*/
   vocRes_t *(*func) _SVR_METHOD_SIG_
} svrMethod;



/******************************************************************************
 *  Sesame Service Declarations
 */

#define MAX_SESAME_ALIASES	16


/*  Result record of a Sesame query.
 */
typedef struct {
    char    target[SZ_FNAME];           	/* target name          */
    char    otype[SZ_FNAME];            	/* object type          */
    double  ra, dec;
    double  raERR, decERR;

    char    service[SZ_FNAME];          	/* service name         */
    char    oname[SZ_FNAME];            	/* object name          */
    char    pos[SZ_FNAME];              	/* jpos string          */
    char   *aliases[MAX_SESAME_ALIASES]; 	/* NYI                  */
    int     naliases;

    Sesame  handle;                     	/* API handle           */
} Resolver, *ResolverP;





/******************************************************************************
 *  Method declarations.
 */
vocRes_t *svr_processMsg (vocMsg_t *msg);

char  *svr_urlEncode (char *str);
char  *svr_encIntResult (int status, int nitems, int val);
char  *svr_getURL (char *url);

int    svr_fileRead (int fd, void *vptr, int nbytes);
int    svr_fileWrite (int fd, void *vptr, int nbytes);


