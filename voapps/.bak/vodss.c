/**
 *  VODSS -- Query the DSS service for an image of a field
 *
 *  Usage:   vodss [<opts>] <name> | <ra dec>
 *
 *  Where
 *       -%%,--test             run unit tests
 *       -h,--nelp              this message
 *       -r,--return            return result from method
 *
 * 	 -s,--size <size>	Field size
 *	      <size>s		Field size (arcsec)
 *	      <size>m		Field size (arcmin)
 *	      <size>d		Field size (degrees, default)
 *	 -1,--dss1		Get the DSS1 image
 *	 -2,--dss2		Get the DSS2 image
 *	 -S,--samp		Broadcast a SAMP image.load.fits message
 *	 -o <name>		Save image to named file
 *
 *	 <name>			Target name to be resolved to position
 *	 <ra> <dec>		Position to query (dec. deg or Eq. sexagesimal)
 *
 *
 *  @file       vodss.c
 *  @author     Mike Fitzpatrick
 *  @date       5/03/12
 *
 *  @brief      Query the DSS service for an image of a field.
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "VOClient.h"
#include "voApps.h"
#include "samp.h"


static double  ra      	= 0.0;		/* default values		*/
static double  dec     	= 0.0;
static double  size    	= 0.25;

static  int   do_samp	= FALSE;	/* broadcast SAMP msg?		*/
static  char *field	= NULL;		/* Input field name		*/
static  char *pos	= NULL;		/* Input position		*/

static char *dss1   = 
		"http://skyview.gsfc.nasa.gov/cgi-bin/vo/sia.pl?survey=dss2&";
static char *dss2   = 
		"http://archive.eso.org/bin/dss_sia/dss.sia?VERSION=1.0&";
static char *dss_url;


static  int 	dss_resolveField (char *field, double *ra, double *dec);
static  int 	dss_resolvePos (char *pos, double *ra, double *dec);
static  double 	dss_getSize (char *arg);
static  int     dss_callService (char *url, double ra, double dec, double size,
			char *ofname, char *format, int maximages);

extern	double	sexa (char *s);


#ifdef USE_RESBUF
static char *resbuf;                    /* result buffer                */
#endif


/*  Task specific option declarations.
 */
int  vodss (int argc, char **argv, size_t *len, void **result);

static int   do_return	= 0;
static Task  self       = {  "vodss",  vodss };
static char  *opts      = "%hF:R:D:P:s:o:S123r";
static struct option long_opts[] = {
        { "field",        1, 0,   'F'},         /* query field name	*/
        { "ra",           1, 0,   'R'},         /* RA of position	*/
        { "dec",          1, 0,   'D'},         /* Dec of position	*/
        { "pos",          1, 0,   'P'},         /* POS position		*/
        { "size",         1, 0,   's'},         /* query size		*/
        { "output",       1, 0,   'o'},         /* set output name	*/
        { "samp",         0, 0,   'S'},         /* broadcast SAMP	*/
        { "dss1",         0, 0,   '1'},         /* use DSS1 img server	*/
        { "dss2",         0, 0,   '2'},         /* use DSS2 img server	*/
        { "help",         0, 0,   'h'},         /* required             */
        { "test",         1, 0,   '%'},         /* required             */
        { "return",       0, 0,   'r'},         /* required             */
        { NULL,           0, 0,    0 }
};

static void Usage (void);
static void Tests (char *input);



/**
 *  Application entry point.
 */
