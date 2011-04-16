from pysc.list_ops import *

class Msc(object):
	def __repr__(self):
		return self.type + " object labeled \"" + self.label + "\""
	def __init__(self, t = "HMsc", l = ""):
		self.type = t
		self.nodes = Set()
		self.start = None
		self.label = l
		# BMsc
		self.instances = Set()
	def iss(self, t):
		return self.type == t
	def add_node(self, n):
		#change so that it uses function insert from list_ops
		self.nodes.insert(n)
		n.owner = self
	def remove_node(self, n):
		self.nodes.remove(n)
	def traverse(self, condition = None, l = []):
		l.append(self.start)
		self.start.traverse(condition, l)
		return l
	def reachable(self):
		return self.start.reachable
	def add_instance(self, instance):
		self.instances.insert(instance)
		instance.bmsc = self
	def __getattr__(self, name):
		if name == "snodes":
			return self.nodes + self.start
		elif name == "events":
			# Might be useful
			return Set([e for i in self.instances for a in i.areas for e in a.events])
		elif name == "BMsc" or name == "HMsc":
			return self.type == name
		elif name == "CoregionEventRelation":
			s = Set()
			for e in self.events:
				if e.CoregionEvent:
					#print("CE: ", e)
					for i in e.successors:
						s += (i.predecessor, i.successor)
			return s
		else:
			return object.__getattribute__(self, name)
	def __setattr__(self, name, value):
		if name == "start":
			object.__setattr__(self, "start", value)
			if value != None:
				value.owner = self
		elif name == "node":
			self.add_node(value)
		elif name == "instance":
			self.add_instance(value)
		else:
			return object.__setattr__(self ,name, value)

class HMsc(Msc):
	def __init__(self, l = ""):
		super(HMsc, self).__init__("HMsc", l)

class BMsc(Msc):
	def __init__(self, l = ""):
		super(BMsc, self).__init__("BMsc", l)
