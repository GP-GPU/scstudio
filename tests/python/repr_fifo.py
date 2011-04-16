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

chm = SRChannelMapper()
print(fc.is_fifo(fill1(), chm))

