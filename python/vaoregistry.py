"""
a module for basic VO Registry interactions.  

A VO registry is a database of VO resources--data collections and
services--that are available for VO applications.  Typically, it is 
aware of the resources from all over the world.  A registry can find 
relevent data collections and services through search
queries--typically, subject-based.  The registry responds with a list
of records describing matching resources.  With a record in hand, the 
application can use the information in the record to access the 
resource directly.  Most often, the resource is a data service that
can be queried for individual datasets of interest.  

This module provides basic, low-level access to the VAO Registry at 
STScI using (proprietary) VOTable-based services.  In most cases,
the Registry task, with its higher-level features (e.g. result caching
and resource aliases), can be a more convenient interface.  The more  
basic interface provided here allows developers to code their own 
interaction models.  
"""
class RegistrySearch():
    """
    a class for submitting searches to the VAO registry.  
    """

    STScIRegistryURL = ""

    @staticmethod
    def connect(cls, url=None):
        """
        connect to an STScI registry at the given URL
        @param url    the base URL of the STScI registry.  If None, 
                        the standard endpoint will be used.  
        """
        if not url:  url = cls.STScIRegistryURL
        return RegistrySearch(url)
        # Note: it's not clear that the C interface would allow us to
        # override the URL.  

    def __init__(self, url):
        """
        connect to an STScI registry at the given URL
        """
        self.url = url
        # Note: it's not clear that the C interface would allow us to
        # override the URL.  

    def search(self, keywords=None, serviceType=None, 
               bandpass=None, contentLevel=None, sqlpred=None):
        """
        Prepare and execute a registry search of the specified
        keywords. 
 
        A search can be constrained by: 
        - bandpass: Radio, Millimeter, Infrared (IR), Optical, 
                    Ultraviolet (UV),  X-Ray (xray), Gamma-Ray (GR)
        - service type: catalog (SCS), image (SIA), spectra (SSA), 
                        table (Vizier), ResourceType from Registry record
        - content level: ...

        The result will be a RegistryResults instance pointing to the
        first matching in the query results
        """
        pass
    
    def resolve(self, ivoid, asVOResource=False):
        """
        Resolve the identifier against the registry, returning a
        resource record.  
        @param ivoid          the IVOA Identifier of the resource
        @param asVOResource   if True, return the VOResource-formated 
                                XML record; otherwise, a SimpleResource
                                instance is returned.
        """
        # Note: it doesn't look like the C interface provides this.  
        pass

    def createQuery(self):
        """
        create a RegistryQuery object that can be refined or saved
        before submitting.  
        """
        return RegistryQuery(self)


