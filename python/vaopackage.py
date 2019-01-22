"""
The VAO Package and Tasking Interfaces.

The tasking interface defines the python-side classes needed to manage and
execute host processes that implement the "VAO Package" binary interface.
These classes load the package (i.e. execute the binary as a connected
processes and interogate it for a list of available tasks).  The VOTask
interface is used to set the input parameters, execute the task and process
the result parameter set (PSet).

The VOPSet interface manages a collection of VOParam parameter objects:
input parameters will typically define options for the host task (e.g.
input file, processing options, etc), while output parameters may contain
arbitrary objects returned by the task (e.g. a FITS file or VOTable) or
strings which make up the stdout/stderr streams of the task.  A VOParam has
attributes needed to describe the type of parameter, by selecting
parameters of the same name from a PSet (e.g. 'msgs') an application can
process multiple output objects as needed.

A 'task' is a computational component which can be executed as a process by
the host operating system (or a related environment such as a cluster).
Tasks may be written in any language so long as the defined tasking
interface is observed.  The 'package' refered to here should not be
confused with a Python package.  A Python package is an element of Python,
a 'package' as referred to here is a collection of tasks and their
associated metadata.

"""


# Module Globals and Functions


def setPkgDirs (dirs):
	''' Set the VO package search path as an absolute list of directories.
		@param dirs		list of directory to use as search path

		Returns:	Nothing
	'''
	vopkg_path = dirs
        
def addPkgDir (dir):
	'''	Append the given directory to the package search path.
		@param dir		the directory to add to the search path

		Returns:	Nothing
	'''
	vopkg_path.append(dir)

def getPkgDirs ():
	''' Get the VO package search path as a list of directories.

		Returns:	Nothing
	'''
	return (vopkg_path)

def pkgList (pattern=None):
	'''	Get the list of available packages.
		@param pattern		the package name pattern to match

		Returns:	A list of tuples containing the package/task names is
					constructed from all valid packages in the search path.
	'''
	pass

def scan ():
	'''	Force a re-scan of the package search path for available package
		files.   Each directory in the path is searched for a ".vop" file
		indicating a VO package executable, this file is then interrogated
		for a list of tasks.  This allows new package files to be installed
		(or removed) as the module is running.

		Returns:	Nothing
	'''
	pass

def loadPackage (name, file=None):
	'''	Load the package.  Effectively all this does is create a VOPackage
		object from the metadata for the named package, this requires
		that we start the package binary to get the metadata information.
		If a 'file' parameter is specified we open that file regardless of
		the name, allowing any VOPackage file to be used dynamically.

		When a package is loaded, bindings for each of the tasks are generated
		automatically in order to allow direct access to the task.  As an
		example:

			>>> vop = loadPackage ("vo-cli")	# from search path
			>>> results = vop.voregistry (searchTerm="quasar")
			>>> printResultPset (results)		# user-defined function

		When developing a package or when an app needs to explicitly include
		a new package, it may be accessed directly as:

			>>> vop = loadPackage ("vo-cli",file="/path/devpkg.e")	

		@param name		the package name
		@param file		the package binary file to execute

		Returns:	The loaded VOPackage object
	'''
	pass

def taskList (pkg, pattern=None):
	'''	List the tasks available in a package.
		@param pkg		the package name
		@param pattern	the task pattern name to match

		Returns:	A list of tasks available in the package who's name
					matches the 'pattern' string.
	'''
	pass

def pkgAttrs (pkg):
	''' Get the attributes for the named package, i.e. create a dictionary
		of package metadata.
		@param pkg		the package name

		Returns:	A dictionary of the package attributes.
	'''
	pass

def taskAttrs (pkg, task):
	''' Get the attributes for the named task in the package, i.e. create 
		a dictionary of task metadata.
		@param pkg		the package name
		@param task		the task name

		 Returns:	A dictionary of the task attributes.
	'''
	pass


class VOPackageError (Exception):
	'''  A base class for VO Package errors.  Use of this exception is TBD.
	'''
	pass


class VOPackage:
	'''  A class defining a VOPackage object.  A VOPackage is a collection
	     of tasks as well as metadata about the package itself.  The
	     functional part of the package is implemented in a binary file 
	     executing as a connected process, task discovery and execution are
	     implemented as commands sent to the package binary, results are
	     returned over the IPC channel as a stream of parameter objects.
	'''

    # Class attributes
	name	= None				# package name
	descr	= None				# description string
	author	= None				# package author
	contact	= None				# contact email address
	iconUrl	= None				# URL to package icon
	version	= None				# package version string
	dir     = None				# the directory containing the package
	binfile = None				# the name of the package binary

	def __init__ (self, dirs):
		pass

	def __iter__ (self, dirs):	# get next task in the package
		pass

	def taskList (self, pattern=None):
		''' Get the list of tasks in the package which match a pattern string.
			If no pattern is specified, all tasks are returned.
			@param pattern	the parameter name to match

			Returns:	A list of available tasks who's name matches
						the pattern string.
		'''
		pass

	def pkgAttrs (self):
		''' Get the attributes for the VO Package as a dictionary string.

			Returns:	A dictionary of the package attributes
		'''
		pass


class VOTaskError (Exception):
	'''  A base class for Task execution errors.
	'''
	pass

