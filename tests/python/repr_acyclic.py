from pysc import *
import pycheck.acyclic_checker as ac

def fill1():
	b1 = BMsc()
	i1 = Instance("p1")
	i2 = Instance("p2")
	b1.add_instance(i1)
	b1.add_instance(i2)
	a1 = StrictOrderArea()
	i1.add_area(a1)
	a2 = StrictOrderArea()
	i2.add_area(a2)
	e1 = a1.add_event()
	e2 = a1.add_event()
	e3 = a2.add_event()
	e4 = a2.add_event()
	m1 = CompleteMessage("a")
	m1.events = (e1, e3)
	m2 = CompleteMessage("a")
	m2.events = (e4, e2)
	return b1

def fill2():
	b1 = BMsc()
	i1 = Instance("p1")
	i2 = Instance("p2")
	b1.add_instance(i1)
	b1.add_instance(i2)
	a1 = StrictOrderArea()
	i1.add_area(a1)
	a2 = StrictOrderArea()
	i2.add_area(a2)
	e1 = a1.add_event()
	e2 = a1.add_event()
	e3 = a2.add_event()
	e4 = a2.add_event()
	m1 = CompleteMessage("a")
	m1.events = (e4, e1)
	m2 = CompleteMessage("a")
	m2.events = (e2, e3)
	return b1

def fill3():
	h1 = HMsc()
	s = StartNode()
	h1.start = s
	n1 = ReferenceNode()
	b1 = BMsc()
	i1 = Instance("p1")
	b1.instance = i1
	i2 = Instance("p2")
	b1.instance = i2
	n1.msc = b1
	h1.node = n1
	return h1

b1 = fill1()
print(ac.is_acyclic(b1))
b2 = fill2()
print(ac.is_acyclic(b2))
h = fill3()
print(ac.checkHMsc(h, None))
