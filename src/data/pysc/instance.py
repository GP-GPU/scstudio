from pysc.list_ops import *
import math

class Instance(object):
	"""
	areas = []
	form = None # line/column
	label = ""
	type = ""
	first = None
	last = None
	bmsc = None # Father
	line_begin = (0, 0)
	line_end = (0, 0)
	width = 0
	"""
	def __repr__(self):
		if self.label:
			return "Instance labeled \"" + self.label + "\""
		else:
			return "Empty instance"
	def __init__(self, l = "", t = ""):
		#self.areas = []
		self.form = None # line/column True/False...
		self.label = l
		self.kind = t
		self.first = None
		self.last = None
		self.bmsc = None # Father
		self.line_begin = (0, 0)
		self.line_end = (0, 0)
		self.width = 0
	def __getattr__(self, name):
		if name == "height":
			dx = line_end[0] - line_begin[0]
			dy = line_end[1] - line_begin[1]
			return math.sqrt(dx^2 + dy^2)
		elif name == "is_empty":
			return self.last == None
		elif name == "has_events":
			return self.first != None and self.last != None
		elif name == "areas":
			runner = self.first
			if not runner:
				return Set()
			s = Set([self.first])
			while runner.next and runner.next != self.last:
				s += runner.next
				runner = runner.next
			s += self.last
			return s
		else:
			object.__getattribute__(self, name)
	def __setattr__(self, name, value):
		if name == "kind" or name == "type":
			object.__setattr__(self, "kind", value)
			object.__setattr__(self, "type", value)
		elif name == "area":
			self.add_area(value)
		else:
			object.__setattr__(self, name, value)
	def add_area(self, a):
		if not self.is_empty:
			self.last.next = a
		else:
			self.first = a
		self.last = a
		a.instance = self
