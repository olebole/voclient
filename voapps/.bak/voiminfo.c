/*
 *  VOIMINFO -- Get the WCS information for a FITS image.
 *
 *    Usage:
 *		voiminfo [<otps>] image.fits
 *
 *  @file       voiminfo.c
 *  @author     Mike Fitzpatrick
 *  @date       11/03/12
 *
 *  @brief      Get the WCS information for a FITS image.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "votParse.h"			/* keep these in order!		*/
#include "voApps.h"
#include "fitsio.h"



#define	MAX_IMAGES	20480		/* max images to process	*/


static int  do_return   = 0;		/* return result?		*/

/*  Global task declarations.  These should all be defined as 'static' to
 *  avoid namespace collisions.
 */
static int do_all	= 0;		/* print all extensions?	*/
static int do_box	= 0;		/* print box coordinates	*/
static int do_corners	= 0;		/* print image corners		*/
static int do_naxis	= 0;		/* print NAXIS values		*/
static int do_sex	= 0;		/* print sexagesimal values	*/

static int debug	= 0;		/* debug flag			*/
static int verbose	= 1;		/* verbose flag			*/


/*  Task specific option declarations.  Task options are declared using the
 *  getopt_long(3) syntax.
 */
int  voiminfo (int argc, char **argv, size_t *len, void **result);

static Task  self       = {  "voiminfo",  voiminfo };
static char  *opts 	= "%habcdvnso:r";
static struct option long_opts[] = {
        { "test",         2, 0,   '%'},		/* --test is std	*/
        { "help",         2, 0,   'h'},		/* --help is std	*/
        { "return",       2, 0,   'r'},		/* --return is std	*/
        { "debug",        2, 0,   'd'},		/* debug flag 		*/
        { "verbose",      2, 0,   'v'},		/* verbose flag 	*/

        { "all",          2, 0,   'a'},		/* print all extns	*/
        { "box",     	  2, 0,   'b'},		/* print box values 	*/
        { "naxes",     	  2, 0,   'n'},		/* print NEXIS values 	*/
        { "corners",      2, 0,   'c'},		/* print image corner 	*/
        { "sex",          2, 0,   's'},		/* sexagesimal values	*/
        { "output",       1, 0,   'o'},		/* output filename	*/
        { NULL,           0, 0,    0 }
};


/**
 *  Image information structure.  Note that although we can report 3-D
 *  images, we're really only setup to deal with equatorial sky coord
 *  systems.
 */
typedef struct {
    char    imname[SZ_PATH];			/* full image name	*/
    char    nextend;				/* number of extensions */

    int	    is_image;				/* is it an image?	*/
    int	    is_table;				/* is it an image?	*/
    int	    has_wcs;				/* image has wcs	*/

    int	    naxis;				/* number of axes 	*/
    int	    naxes[3];				/* axis dimensions	*/
    double  xc[4], yc[4];			/* center position	*/
    double  cx, cy;				/* center position	*/
    double  lx, ly;				/* lower-left corner	*/
    double  ux, uy;				/* upper-right corner	*/

    double  xrval, yrval;			/* CRVAL values		*/
    double  xrpix, yrpix;			/* CRPIX values		*/
    double  width, height;			/* image width/height	*/
    double  radius;				/* cone radius		*/
    double  rotang;				/* rotation angle	*/
} iminfo, *iminfoP;



static void Usage (void);
static void Tests (char *input);

static char   *fmt (double pos, int is_ra);


/*  Public procedures that can be used in other applications.
 */
int  voc_imgInfo (char *name, iminfo *info);
int  voc_imgNaxes (char *name, int *nx, int *ny, int *nz);
int  voc_imgBox (char *name, double *lx, double *ly, double *ux, double *uy, 
			double *rotangle);
int  voc_imgCone (char *name, double *ra, double *dec, double *radius);
int  voc_imgCorners (char *name, double *x, double *y);



/**
 *  Application entry point.
 */
