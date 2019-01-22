static Task  self  	= {  "vodata",  vodata,  0,  0,  0  };
static char *opts  	= "%hr1NSACFHIKMO:RTVXab:ce:fgi:mno:p:qr:s:t:uv";
static struct option long_opts[] = {

    { "onefile",     2, 0, '1' },	/* one-file output		*/
    { "numeric",     2, 0, 'N' },	/* numeric output name		*/
    { "simple",      2, 0, 'S' },	/* simple output name		*/

    { "verbmeta",    2, 0, 'M' },	/* verbose metadata		*/

    { "kml",         1, &mf,  5 },	/* req'd arg word		*/
    { "max",         1, &mf,  6 },	/* req'd arg word		*/
    { "web",         1, &mf, 15 },	/* req'd arg word		*/
    { "format",      1, &mf, 18 },	/* req'd arg word		*/
    { "version",     1, &mf, 19 },	/* req'd arg word		*/

    { "proxy",       0, &mf, 32 },	/* no arg word			*/


----------------------------

    { "kmlmax",  1, &mf, 31 },		/* no arg word			*/
    { "kmlsample",  1, &mf, 31 },	/* no arg word			*/
    { "kmlgroup",  1, &mf, 31 },	/* no arg word			*/
    { "kmlnolabel",  1, &mf, 31 },	/* no arg word			*/
    { "kmlnoregion",  1, &mf, 31 },	/* no arg word			*/
    { "kmlnoverbose",  1, &mf, 31 },	/* no arg word			*/

    { NULL,       0, 0,  0 }
};
