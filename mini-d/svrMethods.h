/**
 *  SVRMETHODS.H -- Method definitions for the MiniDaemon.
 *
 *  @file       svrMethods.h
 *  @author     Michael Fitzpatrick
 *  @version    April 2013
 *
 *************************************************************************
 */


/**
 *  Method declarations.
 */
extern vocRes_t *sm_newConnection _SVR_METHOD_SIG_	/* DAL  methods	    */
extern vocRes_t *sm_getRawURL _SVR_METHOD_SIG_
extern vocRes_t *sm_removeConnection  _SVR_METHOD_SIG_
extern vocRes_t *sm_getServiceCount _SVR_METHOD_SIG_
extern vocRes_t *sm_addServiceURL _SVR_METHOD_SIG_
extern vocRes_t *sm_getServiceURL _SVR_METHOD_SIG_
extern vocRes_t *sm_getQuery _SVR_METHOD_SIG_
extern vocRes_t *sm_addParameter _SVR_METHOD_SIG_
extern vocRes_t *sm_getQueryString _SVR_METHOD_SIG_
extern vocRes_t *sm_execute _SVR_METHOD_SIG_
extern vocRes_t *sm_getQResponse _SVR_METHOD_SIG_
extern vocRes_t *sm_executeCSV _SVR_METHOD_SIG_
extern vocRes_t *sm_executeTSV _SVR_METHOD_SIG_
extern vocRes_t *sm_executeASCII _SVR_METHOD_SIG_
extern vocRes_t *sm_evecuteVOTable _SVR_METHOD_SIG_
extern vocRes_t *sm_getRecordCount _SVR_METHOD_SIG_
extern vocRes_t *sm_getRecord _SVR_METHOD_SIG_
extern vocRes_t *sm_getAttrCount _SVR_METHOD_SIG_
extern vocRes_t *sm_getFieldAttr _SVR_METHOD_SIG_
extern vocRes_t *sm_getAttrList _SVR_METHOD_SIG_
extern vocRes_t *sm_getAttribute _SVR_METHOD_SIG_
extern vocRes_t *sm_intValue _SVR_METHOD_SIG_
extern vocRes_t *sm_floatValue _SVR_METHOD_SIG_
extern vocRes_t *sm_stringValue _SVR_METHOD_SIG_
extern vocRes_t *sm_getDataset _SVR_METHOD_SIG_

extern vocRes_t *sm_regSearch _SVR_METHOD_SIG_		/* Registry methods */
extern vocRes_t *sm_resSearchBySvc _SVR_METHOD_SIG_
extern vocRes_t *sm_regQuery _SVR_METHOD_SIG_
extern vocRes_t *sm_regAddSearchTerm _SVR_METHOD_SIG_
extern vocRes_t *sm_removeSearchTerm   _SVR_METHOD_SIG_
extern vocRes_t *sm_regConstWaveband _SVR_METHOD_SIG_
extern vocRes_t *sm_regConstSvcType _SVR_METHOD_SIG_
extern vocRes_t *sm_regDALOnly _SVR_METHOD_SIG_
extern vocRes_t *sm_regSortRes _SVR_METHOD_SIG_
extern vocRes_t *sm_regGetSTCount _SVR_METHOD_SIG_
extern vocRes_t *sm_regGetQueryString  _SVR_METHOD_SIG_
extern vocRes_t *sm_regExecute _SVR_METHOD_SIG_
extern vocRes_t *sm_regExecuteRaw _SVR_METHOD_SIG_

extern vocRes_t *sm_resGetCount _SVR_METHOD_SIG_
extern vocRes_t *sm_resGetStr _SVR_METHOD_SIG_
extern vocRes_t *sm_resGetFloat _SVR_METHOD_SIG_
extern vocRes_t *sm_resGetInt _SVR_METHOD_SIG_

extern vocRes_t *sm_nameResolver _SVR_METHOD_SIG_	/* Sesame methods   */
extern vocRes_t *sm_srGetPOS _SVR_METHOD_SIG_
extern vocRes_t *sm_srGetOtype _SVR_METHOD_SIG_
extern vocRes_t *sm_srGetRA _SVR_METHOD_SIG_
extern vocRes_t *sm_srGetRAErr _SVR_METHOD_SIG_
extern vocRes_t *sm_srGetDEC _SVR_METHOD_SIG_
extern vocRes_t *sm_srGetDECErr _SVR_METHOD_SIG_