int
voiminfo (int argc, char **argv, size_t *reslen, void **result)
{
    /*  These declarations are required for the VOApps param interface.
     */
    char **pargv, optval[SZ_FNAME], resbuf[SZ_LINE];

    /*  These declarations are specific to the task.
     */
    char  *oname = NULL, *imlist[MAX_IMAGES];
    char   str[SZ_FNAME], *nimlist[MAX_IMAGES];
    int    i, ch = 0, status = OK, pos = 0, nfiles = 0, narg = 0;;
    int    stat, extnum = 0, imnum = 0;
    size_t maxlen = SZ_LINE;
    FILE  *fd = (FILE *) NULL;


    /* Initialize result object	whether we return an object or not.
     */
    *reslen = 0;	
    *result = NULL;
    memset (imlist,  0, MAX_IMAGES);
    memset (nimlist, 0, MAX_IMAGES);


    /*  Parse the argument list.  The use of vo_paramInit() is required to
     *  rewrite the argv[] strings in a way vo_paramNext() can be used to
     *  parse them.  The programmatic interface allows "param=value" to
     *  be passed in, but the getopt_long() interface requires these to
     *  be written as "--param=value" so they are not confused with 
     *  positional parameters (i.e. any param w/out a leading '-').
     */
    pargv = vo_paramInit (argc, argv);
    while ((ch = vo_paramNext(opts,long_opts,argc,pargv,optval,&pos)) != 0) {
        if (ch > 0) {
	    /*  If the 'ch' value is > 0 we are parsing a single letter
	     *  flag as defined in the 'opts string.
	     */
	    switch (ch) {
	    case '%':  Tests (optval);			return (OK);
	    case 'h':  Usage ();			return (OK);
	    case 'r':  do_return=1;	    	    	break;
	    case 'd':  debug++;				break;
	    case 'v':  verbose++;			break;

	    case 'a':  do_all++;			break;
	    case 'b':  do_box++;			break;
	    case 'c':  do_corners++;			break;
	    case 's':  do_sex++;			break;
	    case 'n':  do_naxis++;			break;
	    case 'o':  oname = strdup (optval);		break;
	    default:
		fprintf (stderr, "Invalid option '%s'\n", optval);
		return (1);
	    }

	} else {
	    /*  This code processes the positional arguments.  The 'optval'
	     *  string contains the value but since this string is
	     *  overwritten w/ each arch we need to make a copy (and must
	     *  remember to free it later.
	     */
	    imlist[nfiles++] = strdup (optval);
	    narg++;
	}

	if (narg > MAX_IMAGES) {
            fprintf (stderr, "ERROR: Too many images to process\n");
            return (1);
	}
    }


    /*  Sanity checks.  Tasks should validate input and accept stdin/stdout
     *  where it makes sense.
     */
    if (imlist[0] == NULL || strcmp (imlist[0], "-") == 0) { 
	free (imlist[0]);
	imlist[0] = strdup ("stdin");  
	nfiles = 1;
    }
    if (do_all && strcasecmp (imlist[0], "stdin") == 0) {
	fprintf (stderr, "Option not supported with standard input\n");
	return (ERR);
    }
    if (oname == NULL) oname = strdup ("stdout");
    if (strcmp (oname, "-") == 0) { free (oname), oname = strdup ("stdout"); }


    /*  Open the output file.
     */
    if (strcmp (oname, "stdout") != 0) {
	if ((fd = fopen (oname, "w+")) == (FILE *) NULL) {
	    fprintf (stderr, "Cannot open output file '%s'\n", oname);
	    return (ERR);
	}
    } else
	fd = stdout;


    /**
     *  Optionally expand the image list to include extension numbers.
     */
    if (do_all) {
        fitsfile *fptr;
	int   done = 0;

	imnum = 0;
	for (i=0; i < nfiles; i++) {
	    for (done=0; !done; ) {
	        memset (str, 0, SZ_FNAME);
	        for (extnum=0; ; extnum++) {
	            sprintf (str, "%s[%d]", imlist[i], extnum);
                    if ((stat = ffopen (&fptr, str, READWRITE, &status)) > 0) {
			done++;
			break;
		    }
		    ffclos (fptr, &status);
	            nimlist[imnum++] = strdup (str);
	        }
            }
	}

    } else {
	for (i=0; i < nfiles; i++)
	    nimlist[imnum++] = imlist[i];
    }

    nfiles = imnum;
    status = OK;


    /**
     *  Main body of task
     */
    if (do_naxis) {
	int  nx = 0, ny = 0, nz = 0;

	for (i=0; i < nfiles; i++) {
	    if (debug)
		fprintf (stderr, "doing '%s'\n", nimlist[i]);
	    if (voc_imgNaxes (nimlist[i], &nx, &ny, &nz) == OK) {
		if (*result == NULL)
	            *result = calloc (1, maxlen);

		memset (resbuf, 0, SZ_LINE);
		if (nz)
                    sprintf (resbuf, "%s\t%d %d %d\n", nimlist[i], nx, ny, nz);
		else
                    sprintf (resbuf, "%s\t%d %d\n", nimlist[i], nx, ny);

	        if (do_return)
		    vo_appendResultFromString (resbuf, reslen, result, &maxlen);
	        else
                    fprintf (fd, "%s", resbuf);

	    } else {
		if (debug) printf ("Error getting image size\n");
		;	/*  FIXME  */
	    }
	}

    } else if (do_box) {
	double lx, ly, ux, uy, rot;

	for (i=0; i < nfiles; i++) {
	    if (debug)
		fprintf (stderr, "doing '%s'\n", nimlist[i]);
	    if (voc_imgBox (nimlist[i], &lx, &ly, &ux, &uy, &rot) == OK) {
		if (*result == NULL)
	            *result = calloc (1, maxlen);

		memset (resbuf, 0, SZ_LINE);
                sprintf (resbuf, "%s\t%s %s\t%s %s\t%s\n", nimlist[i], 
		    fmt(lx,1), fmt(ly,0), fmt(ux,1), fmt(uy,0), fmt(rot,0));

	        if (do_return)
		    vo_appendResultFromString (resbuf, reslen, result, &maxlen);
	        else
                    fprintf (fd, "%s", resbuf);

	    } else {
		if (debug) printf ("Error getting Box coords\n");
		;	/*  FIXME  */
	    }
	}

    } else if (do_corners) {
	double xcorners[4], ycorners[4];

	for (i=0; i < nfiles; i++) {
	    if (debug)
		fprintf (stderr, "doing '%s'\n", nimlist[i]);
	    if (voc_imgCorners (nimlist[i], xcorners, ycorners) == OK) {
		if (*result == NULL)
	            *result = calloc (1, maxlen);

		memset (resbuf, 0, SZ_LINE);
                sprintf (resbuf, "%s\t%s %s\t%s %s\t%s %s\t%s %s\n", 
			nimlist[i], 
			fmt (xcorners[0],1), fmt (ycorners[0],0),
			fmt (xcorners[1],1), fmt (ycorners[1],0),
			fmt (xcorners[2],1), fmt (ycorners[2],0),
			fmt (xcorners[3],1), fmt (ycorners[3],0));

	        if (do_return)
		    vo_appendResultFromString (resbuf, reslen, result, &maxlen);
	        else
                    fprintf (fd, "%s", resbuf);

	    } else {
		if (debug) printf ("Error getting Corner coords\n");
		;	/*  FIXME  */
	    }
	}

    } else {
	double  ra, dec, rad;

	for (i=0; i < nfiles; i++) {
	    if (debug)
		fprintf (stderr, "doing '%s'\n", nimlist[i]);
            if (voc_imgCone (nimlist[i], &ra, &dec, &rad) == OK) {

		if (*result == NULL)
	            *result = calloc (1, maxlen);

		memset (resbuf, 0, SZ_LINE);
                sprintf (resbuf, "%s\t%s\t%s\t%s\n", nimlist[i], 
		    fmt (ra,1), fmt (dec,0), fmt (rad,0));

	        if (do_return)
		    vo_appendResultFromString (resbuf, reslen, result, &maxlen);
	        else
                    fprintf (fd, "%s", resbuf);

	    } else {
		if (debug) printf ("Error getting Cone coords\n");
		;	/*  FIXME  */
	    }
	}
    }


    /*  Clean up.  Rememebr to free whatever pointers were created when
     *  parsing arguments.
     */
    for (i=0; i < MAX_IMAGES; i++) {
        if (imlist[i])  free (imlist[i]);
        if (nimlist[i]) 
	    free (nimlist[i]);
	else
	    break;
    }
    if (oname) free (oname);
    if (fd != stdout)
	fclose (fd);

    vo_paramFree (argc, pargv);

    return (status);	/* status must be OK or ERR (i.e. 0 or 1)     	*/
}


