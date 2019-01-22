/**
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "VOClient.h"

double  ra       = 12.0;			/* default values	*/
double  dec      = 12.0;
double  sr       = 0.02;

    
char  *voc_opts = NULL;
char  *server   = "6200:localhost";
char  *service  = \
    "http://gsss.stsci.edu/webservices/vo/ConeSearch.aspx?CAT=GSC23&amp;";

static void callConeService (char *url, double ra, double dec, double sr);


int main (int argc, char *argv[])
{
    /* Process command line arguments.
     */
    if (argc <= 2) {
	/* Use builtin defaults. */
	if (argv[1] && strncmp (argv[1], "-m", 2) == 0)
	    voc_opts = "mini-d";

    } else if (argc >= 3) {
	int arg = 1;
    
	/* Look for a server specification. */
	if (strncmp (argv[arg], "-m", 2) == 0)
	    voc_opts = "mini-d";
	if (strncmp (argv[arg], "-ds", 3) == 0)
	    server = argv[++arg];
	ra   = atof (argv[arg++]);
	dec  = atof (argv[arg++]);
	sr   = atof (argv[arg++]);
	if (arg < argc)
	    service = argv[arg++];

    } else {
        fprintf (stderr, "Usage: zztest [-ds server] ra dec sr [coneURL]\n");
        exit(1);
    }

    /* Now call the Cone Service and summarize the results.
     */
    callConeService (service, ra, dec, sr);

    return (0);
}


/*  Simple test routine to call a Cone search service and summarize results.
 */
static void
callConeService (char *service_url, double ra, double dec, double sr)
{
    char *vot 	   = NULL;

    DAL	      cone;				/* DAL Connection	 */
    Query     query;				/* query handle		 */
	

    /*  Initialize the VOClient code.  Error messages are printed by the
     *  interface so we just quit if there is a problem.
     */
    if (voc_initVOClient (voc_opts) == ERR) 
        return;

    /*  Get a new connection to the named service.
     */
    cone = voc_openConeConnection (service_url);    /* open a connection    */
    query = voc_getConeQuery (cone, ra, dec, sr);   /* form a query  	    */

    if ( (vot = voc_executeVOTable (query)) ) {
        write (fileno(stdout), vot, strlen (vot));
        printf ("\n\n");
    }

    free ((void *) vot);			/* free local storage	     */

    voc_closeConnection (cone);			/* close the cone connection */
    voc_closeVOClient (1);		        /* clean up and shutdown     */

    return;
}
