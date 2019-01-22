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

A “task is a computational component which can be executed as a process by
the host operating system (or a related environment such as a cluster).
Tasks may be written in any language so long as the defined tasking
interface is observed.  The “package” refered to here should not be
confused with a Python package.  A Python package is an element of Python,
a 'package' as referred to here is a collection of tasks and their
associated metadata.

"""

class VOPList:
	'''  A class defining a collection of VO packages.
	'''

	def __init__ (self, dirPath)



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

	def setPkgDirs (self, dirs):
		''' Set the VO package search path by specifying a list of
		    directories.
		'''
		pass

	def getPkgDirs (self):
		''' Get the search path for VO packages.
			Returns:	the list of package directories
		'''

	def appendPkgDirs (self, dir):
		''' Append the VO package search path with the specified directory
		'''
		pass


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
		''' Get the list of tasks in the packages which match a name pattern.
			Returns:	a list of available tasks who's name matches
						the pattern string.
		'''

	def pkgAttrs (self, pkgName):
		''' Get the attributes for a named VO Package.
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