int 
vodss (int argc, char **argv, size_t *reslen, void **result)
{
    char **pargv, optval[SZ_FNAME], ch;
    char  *iname = NULL, *oname = NULL, *dlname = NULL;
    char   tmp[SZ_FNAME], buf[SZ_FNAME];
    int    i=0, status = OK, apos = 0, samp = -1;


    /*  Initialize.
     */
    memset (buf, 0, SZ_FNAME);
    memset (tmp, 0, SZ_FNAME);   
    dss_url = dss1;

    *reslen = 0;
    *result = NULL;


    /*  Parse the argument list.
     */
    pargv = vo_paramInit (argc, argv);
    while ((ch = vo_paramNext (opts,long_opts,argc,pargv,optval,&apos)) != 0) {
	i++;
        if (ch > 0) {
            switch (ch) {
	    case '%':  Tests (optval);			return (OK);
	    case 'h':  Usage ();			return (OK);
	    case 'r':  do_return++;			break;

	    case 'F':  field  = strdup (optval);   	break;

	    case 'R':  if (strchr (optval, (int)':'))
	    		  ra = (15. * (double) sexa (optval));
		       else
	    		  ra = (double) atof (optval);
		       break;
	    case 'D':  if (strchr (optval, (int)':'))
	    		  dec = (double) sexa (optval);
		       else
	    		  dec = (double) atof (optval);
		       break;
	    case 'p':  sscanf (optval, "%lf,%lf", &ra, &dec);
		       break;

	    case 's':  size   = dss_getSize (optval); 	break;
	    case 'o':  oname  = strdup (optval);   	break;

	    case 'S':  do_samp = 1;			break;
	    case '1':  dss_url  = dss1;			break;
	    case '2':  dss_url  = dss2;			break;

	    default:
		fprintf (stderr, "Invalid argument '%c'\n", ch);
		return (ERR);
	    }
	} else {
	    if (isdigit(optval[0]))
		sprintf ((pos = buf), "%s %s", optval, argv[i+1]);
	    else
		field = strdup (optval);
	    break;
	}
    }

    /*  Setup the output name.
     */
    if (oname) {
	dlname = oname;			    /* output name specified	*/
    } else {
	strcpy (tmp, "/tmp/vodssXXXXXX");   /* temp download name	*/
	mktemp (tmp);
	dlname = tmp;
    }


    /* Sanity checks
     */
    if (iname == NULL) iname = strdup ("stdin");
    if (oname == NULL) oname = strdup ("stdout");
    if (strcmp (iname, "-") == 0) { free (iname), iname = strdup ("stdin");  }
    if (strcmp (oname, "-") == 0) { free (oname), oname = strdup ("stdout"); }


    /*  Initialize the VOClient code.  Error messages are printed by the
     *  interface so we just quit if there is a problem.
     */
    if (voc_initVOClient (NULL) == ERR) 
        return (ERR);


    if (field && pos) {
	fprintf (stderr, 
	    "Error: only one of 'field' or 'pos' may be specified.\n");
	return (ERR);
    } else if (field) {
	if (dss_resolveField (field, &ra, &dec) != OK) {
	    fprintf (stderr, "Error: cannot resolve object '%s'\n", field);
	    return (ERR);
	}
    } else if (pos) {
	if (dss_resolvePos (pos, &ra, &dec) != OK) {
	    fprintf (stderr, "Error: cannot convert position '%s'\n", pos);
	    return (ERR);
	}
    }


    /*  Call the DSS service.
     */
    if (dss_callService (dss_url, ra, dec, size, dlname, "fits", 1) == ERR) 
	return (ERR);

    /*  Broadcast the image as a message if requested.
     */
    if (do_samp) {
	if ((samp = sampInit ("vodss", "VOClient Task")) >= 0) {
	    char url[SZ_LINE], cwd[SZ_LINE];

	    samp_setASyncMode (samp);	/* use asynchronous mode	*/
	    sampStartup (samp);		/* register w/ Hub		*/

	    memset (url, 0, SZ_LINE);
	    memset (cwd, 0, SZ_LINE);
	    getcwd (cwd, SZ_LINE);
	    if (tmp[0])
	        sprintf (url, "file://%s", dlname);
	    else
	        sprintf (url, "file://%s/%s", cwd, oname);

            (void) samp_imageLoadFITS (samp, "all", url, "", field);

	    sampShutdown (samp);
	}
    }


    /*  See if we're returning the image.
     */
    if (do_return) {
	vo_setResultFromFile (dlname, reslen, result);
        unlink (dlname);
    }
    if (tmp[0])
        unlink (dlname);


    /*  Clean up and shutdown.
     */
    if (pos)    free (pos);
    if (field)  free (field);
    if (iname)  free (iname);
    if (oname)  free (oname);

    voc_closeVOClient (1);
    vo_paramFree (argc, pargv);

    return (status);
}


/**
 *  USAGE -- Print task help summary.
 */
static void
Usage (void)
{
    fprintf (stderr, "\n  Usage:\n\t"
        "vodss [<opts>] [<field> | <pos>]\n\n"
        "  where\n"
        "       -%%,--test		run unit tests\n"
        "       -h,--help		this message\n"
        "       -r,--return		return result from method\n"
	"       -R,--ra <ra>		RA of position\n"
	"       -D,--dec <dec>		RA of position\n"
	"       -P,--pos <ra>,<dec>	POS position string\n"
 	"       -s,--size <size>        Field size\n"
        "            <size>s            Field size (arcsec)\n"
        "            <size>m            Field size (arcmin)\n"
        "            <size>d            Field size (degrees, default)\n"
        "       -1,--dss1               Get the DSS1 image\n"
        "       -2,--dss2               Get the DSS2 image\n"
        "       -S,--samp               Broadcast a SAMP load message\n"
        "       -o <name>               Save image to named file\n" 
        "       <name>                  Target name to be resolved\n"
        "       <ra> <dec>              Position to query\n"
        "\n"
        "Examples:\n\n"
	"    1)  Print the primary (RA,Dec) columns from a table:\n\n"
	"	    %% votpos test.xml\t\t# un-numbered\n"
	"\n"
	"    2)  Print the primary (RA,Dec) columns to a file:\n\n"
	"	    %% votpos -o pos.txt test.xml\n"
	"\n"
    );
}