extern vocRes_t *sm_skybot _SVR_METHOD_SIG_		/* SkyBot methods   */
extern vocRes_t *sm_sbStrAttr _SVR_METHOD_SIG_
extern vocRes_t *sm_sbDblAttr _SVR_METHOD_SIG_
extern vocRes_t *sm_sbNObjs _SVR_METHOD_SIG_

extern vocRes_t *sm_validateObject _SVR_METHOD_SIG_	/* Utility methods  */


svrMethod vocMethods[] = {
  { "newConnection",	   sm_newConnection  	},  /* DAL Query methods  */
  { "getRawURL",	   sm_getRawURL	  	},
  { "removeConnection",    sm_removeConnection 	},
  { "getServiceCount",     sm_getServiceCount	},
  { "addServiceURL",	   sm_addServiceURL	},
  { "getServiceURL",	   sm_getServiceURL	},
  { "getQuery",	 	   sm_getQuery		},
  { "addParameter",	   sm_addParameter	},
  { "getQueryString",	   sm_getQueryString	},
  { "execute",		   sm_execute		},
  { "getQResponse",	   sm_getQResponse	},
  { "executeCSV",	   sm_executeCSV	},
  { "executeTSV",	   sm_executeTSV	},
  { "executeASCII",	   sm_executeASCII	},
  { "executeVOTable",	   sm_evecuteVOTable	},
  { "getRecordCount",	   sm_getRecordCount	},
  { "getRecord",	   sm_getRecord		},
  { "getAttrCount",	   sm_getAttrCount	},
  { "getFieldAttr",	   sm_getFieldAttr	},
  { "getAttrList",	   sm_getAttrList	},
  { "getAttribute",	   sm_getAttribute	},
  { "intValue",	 	   sm_intValue		},
  { "floatValue",	   sm_floatValue	},
  { "stringValue",	   sm_stringValue	},
  { "getDataset",	   sm_getDataset	},

  { "regSearch",	   sm_regSearch		},  /* Registry methods	    */
  { "regSearchBySvc",	   sm_resSearchBySvc	},
  { "regQuery",		   sm_regQuery		},
  { "regAddSearchTerm",	   sm_regAddSearchTerm	},
  { "regRemoveSearchTerm", sm_removeSearchTerm  },
  { "regConstWaveband",	   sm_regConstWaveband	},
  { "regConstSvcType",	   sm_regConstSvcType	},
  { "regDALOnly",	   sm_regDALOnly	},
  { "regSortRes",	   sm_regSortRes	},
  { "regGetSTCount",	   sm_regGetSTCount	},
  { "regGetQueryString",   sm_regGetQueryString },
  { "regExecute",	   sm_regExecute	},
  { "regExecuteRaw",	   sm_regExecuteRaw	},

  { "resGetCount",	   sm_resGetCount	},
  { "resGetString",	   sm_resGetStr		},
  { "resGetFloat",	   sm_resGetFloat	},
  { "resGetInt",	   sm_resGetInt		},

  { "nameResolver",	   sm_nameResolver	},  /* Sesame methods	    */
  { "srGetPOS",		   sm_srGetPOS		},
  { "srGetOtype",	   sm_srGetOtype	},
  { "srGetRA",		   sm_srGetRA		},
  { "srGetRAErr",	   sm_srGetRAErr	},
  { "srGetDEC",	   	   sm_srGetDEC		},
  { "srGetDECErr",	   sm_srGetDECErr	},

  { "skybot",		   sm_skybot		},  /* SkyBot methods	    */
  { "sbStrAttr",	   sm_sbStrAttr		},
  { "sbDblAttr",	   sm_sbDblAttr		},
  { "sbNObjs",	 	   sm_sbNObjs		},

  { "validateObject",	   sm_validateObject	},  /* Utility methods      */

  { NULL,                  NULL        		}
};