/**
 *  USAGE -- Print task help summary.
 */
static void
Usage (void)
{
    fprintf (stderr, "\n  Usage:\n\t"
        "voiminfo [<opts>] votable.xml\n\n"
        "  where\n"
        "       -%%,--test		run unit tests\n"
        "       -h,--help		this message\n"
        "       -o,--output=<file>	output file\n"
        "       -r,--return		return result from method\n"
        "       -d,--debug		debug flag\n"
        "       -v,--verbose		verbose flag\n"
	"\n"
        "	-a,--all   		print all extns\n"
        "	-b,--box     		print box values\n"
        "	-n,--naxes     		print NEXIS values\n"
        "	-c,--corners   		print image corner\n"
        "	-s,--sex       		sexagesimal values\n"
        "	-o,--output    		output filename\n"
	"\n"
	"\n"
 	"  Examples:\n\n"
	"    1)  First example\n\n"
	"	    %% voiminfo test.xml\n"
	"	    %% voiminfo -n test.xml\n"
	"	    %% cat test.xml | voiminfo\n"
	"\n"
	"    2)  Second example\n\n"
	"	    %% voiminfo -o pos.txt test.xml\n"
	"\n"
    );
}


/**
 *  Tests -- Task unit tests.
 */
static void
Tests (char *input)
{
   /*  First argument must always be the 'self' variable, the last must 
    *  always be a NULL to terminate the cmd args.
    */
   vo_taskTest (self, "--help", NULL);
}




