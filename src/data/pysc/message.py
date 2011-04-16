class CompleteMessage(object):
	"""
	label = "" -- label
	type = "" -- Complete/Incomplete
	send_event = sender
	receive_event = receiver
	sender = send_event.instance
	receiver = receive_event.instance
	event = None
	"""
	def __init__(self, label = "", sender = None, receiver = None, original = None):
		object.__setattr__(self, "send_event", None)
		object.__setattr__(self, "receive_event", None)
		self.otype = "Complete"
		self.label = label
		self.send_event = sender
		self.receive_event = receiver
	def __getattr__(self, name):
		if name == "sender":
			return self.send_event.instance
		elif name == "receiver":
			return self.receive_event.instance
		elif name == "is_glued":
			return (self.send_event and self.receive_event)
		elif name == "CompleteMessage":
			return True
		elif name == "IncompleteMessage":
			return False
		else:
			return object.__getattribute__(self, name)
	def __setattr__(self, name, value):
		if name == "send_event":
			if value != self.send_event:
				if self.send_event:
					self.send_event.message = None
				if value:
					value.message = self
				object.__setattr__(self, name, value)
		elif name == "receive_event":
			if value != self.receive_event:
				if self.receive_event:
					self.receive_event.message = None
				if value:
					value.message = self
				object.__setattr__(self, name, value)
		elif name == "events":
			if len(value) != 2:
				raise AttributeError("Incorrect amount of attributes, 2 attributes necessary")
			self.send_event = value[0]
			self.receive_event = value[1]
		else:
			object.__setattr__(self, name, value)
	def is_glued(self):
		return self.send_event and self.receive_event


class IncompleteMessage(object):
	"""
	label = "" -- label
	type = "" -- Complete/Incomplete
	event = None
	dot_position = (a, b)
	instance_label = ""
	"""
	def __init__(self, label = "", sender = None, receiver = None, original = None):
		object.__setattr__(self, "event", None)
		self.otype = "Incomplete"
		self.type = ""
		self.label = label
		#Incomplete
		self.dot_position = None
		self.instance_label = None
		self.event = None
	def __getattr__(self, name):
		if name == "is_glued":
			return self.event != None
		elif name == "is_lost":
			return self.type == "lost"
		elif name == "is_found":
			return self.type == "found"
		elif name == "CompleteMessage" or name == "IncompleteMessage":
			return self.otype == name[:-7]
		else:
			object.__getattribute__(self, name)
	def __setattr__(self, name, value):
		if name == "event":
			if value != self.event:
				if self.event:
					self.event.message = None
				if value:
					value.message = self
				object.__setattr__(self, name, value)
		else:
			object.__setattr__(self, name, value)
	def glue_event(self, event):
		if event != self.event:
			if self.event:
				self.event.message = None
			if event:
				event.message = self
			self.event = event
