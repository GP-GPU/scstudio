# These are not very effective versions of basic set operations. Their ineffectivity implies from the fact that their objects might not be comparable/hashable and therefore every known more efficient algorithm (that is based on comparing) fails here.

def defined(a):
	return a != None

def insert(list_t, candidate):
	cond = candidate not in list_t
	if cond:
		list_t.append(candidate)
	return cond

def mkset(list_l):
	set_t = []
	for i in list_l:
		insert(set_t, i)
	return set_t

def eq(a, b):
	for i in a:
		if i not in b:
			return False
	if len(a) != len(b):
		return False
	return True

def neq(a, b):
	return not eq(a, b)

def is_subset(a, b):
	for i in a:
		if i not in b:
			return False
	return True

def is_dsubset(a, b):
	return is_subset(a, b) and a != b

def is_overset(a, b):
	return is_subset(b, a)

def is_doverset(a, b):
	return is_dsubset(b, a)

def unite(a, b):
	set_t = nefset()
	for i in a:
		set_t.append(i)
	for i in b:
		set_t.insert(i)
	return set_t

def intersect(a, b):
	set_t = nefset()
	for i in a:
		if i in b:
			insert(set_t, i)
	return set_t

def diff(a, b):
	set_t = nefset()
	for i in a:
		if i not in b:
			set_t.append(i)
	return set_t

def sym_diff(a, b):
	return unite(diff(a,b), diff(b, a))

def cartesian(a, b):
	r = nefset()
	for i in a:
		for j in b:
			r.insert((i, j))
	return r

def transitive(a):
	lo = list(a)
	mod = True
	while mod:
		mod = False
		for (a1, a2) in lo:
			for (b1, b2) in lo:
				if a2 == b1 and (a1, b2) not in lo:
					lo.append((a1, b2))
					mod = True
	return Set(lo)

class nefset(list):
	def __str__(self):
		return "This is not very effective set of non-hashable objects, most operations are about n^2"
	def __repr__(self):
		return "{" + super(nefset, self).__repr__()[1:-1] + "}"
	def __init__(self, t = [], sflag = True):
#		print mkset(t)
		if sflag:
			k = mkset(t)
		else:
			k = t
		super(nefset, self).__init__(k)
	__eq__ = eq
	__ne__ = neq
#	append = insert
	insert = insert
	add = insert
	__le__ = is_subset
	__lt__ = is_dsubset
	__gt__ = is_doverset
	__ge__ = is_overset
	__or__ = unite
	__and__ = intersect
	__sub__ = diff
	__xor__ = sym_diff
	__mul__ = cartesian

def predTransitive(a):
	lo = list(a.data)
	mod = True
	while mod:
		mod = False
		for (a1, a2) in lo:
			for (b1, b2) in lo:
				if a2 == b1 and (a1, b2) not in lo:
					lo.append((a1, b2))
					mod = True
	return Set(lo)

class Set(object):
	def __str__(self):
		return self.__repr__()
	def __iter__(self):
		return self.data.__iter__()
	def __repr__(self):
		return self.data.__repr__()
	def __init__(self, t = [], hashable = True):
		self.hashable = True
		try:
			self.data = set(t)
		except TypeError:
			self.data = nefset(t)
			self.hashable = False
	def __eq__(self, b):
		if self.hashable == b.hashable:
			return  self.data == b.data
		elif self.hashable:
			j = self
			k = b
		else:
			j = b
			k = self
		return nefset(j.data, False) == k.data
	def __ne__(self, b):
		return not self == b
	def __le__(self, b):
		if self.hashable == b.hashable:
			return  self.data <= b.data
		elif self.hashable:
			j = self
			k = b
		else:
			j = b
			k = self
		return nefset(j.data, False) <= k.data
	def __lt__(self, b):
		if self.hashable == b.hashable:
			return  self.data < b.data
		elif self.hashable:
			j = self
			k = b
		else:
			j = b
			k = self
		return nefset(j.data, False) < k.data
	def __ge__(self, b):
		if self.hashable == b.hashable:
			return  self.data >= b.data
		elif self.hashable:
			j = self
			k = b
		else:
			j = b
			k = self
		return nefset(j.data, False) >= k.data
	def __gt__(self, b):
		if self.hashable == b.hashable:
			return  self.data > b.data
		elif self.hashable:
			j = self
			k = b
		else:
			j = b
			k = self
		return nefset(j.data, False) > k.data
	def __or__(self, b):
		set_t = Set()
		set_t.hashable = False
		if self.hashable == b.hashable:
			set_t.hashable = self.hashable
			set_t.data = self.data | b.data
		elif self.hashable:
			set_t.data = nefset(self.data, False) | b.data
		else:
			set_t.data = self.data | nefset(b.data, False)
		return set_t
	def __and__(self, b):
		set_t = Set()
		if self.hashable == b.hashable:
			set_t.hashable = self.hashable
			set_t.data = self.data & b.data
			return  set_t
		elif self.hashable:
			j = self
			k = b
		else:
			j = b
			k = self
		# Hashing is intrusive in intersection
		hashable = True
		list_t = []
		for i in j:
			if i in k:
				list_t.append(i)
		set_t.data = set(list_t)
		return set_t
	def __sub__(self, b):
		set_t = Set()
		set_t.hashable = False
		if self.hashable == b.hashable:
			set_t.hashable = self.hashable
			set_t.data = self.data - b.data
		elif self.hashable:
			set_t.hashable = True
			list_t = []
			for i in self:
				if i not in b:
					list_t.append(i)
			set_t.data = set(list_t)
		else:
			set_t.data = self.data - nefset(b.data, False)
		return set_t
	def __xor__(self, b):
		set_t = Set()
		set_t.hashable = False
		if self.hashable == b.hashable:
			set_t.hashable = self.hashable
			set_t.data = self.data - b.data
		elif self.hashable:
			set_t.data = nefset(self.data, False) ^ b.data
		else:
			set_t.data = self.data ^ nefset(b.data, False)
		return set_t
	def __len__(self):
		return len(self.data)
	def __add__(self, m):
		return self | Set([m])
	def __iadd__(self, m):
		self.insert(m)
		return self
	def __mul__(self, m):
		r = Set()
		if self.hashable and m.hashable:
			r.hashable = True
			r.data = set()
		else:
			r.hashable = False
			r.data = nefset()
		for i in self:
			for j in m:
				r.data.add((i, j))
		return r
	def insert(self, m):
		try:
			self.data.add(m)
		except TypeError:
			# We have to change our representation to nefset
			# m is not a hashable object
			# Fortunately, this is just O(n) operation
			set_t = self.data
			self.hashable = False
			self.data = nefset(set_t, False)
			self.data.add(m)
	def remove(self, m):
		self.data.remove(m)
		# todo - make hashable if possible
	transitive = predTransitive
