/**
 */

#include <stdio.h>
#include <stdlib.h>
#include "VOClient.h"


char	*target  = "m31";

int main (int argc, char *argv[])
{
    Sesame  sr = 0;

    /*  Process command line arguments. 
     */
    target = (argc <= 1 ? target : argv[1]);


    /***********************************************
     **  Test using the voclient daemon
     **********************************************/

    /*  Now call the Resolver Service and summarize the results.   We'll 
     *  let the interface initialize the VO Client server and simply call
     *  the procedure we need.
     */
    sr = voc_nameResolver (target);

    printf ("%s: %f (%.2f)  %f (%.2f) (%s) '%s'\n", 
	target, 
	voc_resolverRA (sr), voc_resolverRAErr (sr), 
	voc_resolverDEC (sr), voc_resolverDECErr (sr), 
	voc_resolverOtype (sr), voc_resolverPos (sr));


    /***********************************************
     **  Test using the direct interface
     **********************************************/

    sr = cvo_nameResolver (target);

    printf ("%s: %f (%.2f)  %f (%.2f) (%s) '%s'\n", 
	target, 
	cvo_resolverRA (sr), cvo_resolverRAErr (sr), 
	cvo_resolverDEC (sr), cvo_resolverDECErr (sr), 
	cvo_resolverOtype (sr), cvo_resolverPos (sr));



    return (0);
}
