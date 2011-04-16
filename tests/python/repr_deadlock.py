from pysc import *
import pycheck.deadlock_checker as dc

def fill_input():
	print ("Test of deadlock:                    ")
	print ("h1:                     h2:          ")
	print ("   start                   start     ")
	print ("     |                       |  \\   ")
	print ("    p1                       n1  |   ")
	print ("    / \\                    / ||  |  ")
	print ("  p2   p3(h2)              | n2  |   ")
	print ("   |     \\                 \\ |   | ")
	print ("   |     p4                  n3  /   ")
	print ("   |    /                     | /    ")
	print ("   end                       n4      ")

	h1 = Msc("HMsc")
	h2 = HMsc()
	start1 = Node("Start")
	h1.start = start1
	end1 = Node("End")
	h1.add_node(end1)
	p1 = ReferenceNode()
	h1.add_node(p1)
	p2 = Node("Reference")
	h1.add_node(p2)
	p3 = Node("Reference")
	h1.add_node(p3)
	p4 = Node("Reference")
	h1.add_node(p4)

	start2 = StartNode()
	h2.start = start2
	n1 = Node("Reference")
	h2.add_node(n1)
	n2 = ReferenceNode()
	h2.add_node(n2)
	n3 = Node("Reference")
	h2.add_node(n3)
	n4 = Node("Reference")
	h2.add_node(n4)

	end2 = EndNode()
	h2.add_node(end2)

	start1.add_successor(p1)
	p1.add_successor(p2)
	p1.add_successor(p3)
	p3.add_successor(p4)
	p2.add_successor(end1)
	p4.add_successor(end1)
	p3.msc = h2

	start2.add_successor(n1)
	start2.add_successor(n4)
	n1.add_successor(n2)
	n1.add_successor(n3)
	n2.add_successor(n3)
	n3.add_successor(n4)

	print ("start1:", start1)
	print ("start2:", start2)
	print ("p1:", p1)
	print ("p2:", p2)
	print ("p3:", p3)
	print ("p4:", p4)
	print ("n1:", n1)
	print ("n2:", n2)
	print ("n3:", n3)
	print ("n4:", n4)
	print ("end1:", end1)
	print ("end2:", end2)

	return h1

print("GOING TO REPRODUCE")
h1 = fill_input()
print("Referenced(h1):", dc.referenced(h1))
print("isBMscGraph:", dc.isBMscGraph(h1))
print("has deadlock:", dc.hasDeadlock(h1))
