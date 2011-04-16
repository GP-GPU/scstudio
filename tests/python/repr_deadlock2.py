from pysc import *
import pycheck.deadlock_checker as dc

def fill_input():
	print ("Test of deadlock:   ")
	print ("h1:                 ")
	print ("   start            ")
	print ("     |        p2    ")
	print ("    p1              ")
	print ("     |              ")
	print ("    end             ")

	h1 = Msc("HMsc")
	start1 = Node("Start")
	h1.start = start1
	p1 = ReferenceNode()
	h1.add_node(p1)
	end1 = Node("End")
	h1.add_node(end1)
	p2 = Node("Reference")
	h1.add_node(p2)

	start1.add_successor(p1)
	p1.add_successor(end1)

	print ("start1:", start1)
	print ("p1:", p1)
	print ("p2:", p2)
	print ("end1:", end1)

	return h1

print("GOING TO REPRODUCE")
h1 = fill_input()
print("Referenced(h1):", dc.referenced(h1))
print("isBMscGraph:", dc.isBMscGraph(h1))
print("has deadlock:", dc.hasDeadlock(h1))