/***************************************************************************/
/****  			    Public Procedures				****/
/***************************************************************************/

/**
 *  VOC_IMGINFO -- Get the image corner positions as a simple 4-element
 *  array of (x,y) values.  Positions start in the lower-left corner and
 *  move in a clockwise direction.
 */
int
voc_imgInfo (char *name, iminfo *info)
{
    fitsfile *fptr;

    double  xrval=0.0, yrval=0.0, xrpix=0.0, yrpix=0.0, xpix=0.0, ypix=0.0;
    double  xinc=0.0, yinc=0.0, rot=0.0, cx=0.0, cy=0.0, lx=0.0, ly=0.0;
    long    naxes[3], pcount, gcount;
    int     status=SKIP_TABLE, extend, simple, naxis, bitpix;
    char    ctype[5];


    /*  Open the FITS file.
     */
    memset (info, 0, sizeof (info));
    if (ffopen (&fptr, name, READWRITE, &status) > 0) {
	fprintf (stderr, "Error: open status = %d\n", status);
	exit (0);
    }
    strncpy (info->imname, name, strlen (name));

    /*  Get the primary header keywords.
     */
    ffghpr (fptr, 99, &simple, &bitpix, &naxis, naxes, &pcount,
           &gcount, &extend, &status);
    info->naxes[0] = naxes[0];
    info->naxes[1] = naxes[1];
    info->naxes[2] = naxes[2];

    /*  Get the header WCS keywords.
     */
    ffgics (fptr, &xrval, &yrval, &xrpix,
               &yrpix, &xinc, &yinc, &rot, ctype, &status);
    if (status == 233) {			/* NOT_IMAGE	*/
	info->has_wcs = info->is_image = 0;
	info->is_table = 0;	/* FIXME -- need to verify it's a table */
	return (ERR);
    }
    if (status != 506 && status != 0) {
	info->has_wcs = 0;
	info->is_table = info->is_image = 0;
        fprintf (stderr, "Read WCS keywords with ffgics status = %d\n",status);
    }

    info->xrval = xrval;
    info->yrval = yrval;
    info->xrpix = xrpix;
    info->yrpix = yrpix;
    info->rotang = rot;
    info->has_wcs = 1;
    info->is_image = 1;

    xpix = (double) 0.5;			/*   Lower-left		*/
    ypix = (double) 0.5;
    status = 0;
    ffwldp (xpix, ypix, xrval, yrval, xrpix, yrpix, xinc, yinc, rot, ctype,
           &info->xc[0], &info->yc[0], &status);

    xpix = (double) 0.5;			/*   Upper-left		*/
    ypix = (double) naxes[1] - 0.5;
    status = 0;
    ffwldp (xpix, ypix, xrval, yrval, xrpix, yrpix, xinc, yinc, rot, ctype,
           &info->xc[1], &info->yc[1], &status);

    xpix = (double) naxes[1] - 0.5;		/*   Upper-right	*/
    ypix = (double) naxes[1] - 0.5;
    status = 0;
    ffwldp (xpix, ypix, xrval, yrval, xrpix, yrpix, xinc, yinc, rot, ctype,
           &info->xc[2], &info->yc[2], &status);

    xpix = (double) naxes[1] - 0.5;		/*   Lower-right	*/
    ypix = (double) 0.5;
    status = 0;
    ffwldp (xpix, ypix, xrval, yrval, xrpix, yrpix, xinc, yinc, rot, ctype,
           &info->xc[3], &info->yc[3], &status);


    /*  Get center and cone radius.
     */
    status = 0;
    xpix = (double) info->naxes[0] / 2.;
    ypix = (double) info->naxes[1] / 2.;
    ffwldp (xpix, ypix, xrval, yrval, xrpix, yrpix, xinc, yinc, rot, ctype,
           &cx, &cy, &status);
    info->cx = cx;
    info->cx = cy;
    info->radius = sqrt ((cx - lx) * (cx - lx) + (cy - ly) * (cy - ly));


    /*  Close the image.
     */
    if (ffclos (fptr, &status) > 0) {
	fprintf (stderr, "Error: close status = %d\n", status);
	return (-1);
    }

    return (OK);
}


