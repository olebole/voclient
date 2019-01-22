

class Package:
	'''  A class defining a VO Package
	'''

    # class attributes
    name	= None				# package name
	descr	= None				# description string
	author	= None				# package author
	contact	= None				# contact email address
	iconUrl	= None				# URL to package icon
	version	= None				# package version string

	def __init__ (self, dirs):

	def setPkgDirs (self, dirs):
		''' Set the VO package search path by specifying a list of
		    directories.
		'''

	def getPkgDirs (self):
		''' Get the search path for VO packages.
			Returns:	the list of package directories
		'''

	def scanPkgDirs (self):
		''' Scan the directory search path dirs for available package.  This
			method can be used to force the search for new packages or to
			refresh the package list should the environment change.
		Returns:	a list of available package names.
		'''

	def pkgList (self, pattern):
		''' Get the list of available packages matching a pattern string.
			Returns:	a list of available packages 
		'''

	def taskList (self, pkg, pattern):
		''' Get the list of tasks in the packages which match a name pattern..
			Returns:	a list of available tasks who's name matches
						the pattern string.
		'''

	def pkgAttrs (self, pkg):
		''' Get the attributes for a VO Package.
			Returns:	a dictionary of the package attributes
		'''


class Task:
	'''  A class defining a VO Package Task
	'''

    name		= None			# task name
	descr		= None			# description string
	inParams	= None			# task input parameters
	outParams	= None			# task output parameters

	status		= None			# task return status (OK or ERROR)
	msg			= None			# task return error message
	

	def __init__ (self, dirs):

	def taskAttrs (self, task):
		''' Get the attributes of the named task
			Returns:	a dictionary of the named task attributes
		'''

	def setCallback (pattern, func):
		''' Set a callback function to be run whenever a parameter name
			that matches the pattern is encountered.  Pattern applies only
			to the output parameter set.
			Returns:	nothing
		'''

	def executeSync (self):
		''' Execute the task as a synchronous process
			Returns:	nothing
		'''

	def executeASync (self):
		''' Execute the task as a asynchronous process
			Returns:	nothing
		'''

	def wait (self):
		''' Wait for the exit of an asynchronous execution
			Returns:	status of result
		'''


class Pset 
	''' A parameter set (pset) is a named collection of parameters.  '''

	# class attributes
    name	    = None			# pset name
    pkg		    = None			# package name
    task	    = None			# task name
    description = None			# description string

	def __init__ (self, name, pkg, task, desc):
		'''  '''
         
	def loadPset (self, pkg=None, task=None, saveFile=None):
		''' Load a pset for the named package/task from the specified save
			file.  Is 'saveFile' is None the pset is the pset associated
			with the named 'task' in the named 'pkg'.
			Returns:	the created pset object.  
		'''
         
	def savePset (self, saveFile):
		''' Save the pset to the named file.
			[Do we need to specify some sort of serialization here? A
			 parameter may contain a binary object such as a FITS image,
			 is the saved file just the python 'print' output or is there
			 a core VOClient method to do this?]
			Returns:	nothing
		'''

	def paramList (self, pattern):
		''' Get a list of parameters that match the given pattern.
			Returns:	a list of (name,value) tuples.  
		'''

	def paramSet (self, pattern):                 # Param names and values
		''' Create a pset list for parameters that match the given pattern.  
			This method is used primarily to extract a parameters of specific
		    type from a task result set (e.g. the Registry task might return
			a Resource dict for each service that matches a specific query, 
			this result set might name all such parameters 'result').
			Returns:	the Pset matching the pattern
		'''

	def addParam (self, name, type, description, attrs=None):
		''' Add a parameter to the pset and initialize it with the given
			attributes.
			Returns:	the Param object created
		'''

	def getParam (self, name):
		''' Get the parameter with the given name attribute.  Assumed that
			the 'name' is unique, otherwise the first match is used.
			Returns:	the Param object with the matching name,
		'''


-----------------------------------------------------------------------------
Example Usage:

  - Create a pset from all the "msg" parameters in a result stream

		resultSet = vo.sia ( ..... )
		msgs = resultSet ("msg")

  - Save a pset to the named file:

		pset.savePset ("/tmp/results")

class Param:
	''' A class to represent a Param object.  '''

	# class attributes
	name	 = None			# parameter name
	type	 = None			# parameter type (string/int/real/bool/blob)
	encoding = None			# encoding of param (i.e. mime type)
	desc	 = None			# parameter description string
    
	def __init__ (self, name, type, encoding, desc):
		'''  '''

	def paramAttrs (self):
		''' The the attributes of a Parameter.
		    Returns:	a dictionary fo attributes.  
		'''

	def getValue (self):
		''' Get the value of the parameter (may be a list).  
		    Returns:	the parameter value object
		'''
         
	def setValue (self, val):
		''' Set the value of a parameter object.  
		    Returns:	nothing
		'''

------------------------------------------------------------------------------
Example Usage:


- Create a Param object

	p = Param ("verbose", "bool", None, "Verbose output?")
    p.setValue("foo")


