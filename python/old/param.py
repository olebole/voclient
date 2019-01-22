
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