/**
 *  VOC_IMGNAXES -- Get the image corner positions as a simple 4-element
 *  array of (x,y) values.  Positions start in the lower-left corner and
 *  move in a clockwise direction.
 */
int
voc_imgNaxes (char *name, int *nx, int *ny, int *nz)
{
    fitsfile *fptr;
    long    naxes[3] = {0, 0, 0}, pcount, gcount;
    int     status=SKIP_TABLE, extend, simple, naxis, bitpix;


    /*  Open the FITS file.
     */
    if (ffopen (&fptr, name, READWRITE, &status) > 0) {
	fprintf (stderr, "Error: open status = %d\n", status);
	exit (0);
    }

    /*  Get the primary header keywords.
     */
    ffghpr (fptr, 99, &simple, &bitpix, &naxis, naxes, &pcount,
           &gcount, &extend, &status);

    if (debug)
	fprintf (stderr, "%s:  pcount=%d  gcount=%d  extend=%d\n", 
	    name, (int) pcount, (int) gcount, (int) extend);

    *nx = naxes[0];
    *ny = naxes[1];
    *nz = naxes[2];

    /*  Close the image.
     */
    if (ffclos (fptr, &status) > 0) {
	fprintf (stderr, "Error: close status = %d\n", status);
	return (-1);
    }

    return (OK);
}


/**
 *  VOC_IMGBOX -- Get the image box corners and rotation angle.
 */
