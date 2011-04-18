from pysc.list_ops import *

class Node(object):
	def __init__(self, t = "Start", l = ""):
		self.owner = None
		self.msc = None
		self.succ = []
		self.pred = []
		self.type = t
		self.position = (None, None)
		self.label = l
	def iss(self, t):
		return self.type == t
	def add_successor(self, succ):
		if succ not in self.succ:
			self.succ.append(succ)
			succ.pred.append(self)
	def traverse(self, condition = None, l = []):
		if self.ReferenceNode and self.msc and self.msc.HMsc:
			self.msc.traverse(condition, l)
		for node in self.succ:
			if node not in l:
				l.append(node)
				node.traverse(condition, l)
				if condition and condition(node):
					print("YUPI")
			else:
				#Gray Node Found
				pass
		return l
	def __getattr__(self, name):
		if name == "StartNode" or name == "ReferenceNode" or name == "ConnectionNode" or name == "ConditionNode" or name == "EndNode":
			return self.type == name[:-4] # or self.type == name
		elif name == "reachable":
			l = []
			self.traverse(None, l)
			return l
		else:
			return object.__getattribute__(self, name)
	def __setattr__(self, name, value):
		if name == "successor":
			self.add_successor(value)
		else:
			object.__setattr__(self, name, value)

class StartNode(Node):
	def __init__(self, t = "Start"):
		super(StartNode, self).__init__(t)

class ReferenceNode(Node):
	def __init__(self, t = "Reference"):
		super(ReferenceNode, self).__init__(t)

class ConnectionNode(Node):
	def __init__(self, t = "Connection"):
		super(ConnectionNode, self).__init__(t)

class ConditionNode(Node):
	def __init__(self, t = "Condition"):
		super(ConditionNode, self).__init__(t)

class EndNode(Node):
	def __init__(self, t = "End"):
		super(EndNode, self).__init__(t)

