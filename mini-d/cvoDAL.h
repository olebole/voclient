
char *cvo_coneCaller (char *url, double ra, double dec, double sr, int otype);
int   cvo_coneCallerToFile (char *url, double ra, double dec, double sr, 
	int otype, char *file);

char *cvo_siapCaller (char *url, double ra, double dec, double rsize, 
	double dsize, char *fmt, int otype);
int   cvo_siapCallerToFile (char *url, double ra, double dec, double rsize, 
	double dsize, char *fmt, int otype, char *file);


char *cvo_ssapCaller (char *url, double ra, double dec, double size, 
	char *band, char *time, char *fmt, int otype);
int   cvo_ssapCallerToFile (char *url, double ra, double dec, double size, 
	char *band, char *time, char *fmt, int otype, char *file);

char *cvo_getRawURL (char *url, int *nbytes);

DAL   cvo_openConnection (char *service_url, int type, ...);
DAL   cvo_openConeConnection (char *service_url, ...);
DAL   cvo_openSiapConnection (char *service_url, ...);
DAL   cvo_openSsapConnection (char *service_url, ...);


void  cvo_closeConnection (DAL dal);
int   cvo_getServiceCount (DAL dal);
void  cvo_addServiceURL (DAL dal, char *service_url);
char *cvo_getServiceURL (DAL dal, int index);

Query cvo_getQuery (DAL dal, int type);
Query cvo_getConeQuery (DAL dal, double ra, double dec, double sr);
Query cvo_getSiapQuery (DAL dal, double ra, double dec, double ra_size, 
	double dec_size, char *format);

Query cvo_getSsapQuery (DAL dal, double ra, double dec, double size, 
	char *band, char *tim, char *format);


int   cvo_addIntParam (Query query, char *name, int ival);
int   cvo_addFloatParam (Query query, char *name, double dval);
int   cvo_addStringParam (Query query, char *name, char *str);

char *cvo_getQueryString (Query query, int type, int index);


QResponse cvo_executeQuery (Query query);
QResponse cvo_getQueryResponse (Query query);

char *cvo_executeCSV (Query query);
char *cvo_executeTSV (Query query);
char *cvo_executeASCII (Query query);
char *cvo_executeVOTable (Query query);
int   cvo_executeQueryAs (Query query, char *fname, int type);

char *cvo_getErrMsg ();

int   cvo_getRecordCount (QResponse qr);


/***************************************************************************
** Access by dataset attribute:
*/
QRecord cvo_getRecord (QResponse qr, int recnum);
int   cvo_getAttrCount (QRecord rec);
char *cvo_getFieldAttr (QResponse qr, int index, char *attr);
char *cvo_getAttrList (QRecord rec);


QRAttribute cvo_getAttribute (QRecord rec, char *attrname);

void   cvo_setIntAttr (QRecord rec, char *attrname, int ival);
void   cvo_setFloatAttr (QRecord rec, char *attrname, double dval);
void   cvo_setStringAttr (QRecord rec, char *attrname, char *str);

int    cvo_getIntAttr (QRecord rec, char *attrname);
double cvo_getFloatAttr (QRecord rec, char *attrname);
char  *cvo_getStringAttr (QRecord rec, char *attrname);

int    cvo_intValue (QRAttribute v);
double cvo_floatValue (QRAttribute v);
char  *cvo_stringValue (QRAttribute v);

int    cvo_getDataset (QRecord rec, char *acref, char *fname);