int
voc_imgBox (char *name, double *lx, double *ly, double *ux, double *uy, 
		double *rotangle)
{
    fitsfile *fptr;

    double  xrval=0.0, yrval=0.0, xrpix=0.0, yrpix=0.0, xpix=0.0, ypix=0.0;
    double  xinc=0.0, yinc=0.0, rot=0.0;
    long    naxes[3], pcount, gcount;
    int     status=SKIP_TABLE, extend, simple, naxis, bitpix;
    char    ctype[5];


    /*  Open the FITS file.
     */
    if (ffopen (&fptr, name, READWRITE, &status) > 0) {
	fprintf (stderr, "Error: open status = %d\n", status);
	return (ERR);
    }

    /*  Get the primary header keywords.
     */
    ffghpr (fptr, 99, &simple, &bitpix, &naxis, naxes, &pcount,
           &gcount, &extend, &status);

    /*  Get the header WCS keywords.
     */
    ffgics (fptr, &xrval, &yrval, &xrpix,
               &yrpix, &xinc, &yinc, &rot, ctype, &status);
    if (status == 233) 			/* NOT_IMAGE	*/
	return (ERR);
    if (status != 506 && status != 0)
        fprintf (stderr, "Read WCS keywords with ffgics status = %d\n",status);
    *rotangle = rot;


    xpix = 0.5;
    ypix = 0.5;
    status = 0;
    if (ffwldp (xpix, ypix, xrval, yrval, xrpix, yrpix, xinc, yinc, rot, ctype,
               lx, ly, &status) > 0) {
	return (ERR);

    } else {
        status = 0;
        xpix = (double) naxes[0];
        ypix = (double) naxes[1];
        ffwldp (xpix, ypix, xrval, yrval, xrpix, yrpix, xinc, yinc, rot, ctype,
           ux, uy, &status);
    }

    /*  Close the image.
     */
    if (ffclos (fptr, &status) > 0) {
	fprintf (stderr, "Error: close status = %d\n", status);
	return (-1);
    }

    return (OK);
}


/**
 *  VOC_IMGCONE -- Get the image center and radius.
 */
int
voc_imgCone (char *name, double *ra, double *dec, double *rad)
{
    fitsfile *fptr;

    double  xrval=0.0, yrval=0.0, xrpix=0.0, yrpix=0.0, xpix=0.0, ypix=0.0;
    double  xinc=0.0, yinc=0.0, rot=0.0;
    double  ll_x=0.0, ll_y=0.0, c_x=0.0, c_y=0.0;
    long    naxes[3], pcount, gcount;
    int     status=SKIP_TABLE, extend, simple, naxis, bitpix;
    char    ctype[5];


    /*  Open the FITS file.
     */
    if (ffopen (&fptr, name, READWRITE, &status) > 0) {
	fprintf (stderr, "Error: open status = %d\n", status);
	return (ERR);
    }

    /*  Get the primary header keywords.
     */
    ffghpr (fptr, 99, &simple, &bitpix, &naxis, naxes, &pcount,
           &gcount, &extend, &status);

    /*  Get the header WCS keywords.
     */
    ffgics (fptr, &xrval, &yrval, &xrpix,
               &yrpix, &xinc, &yinc, &rot, ctype, &status);
    if (status == 233)			/* NOT_IMAGE	*/
	return (ERR);
    if (status != 506 && status != 0)
        fprintf (stderr, "Read WCS keywords with ffgics status = %d\n",status);

    xpix = 0.5;
    ypix = 0.5;
    status = 0;
    if (ffwldp (xpix, ypix, xrval, yrval, xrpix, yrpix, xinc, yinc, rot, ctype,
               &ll_x, &ll_y, &status) > 0) {
	return (ERR);

    } else {
        status = 0;
        xpix = (double) naxes[0] / 2.;
        ypix = (double) naxes[1] / 2.;
        ffwldp (xpix, ypix, xrval, yrval, xrpix, yrpix, xinc, yinc, rot, ctype,
           &c_x, &c_y, &status);
    }

    *ra   = c_x;
    *dec  = c_y;
    *rad  = sqrt ((c_x - ll_x)*(c_x - ll_x) + (c_y - ll_y)*(c_y - ll_y));

    /*  Close the image.
     */
    if (ffclos (fptr, &status) > 0) {
	fprintf (stderr, "Error: close status = %d\n", status);
	return (ERR);
    }

    return (OK);
}


