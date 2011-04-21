from pysc.list_ops import *
from pysc.event import *

class EventArea(object):
	def __init__(self, t = ""):
		object.__setattr__(self, "next", None)
		object.__setattr__(self, "previous", None)
		self.begin_height = None
		self.end_height = None
		self.width = None
		self.instance = None
		self.type = t
	def __setattr__(self, name, value):
		if name == "next":
			object.__setattr__(self, name, value)
			object.__setattr__(value, "previous", self)
		elif name == "previous":
			object.__setattr__(self, name, value)
			object.__setattr__(value, "next", self)
		else:
			object.__setattr__(self, name, value)
	def __getattr__(self, name):
		if name == "is_empty":
			pass
		elif name == "is_first":
			return self.previous == None
		elif name == "height":
			if self.end_height != None and self.begin_height != None:
				return abs(self.end_height - self.begin_height)
			else:
				return 0
		elif name == "StrictOrderArea" or name == "CoregionArea":
			return self.type == name
		else:
			return object.__getattribute__(self, name)

class StrictOrderArea(EventArea):
	def __init__(self):
		super(StrictOrderArea, self).__init__(t = "StrictOrderArea")
		object.__setattr__(self, "first", None)
		object.__setattr__(self, "last", None)
		#self.events = Set()
	def __setattr__(self, name, value):
		if name == "first":
			if not self.is_empty:
				object.__setattr__(value, "successor", self.first)
			object.__setattr__(self, name, value)
			if not self.last:
				self.last = self.first
			#self.events += value
		elif name == "last":
			if not self.last:
				object.__setattr__(self, name, value)
				object.__setattr__(self, "first", value)
			else:
				self.last.successor = value
				object.__setattr__(self, name, value)
			value.area = self
			#self.events += value
		elif name == "event":
			self.last = value
#			self.add_event(value)
		else:
			super(StrictOrderArea, self).__setattr__(name, value)
	def __getattr__(self, name):
		if name == "is_empty":
			return self.first == None
		elif name == "events":
			if self.is_empty:
				return Set()
			s = Set([self.first])
			runner = self.first
			while runner.successor and runner.successor != self.last:
				s += runner.successor
				runner = runner.successor
			s += self.last
			return s
		else:
			return super(StrictOrderArea, self).__getattr__(name)
	def add_event(self, e = None):
		if not e:
			e = StrictEvent()
		self.last = e
		return e

class CoregionArea(EventArea):
	def __init__(self):
		super(CoregionArea, self).__init__(t = "CoregionArea")
		self.form = None
		self.minimal_events = Set([])
		self.maximal_events = Set([])
	def add_minimal_event(self, event):
		event.area = self
		self.minimal_events.insert(event)
	def remove_minimal_event(self, event):
		try:
			self.minimal_events.remove(event)
		except:
			pass
	def add_event(self, event = CoregionEvent()):
		# self.events.insert(event)
		if event.is_minimal:
			self.minimal_events.insert(event)
		elif event.is_maximal:
			self.maximal_events.insert(event)
		event.area = self
		return event
	def add_maximal_event(self, event):
		event.area = self
		self.maximal_events.insert(event)
	def remove_maximal_event(self, event):
		try:
			self.maximal_events.remove(event)
		except:
			pass
	def __getattr__(self, name):
		if name == "is_empty":
			return len(self.minimal_events) == 0
		elif name == "events":
			return self.minimal_events | self.maximal_events
		else:
			return super(CoregionArea, self).__getattr__(name)
	def __setattr__(self, name, value):
		if name == "minimal_event":
			self.add_minimal_event(value)
		elif name == "maximal_event":
			self.add_maximal_event(value)
		else:
			EventArea.__setattr__(self, name, value)
