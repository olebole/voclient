
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