class RegistryQuery():
    """
    a representation of a registry query that can be built up over
    successive method calls and then executed.  An instance is normally
    obtained via a call to RegistrySearch.createQuery()
    """
    
    def __init__(self, registry):
        """
        create the query instance
        """
        self.reg = registry
        self.kw = []          # list of individual keyword phrases
        self.preds = []       # list of SQL predicates
        self.svctype = None
        self.band = None
        self.orKw = True
        self.doSort = True
        self.dalonly = False

    def addKeywords(self, keywords):
        """
        add keywords that should be added to this query.  Keywords 
        are searched against key fields in the registry record.  A
        keyword can in fact be a phrase--a sequence of words; in this
        case the sequence of words must appear verbatim in the record
        for that record to be matched. 
        @param keywords  either a single keyword phrase (as a string) 
                           or a list of keyword phrases to add to the 
                           query.  
        """
        if isinstance(keywords, str):
            keywords = [keywords]
        self.kw.extend(keywords)

    def removeKeywords(self, keywords):
        """
        remove the given keyword or keywords from the query.  A
        keyword can in fact be a phrase--a sequence of words; in this
        case, the phrase will be remove.  
        @param keywords  either a single keyword phrase (as a string) 
                           or a list of keyword phrases to remove from
                           the query.  
        """
        if isinstance(keywords, str):
            keywords = [keywords]
        for kw in keywords:
            self.kw.remove(kw)

    def orKeywords(self, ored):
        """
        set whether keywords are OR-ed or AND-ed together.  When
        the keywords are OR-ed, returned records will match at 
        least one of the keywords.  When they are AND-ed, the 
        records will match all of the keywords provided.  
        @param ored   true, if the keywords should be OR-ed; false,
                        if they should be AND-ed.
        """
        self.orKw = ored

    def willOrKeywords(self):
        """
        set true if the keywords will be OR-ed or AND-ed together
        in the query.  True is returned if the keywords will be 
        OR-ed.  
        """
        return self.orKw

    def getKeywords():
        """
        return the current set of keyword constraints
        """
        return list(self.kw)

    def getKeywordCount():
        """
        return the number of currently set keyword constraints
        """
        return len(self.kw)

    def setServiceTypeConstraint(self, serviceType):
        """
        restrict the results to contain services of the given type.
        @param serviceType   the desired type of service, one of 
                                "catalog", "table", "image", or 
                                "spectra".
        """
        self.svcType = serviceType

    def clearServiceTypeConstraint(self):
        """
        remove any currently set service type constraint.  The query
        then will not be restricted to a particular type of service.
        """
        self.svcType = None

    def setWavebandConstraint(self, band):
        """
        restrict the results to contain resources provides data 
        covering a given waveband
        """
        self.band = band

    def clearWavebandConstraint(self):
        """
        remove any currently set waveband constraint.  The query
        then will not be restricted to a particular type of service.
        """
        self.band = None

    def addPredicate(self, pred):
        """
        add an SQL search predicate to the query.  This predicate should
        be of form supported by STScI VOTable search services.  This 
        predicate will be AND-ed with all other constraints (including
        previously added predicates); that is, this constraint must be
        satisfied in addition to the other constraints to match a 
        particular resource record.
        """
        self.preds.append(pred)

    def removePredicate(self, pred):
        """
        remove the give predicate from the current set of predicate
        constraints.  
        """
        self.preds.remove(pred)

    def clearPredicates(self):
        """
        remove all previously added predicates.
        """
        self.preds = []

    def getPredicates(self):
        """
        return (a copy of) the list of predicate constraints that will 
        be applied to the query.  These will be AND-ed with all other 
        constraints (including previously added predicates); that is, 
        this constraint must be satisfied in addition to the other 
        constraints to match a particular resource record.
        """
        return list(self.preds)

    def execute(self):
        """
        submit the query and return the results as a RegistryResults
        instance.  
        @throws RegistryServiceError   for errors connecting to or 
                    communicating with the service
        @throws RegistryQueryError     if the service responds with 
                    an error, including a query syntax error.  A 
                    syntax error should only occur if the query 
                    query contains non-sensical predicates.
        """
        pass

    def executeRaw(self):
        """
        submit the query and return the raw VOTable XML as a string
        """
        pass

    def getQueryURL(self):
        """
        return the GET URL that will submit the query and return the 
        results as a VOTable
        """
        pass


class RegistryAccessError(Exception):
    """
    a base class for registry access failures
    """
    pass

class RegistryServiceError(RegistryAccessError):
    """
    an exception indicating a failure communicating with a registry 
    service.
    """
    pass

class RegistryQueryError(RegistryAccessError):
    """
    an exception indicating an error by the registry in processing a
    query, including query-syntax errors.
    """
    pass

class RegistryResults():
    """
    an iterable set of results from a registry query.  Each record is
    returned as SimpleResource instance
    """

    def __init__(self):
        pass

    def __iter__(self):
        pass

    def getRecordCount(self):
        """
        return the total number of records returned in this result
        """
        pass

    def meta(self):
        """
        List table metadata.
        """
        # Note: it doesn't look like the C interface provides this 
        #       itself.
        pass

    def attributeNames(self):
        """
        return the names of the available record attributes.
        """
        # Note: it doesn't look like the C interface provides this 
        #       itself.
        pass

    def getAttribute(self, name, index):
        """
        return the value of an attribute from a particular record in 
        the results
        @param name   the name of the attribute
        @param index  the zero-based index of the record
        """
        pass

    def getRecord(self, index):
        """
        return all the attributes of a record with the given index
        as SimpleResource instance
        @param index  the zero-based index of the record
        """
        pass

class RegistryCursor():
    """
    a class for iterating through the result of a registry query
    """

    def getRecordCount(self):
        """
        return the number of records left to access by this result cursor
        """
        pass

    def next(self):
        """
        return the next available resource record as a SimpleResource
        object.  This is equivalent to fetch.
        """
        return fetch()

    def fetch(self):
        """
        return the next available resource record as a SimpleResource
        object
        """
        pass

class SimpleResource(dict):
    """
    a container for the resource attributes returned by a registry query.
    A SimpleResource is a dictionary, so in general, all attributes can 
    be accessed by name via the [] operator, and the attribute names can 
    by returned via the keys() function.  For convenience, it also stores 
    key values as public python attributes; these include:

       title         the title of the resource
       shortName     the resource's short name
       ivoid         the IVOA identifier for the resource
       accessURL     when the resource is a service, the service's access 
                       URL.
    """

    def __init__(self):
        self.title = None
        self.shortName = None
        self.ivoid = None
        self.accessURL = None


    