class VOTaskParameterError (VOTaskError):
	'''  an exception indicating an error in the task calling parameters
	'''
	pass

class VOTaskExecutionError (VOTaskError):
	'''  an exception indicating an error when executing the task
	'''
	pass


class VOTask:
	'''  A class defining a VO Task object.
	'''

	name		= None			# task name
	pkg			= None			# parent package name
	descr		= None			# task description string
	params		= None			# task input parameter set

	status		= None			# task execution return status (OK or ERROR)
	msg			= None			# task execution return error message
	

	def __init__ (self, name, pkg, descr, params):
		''' create the VOTask instance
		'''
		pass

	def taskAttrs (self):
		''' Get the attributes of the task as a dictionary.

			Returns:	A dictionary of the task attributes
		'''
		pass

	def setParams (pset):
		''' Set the task parameter pset.  Parameters in the pset argument
			will be used set the values for the task parameter pset, i.e.
			the argument pset can be just a subset of the task parameters,
			we'll match the names and set the values for only those params.
			If the argument pset contains a parameter not already in the 
		    task input pset, it will be added as a new parameter.
			@param pset		parameter set to load

			Returns:	A dictionary of the task attributes
		'''
		pass

	def getParams ():
		''' Set the task parameter pset.

			Returns:	The task parameter pset.
		'''
		pass

	def setCallback (pattern, func):
		''' Set a callback function to be run whenever a parameter name
			that matches the pattern is encountered.  Pattern applies only
			to the output parameter set.
			@param pattern	the parameter name to match
			@param func		the function to be called when parameter encountered

			Returns:	nothing
		'''
		pass

	def executeSync (self):
		''' Execute the task as a synchronous process.
			@throws VOTaskParameterError	thrown when parameters are invalid
			@throws VOTaskExecutionError	thrown when there is an error is
											executing a task, e.g. a segfault.

			Returns:	The result pset.
		'''
		pass

	def executeASync (self):
		''' Execute the task as a asynchronous process.

			Returns:	Nothing
		'''
		pass

	def wait (self):
		''' Wait for the exit of an asynchronous execution
			@throws VOTaskParameterError	thrown when parameters are invalid
			@throws VOTaskExecutionError	thrown when there is an error is
											executing a task, e.g. a segfault.

			Returns:	The result pset.
		'''
		pass

	def status (self):
		''' Get the status of an executing asynchronous task.

			Returns:	Task exec status ('Pending','Running','Done','Error')
		'''
		pass


class VOPset:
	'''  A class defining a PSet object.
	'''

	name		= None				# pset name
	pkg			= None				# package name
	task		= None				# task name associated with pset
	description = None				# pset description name

	def __init__ (self, name, type, descr, encoding):
		''' create the VOPset instance
		'''
		pass

	def loadPset (pkg=None, task=None, saveFile=None):
		''' Load the pset from the named task in the package.  If 'saveFile'
			is specified the pset is restored from that file.
			@param pkg		package name
			@param task		task name
			@param saveFile	name of the saved parameter file to load

			Returns:	The loaded PSet
		'''
		pass
			 
	def savePset (saveFile):
		''' Save the PSet to the named file.  Serialization of the PSet is
			TBD, probably some sort of simple XML schema.
			@param saveFile		name of the saved parameter file

			Returns:	Nothing
		'''
		pass

	def paramSet (pattern=None):	
		''' Create pset from params who's name matches the 'pattern' string.
			If no pattern is specified, all parameters are returned.
			@param pattern		parameter pattern name to match

			Returns:	The constructed PSet
		'''
		pass

	def paramList (pattern=None):
		''' Get list of params who's name matches the 'pattern' string.  If
			no pattern is specified, all parameters are returned.
			@param pattern		parameter pattern name to match

			Returns:	Nothing
		'''
		pass

	def addParam (name, type, description, encoding=None):
		''' Add a new parameter with the specified attributes to the pset.
			@param name		name of the parameter to add
			@param type		parameter type value
			@param descr	parameter description string
			@param encoding	parameter encoding

			Returns:	The created parameter
		'''
		pass

	def delParam (name):
		''' Delete the named parameter from the pset.
			@param name		name of the parameter to delete

			Returns:	Nothing
		'''
		pass

	def getParam (name):
		''' Get the parameter with the given name.
			@param name		name of the parameter to retrieve

			Returns:	Requested parameter
		'''
		pass


class VOParam:
	'''  A class defining a Parameter object.
	'''

	name	 = None				# parameter name
	type	 = None				# parameter type (string/int/real/bool/blob)
	desc	 = None				# parameter description string
	encoding = None				# encoding of param (i.e.  mime type)
				

	def __init__ (self, name, type, descr, encoding):
		''' create the VOParam instance
		'''
		pass

	def paramAttrs ():
		''' Get the parameter attributes.

			Returns:	A dictionary of parameter attrbutes
		'''
		pass
				  
	def getValue ():
		''' Get the value of the parameter (may be a list)

			Returns:	The parameter's value
		'''
		pass
				  
	def setValue (val):
		''' Set the value of a parameter.
			@param val		the value of the paramter (arbitrary type)

			Returns:	Nothing
		'''
		pass


