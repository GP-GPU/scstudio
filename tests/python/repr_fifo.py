from pysc import *
import pycheck.fifo_checker as fc

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
	b1 = BMsc("Page_1")
	i1 = Instance("A")
	i2 = Instance("B")
	i3 = Instance("C")
	i4 = Instance("D")
	b1.instance = i1
	b1.instance = i2
	b1.instance = i3
	b1.instance = i4
	a1 = StrictOrderArea()
	a2 = StrictOrderArea()
	a3 = StrictOrderArea()
	a4 = StrictOrderArea()
	i1.area = a1
	i2.area = a2
	i3.area = a3
	i4.area = a4
	e1 = a1.add_event()
	e2 = a2.add_event()
	e3 = a2.add_event()
	e4 = a3.add_event()
	e5 = a3.add_event()
	e6 = a4.add_event()
	m1 = CompleteMessage("m1")
	m2 = CompleteMessage("m2")
	m3 = CompleteMessage("m3")
	m1.events = (e1, e2)
	m2.events = (e3, e4)
	m3.events = (e5, e6)
	return b1

chm = SRMChannelMapper()
#print(fc.checkBMsc(fill1(), chm))
print(fc.checkBMsc(fill2(), chm))