/**
 *  Tests -- Task unit tests.
 */
static void
Tests (char *input)
{
   vo_taskTest (self, "--help", NULL);
   vo_taskTest (self, "m51", NULL);
   vo_taskTest (self, "-o", "m51.fits", "m51", NULL);	unlink ("m51.fits");
   vo_taskTest (self, "--ra=202.47", "--dec=47.195", "--samp", NULL);
}



/**
 *  Simple test routine to call a Siap search service and summarize results.
 */
static int
dss_callService (char *svc_url, double ra, double dec, double size,
    char *ofname, char *format, int maximages)
{
    char *acref    = NULL, *fmt = NULL;
    int   i, nrec = 0, recnum = 0;

    DAL	      siap;				/* DAL Connection	 */
    Query     query;				/* query handle		 */
    QResponse qr;                               /* query response handle */
    QRecord   rec;				/* result record handle	 */
    QRAttribute v;				/* dataset attribute	 */
	

    /*  Get a new connection to the named service.
     */
    siap = voc_openSiapConnection (svc_url);    /* open a connection    */

    /*  Form a query.  Here we'll use the one search size we're given for
     *  both the RA,DEC sizes, and specify a null format.
     */
    query = voc_getSiapQuery (siap, ra, dec, size, size, format);


    if (VOAPP_DEBUG) {
        fprintf (stderr, "Executing Query:\n  %s\n\n", 
            voc_getQueryString (query, SIAP_CONN, 0));
    }


    /* Execute the query.
     */
    qr = voc_executeQuery (query);                  /* execute the query    */
    if ((nrec = voc_getRecordCount (qr)) <= 0)
        return (ERR);

    /*  Download the first 'maximages' images.
     */
    for (i=0; i < nrec && i < nrec; i++) {
	rec = voc_getRecord (qr, i);            /* get a row in the table   */

	v = voc_getAttribute (rec, "Format"); 	/* get the right format	    */
	if ((fmt=voc_stringValue(v)) && strcasestr (fmt, format)) {
	    if ((v = voc_getAttribute (rec, "AccessReference"))) {
	        acref = voc_stringValue (v);
	        if ( voc_getDataset (rec, acref, ofname) != OK )
                    return (ERR);

	        if ( ++recnum >= maximages )
	            break;
	    }
	}
    }

    voc_closeConnection (siap);			/* close the siap connection */
    return (OK);
}


/**
 *  DSS_RESOLVEPOS -- Resolve a position string to decimal degrees.
 */
static int
dss_resolvePos (char *pos, double *ra, double *dec)
{
    char *ip = NULL, *s1 = NULL, *s2 = NULL;


    if ( (ip = strchr (pos, (int)' '))) {
	*ip = '\0';
	s1 = pos;
	s2 = ip + 1;

	if (strchr (s1, (int)':'))
	    *ra = (15. * (double) sexa (s1));
	else
	    *ra =  (double) atof (s1);
	*dec = (strchr (s2,(int)':') ? (double) sexa (s2) : (double) atof (s2));

	return (0);
    } else
	return (ERR);

}


/**
 *  DSS_RESOLVEFIELD -- Resolve an object field name to a position in decimal
 *  degrees.
 */
static int
dss_resolveField (char *field, double *ra, double *dec)
{
    Sesame sr;

    sr = voc_nameResolver (field);
    *ra = (double) voc_resolverRA (sr);
    *dec = (double) voc_resolverDEC (sr);

    return (0);
}


/**
 *  DSS_GETSIZE -- Convert an argument size spec into a decimal degree value.
 */
static double
dss_getSize (char *arg)
{
    int  len = strlen (arg);
    int  unit = 'd';
    double  size = (double) 0.0;


    if (isalpha ((int) arg[len-1]) && arg[len-1] != '.') {
	unit = tolower (arg[len-1]);
	if (strchr ("mdsMDS", unit) == NULL) {
	    fprintf (stderr, "Error: Invalid size unit '%c'\n", unit);
	    return (size);
	}
	arg[len-1] = '\0';
    } 

    /*  Convert to decimal degrees for the query.
     */
    switch (unit) {
    case 'd': 	return ( atof (arg)         );
    case 'm': 	return ( atof (arg) / 60.   );
    case 's': 	return ( atof (arg) / 3600. );
    }

    return ((double) 0.0);
}
