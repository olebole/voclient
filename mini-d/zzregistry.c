/**
 */

#include <stdio.h>
#include <stdlib.h>

//#define VOC_DIRECT

#include "VOClient.h"


char	*voc_opts = NULL;
char	*server   = "6200:localhost";

char	*sql   	= NULL;
char	*term  	= "NOAO";

static  void 	keyPrint (RegResult reg, char *field, int index);


int
main (int argc, char *argv[])
{
    RegResult  reg = 0;
    int    i = 0, nres;


    /*  Process command line arguments. 
     */
    term = (argc <= 1 ? term : argv[1]);


    /*  Initialize the VOClient code.  Error messages are printed by the
     *  interface so we just quit if there is a problem.
     */
    if (voc_initVOClient (voc_opts) == ERR) 
        return (0);


    /***********************************************
     **  Test using the voclient daemon
     **********************************************/

    reg = voc_regSearch (NULL, term, 1);

    printf ("search term: '%s' -> %d results \n", 
	term, (nres = voc_resGetCount (reg)) );

    for (i=0; i < nres; i++) {
	printf ("%4d  ---------------\n", i);

	keyPrint (reg, "tags", i);
	keyPrint (reg, "shortName", i);
	keyPrint (reg, "title", i);
	keyPrint (reg, "description", i);
	keyPrint (reg, "publisher", i);
	keyPrint (reg, "waveband", i);
	keyPrint (reg, "identifier", i);
	keyPrint (reg, "updated", i);
	keyPrint (reg, "subject", i);
	keyPrint (reg, "type", i);
	keyPrint (reg, "contentLevel", i);
	keyPrint (reg, "regionOfRegard", i);
	keyPrint (reg, "version", i);
	keyPrint (reg, "resourceID", i);
	keyPrint (reg, "capabilityClass", i);
	keyPrint (reg, "capabilityStandardID", i);
	keyPrint (reg, "capabilityValidationLevel", i);
	keyPrint (reg, "interfaceClass", i);
	keyPrint (reg, "interfaceVersion", i);
	keyPrint (reg, "interfaceRole", i);
	keyPrint (reg, "accessURL", i);
	keyPrint (reg, "maxRadius", i);
	keyPrint (reg, "maxRecords", i);
	keyPrint (reg, "publisherID", i);
	keyPrint (reg, "referenceURL", i);
    }

    (void) voc_closeVOClient (1);
    return (0);
}

static void
keyPrint (RegResult reg, char *field, int index)
{
    if ((strcasecmp (field, "numCapabilities") == 0) ||
	(strcasecmp (field, "numInterfaces") == 0) ||
	(strcasecmp (field, "numStdCapabilities") == 0) ) {
            int  ival = voc_resGetInt (reg, field, index);
            printf ("%12.12s : '%d'\n", field, ival);

    } else if (strcasecmp (field,        "regionOfRegard") == 0) {
        double  dval = voc_resGetFloat (reg, field, index);
        printf ("%12.12s : '%g'\n", field, dval);

    } else {
        char  *s = voc_resGetStr (reg, field, index);
        printf ("%12.12s : '%s'\n", field, s);
        if (s) free (s);
    }
}
