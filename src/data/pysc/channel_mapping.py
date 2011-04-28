class GeneralMapper(object):
	def __init__(self):
		self._instance = None
		self.channels = []
	def __getattr__(self, name):
		if name == "channel_count":
			return len(self.channels)
		elif name == "instance":
			if not self._instance:
				self._instance = GeneralMapper()
			return self._instance
		else:
			return object.__getattribute(self, name)
	def channel(self, event):
		pass
	def same_channel(self, e1, e2):
		pass

class SRMessagePart(object):
	def __init__(self, event):
		self.sender = event.sender_label
		self.receiver = event.receiver_label
	def __le__(self, mp):
		return self.sender < mp.sender or (self.sender == mp.sender and self.receiver < mp.receiver)
	def same_channel(self, e1, e2):
		if (e1.is_send and e2.is_send) or (e1.is_receive and e2.is_receive):
			return e1.sender_label == e2.sender_label and e1.receiver_label == e2.receiver_label
		return False

class SRMMessagePart(object):
	def __init__(self, event):
		self.sender = event.sender_label
		self.receiver = event.receiver_label
		self.label = event.message.label
	def __le__(self, b):
		return self.sender < mp.sender or (self.sender == mp.sender and (self.receiver < mp.receiver or (self.receiver == mp.receiver and self.label < mp.label)))
	def same_channel(self, e1, e2):
		if (e1.is_send and e2.is_send) or (e1.is_receive and e2.is_receive):
			return e1.sender_label == e2.sender_label and e1.receiver_label == e2.receiver_label and e1.message.label == e2.message.label
		return False

class SRChannelMapper(GeneralMapper):
	def channel(self, ch):
		part = SRMessagePart(event)
		try:
			idx = self.channel.index(part)
		except ValueError:
			self.channels.insert((part,len(self.channels)))
			return len(self.channels)-1
		return self.channels[idx][1]
	def same_channel(self, e1, e2):
		if (e1.is_send and e2.is_send) or (e1.is_receive and e2.is_receive):
			return e1.sender_label == e2.sender_label and e1.receiver_label == e2.receiver_label
		return False

class SRMChannelMapper(object):
	def channel(self, event):
		part = SRMMessagePart(event)
		try:
			idx = self.channel.index(part)
		except ValueError:
			self.channels.insert((part,len(self.channels)))
			return len(self.channels)-1
		return self.channels[idx][1]
	def same_channel(self, e1, e2):
		if (e1.is_send and e2.is_send) or (e1.is_receive and e2.is_receive):
			return e1.sender_label == e2.sender_label and e1.receiver_label == e2.receiver_label and e1.message and e2.message and e1.message.label == e2.message.label
		return False
