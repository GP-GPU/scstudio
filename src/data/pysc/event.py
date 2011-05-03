from pysc.list_ops import *

class Event(object):
	def __init__(self, t = "Coregion", original = None):
		self.type = t
		self.position = (None, None)
		self.message = None
		self.area = None
		if original:
			self.position = original.position
	def __getattr__(self, name):
		if "complete_message" == name:
			if self.message and self.message.otype == "Complete":
				return self.message
			else:
				return None
		elif "incomplete_message" == name:
			if self.message and self.message.otype == "Incomplete":
				return self.message
			else:
				return None
		elif "matching_event" == name:
			if self.is_matched:
				if self.is_send:
					return self.complete_message.receive_event
				return self.complete_message.send_event
			return None
		elif "is_matched" == name:
			return self.message and self.message.CompleteMessage
		elif "is_send" == name:
			if self.is_matched:
				return self.message.send_event == self
			elif self.message:
				return self.message.is_lost
			else:
				return False
		elif "is_receive" == name:
			if self.message:
				return not self.is_send
			else:
				return False
		elif "receiver_label" == name:
			try:
				if self.is_matched:
					return self.message.receiver.label
				else:
					if self.is_send:
						return self.message.instance_label
					else:
						return self.instance.label
			except:
				return ""
		elif "sender_label" == name:
			try:
				if self.is_matched:
					return self.message.sender.label
				else:
					if self.is_receive:
						return self.message.instance_label
					else:
						return self.instance.label
			except:
				return ""
		elif "general_area" == name:
			return self.area
		elif "instance" == name:
			if self.area:
				return self.area.instance
			return None
		elif "StrictEvent" == name or "CoregionEvent" == name:
			return self.type == name[:-5]
		else:
			return object.__getattribute__(self, name)

class StrictEvent(Event):
	def __init__(self, t = "Strict"):
		super(StrictEvent, self).__init__(t)
		object.__setattr__(self, "successor", None)
		self.predecessor = None
	def __setattr__(self, name, value):
		if name == "successor":
			if not self.is_last:
				object.__setattr__(self.successor, "predecessor", value)
			object.__setattr__(value, "predecessor", self)
			object.__setattr__(value, "successor", self.successor)
			object.__setattr__(self, "successor", value)
		else:
			object.__setattr__(self, name, value)
	def __getattr__(self, name):
		if name == "is_first":
			return self.predecessor == None
		elif name == "is_last":
			return self.successor == None
		else:
			return super(StrictEvent, self).__getattr__(name)

class CoregionEvent(Event):
	def __init__(self, t = "Coregion"):
		super(CoregionEvent, self).__init__(t)
		self.successors = Set()
		self.predecessors = Set()
	def add_successor(self, succ):
		try:
			succ.CoregionEventRelation
			e = succ
		except:
			e = CoregionEventRelation(self, succ)
		self.successors += e
		e.predecessor = self
		self.coregion_area.remove_maximal_event(self)
		e.successor.predecessors.insert(e)
		e.successor.coregion_area.remove_minimal_event(e.successor)
	def remove_successor(self, succ):
		succ.predecessors.remove(self)
		self.successors.remove(succ)
	def __getattr__(self, name):
		if name == "is_minimal":
			return len(self.predecessors) == 0
		elif name == "is_maximal":
			return len(self.successors) == 0
		if name == "has_predecessors":
			return len(self.predecessors) != 0
		elif name == "has_successors":
			return len(self.successors) != 0
		elif name == "coregion_area":
			return self.area
		elif name == "lcoregionrelations":
			return list(self.successors)
		else:
			return super(CoregionEvent, self).__getattr__(name)
	def __setattr__(self, name, value):
		if name == "successor":
			self.add_successor(value)
		else:
			Event.__setattr__(self, name, value)

class CoregionEventRelation(object):
	def __init__(self, pred = None, succ = None, line = (None, None)):
		self.predecessor = pred
		self.successor = succ
		self.line = line
	def __getattr__(self, name):
		if name == "CoregionEventRelation":
			return True
		else:
			return object.__getattribute__(self, name)