/**
 *  VOC_IMGCORNERS -- Get the image corner positions as a simple 4-element
 *  array of (x,y) values.  Positions start in the lower-left corner and
 *  move in a clockwise direction.
 */
int
voc_imgCorners (char *name, double *x, double *y)
{
    fitsfile *fptr;

    double  xrval=0.0, yrval=0.0, xrpix=0.0, yrpix=0.0, xpix=0.0, ypix=0.0;
    double  xinc=0.0, yinc=0.0, rot=0.0;
    long    naxes[3], pcount, gcount;
    int     status=SKIP_TABLE, extend, simple, naxis, bitpix;
    char    ctype[5];


    /*  Open the FITS file.
     */
    if (ffopen (&fptr, name, READWRITE, &status) > 0) {
	fprintf (stderr, "Error: open status = %d\n", status);
	exit (0);
    }

    /*  Get the primary header keywords.
     */
    ffghpr (fptr, 99, &simple, &bitpix, &naxis, naxes, &pcount,
           &gcount, &extend, &status);

    /*  Get the header WCS keywords.
     */
    ffgics (fptr, &xrval, &yrval, &xrpix,
               &yrpix, &xinc, &yinc, &rot, ctype, &status);
    if (status == 233)			/* NOT_IMAGE	*/
	return (ERR);
    if (status != 506 && status != 0)
        fprintf (stderr, "Read WCS keywords with ffgics status = %d\n",status);


    xpix = (double) 0.5;			/*   Lower-left		*/
    ypix = (double) 0.5;
    status = 0;
    ffwldp (xpix, ypix, xrval, yrval, xrpix, yrpix, xinc, yinc, rot, ctype,
           &x[0], &y[0], &status);

    xpix = (double) 0.5;			/*   Upper-left		*/
    ypix = (double) naxes[1] - 0.5;
    status = 0;
    ffwldp (xpix, ypix, xrval, yrval, xrpix, yrpix, xinc, yinc, rot, ctype,
           &x[1], &y[1], &status);

    xpix = (double) naxes[1] - 0.5;		/*   Upper-right	*/
    ypix = (double) naxes[1] - 0.5;
    status = 0;
    ffwldp (xpix, ypix, xrval, yrval, xrpix, yrpix, xinc, yinc, rot, ctype,
           &x[2], &y[2], &status);

    xpix = (double) naxes[1] - 0.5;		/*   Lower-right	*/
    ypix = (double) 0.5;
    status = 0;
    ffwldp (xpix, ypix, xrval, yrval, xrpix, yrpix, xinc, yinc, rot, ctype,
           &x[3], &y[3], &status);


    /*  Close the image.
     */
    if (ffclos (fptr, &status) > 0) {
	fprintf (stderr, "Error: close status = %d\n", status);
	return (-1);
    }

    return (OK);
}


/**
 *  FMT -- Output formatter.
 */
static char *
fmt (double pos, int is_ra)
{
    static char strbuf[10][SZ_LINE];
    static int  bufnum = 0;

    bufnum = (bufnum + 1) % 10;
    memset (strbuf[bufnum], 0, SZ_LINE);

    pos = (pos < 0.0 ? -pos : pos);

    if (do_sex) {
        int   d, m;
        float s, frac;
        char  sign = (pos < 0.0 ? '-' : ' ');

        pos = (is_ra ? (pos / 15.0) : pos);
        d = (int) pos;
        frac = (pos - d);
        m = frac * 60.0;
        s = ((frac * 60.0) - m) * 60.0;

        sprintf (strbuf[bufnum], "%c%02d:%02d:%04.1f", sign, d, m, s);

    } else
        /*sprintf (strbuf[bufnum], "%f", (is_ra ? (pos * 15.0) : pos)); */
        sprintf (strbuf[bufnum], "%f", pos);

    return (strbuf[bufnum]);
}


