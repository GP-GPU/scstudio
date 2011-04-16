from pysc import *
import pycheck.livelock_checker as lc

def fill1():
	print ("Test of deadlock:                    ")
	print ("jimmie:               tommie:        ")
	print ("   start                   start     ")
	print ("     |                       |       ")
	print ("    p1                       n1      ")
	print ("    / \\                    / |      ")
	print ("  p2   p3(h2)             n2--n3     ")
	print ("   |     \\                          ")
	print ("   |     p4                          ")
	print ("   |    /                            ")
	print ("   end                               ")

	h1 = HMsc("jimmie")
	h2 = HMsc("tommie")
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

	start1.add_successor(p1)
	p1.add_successor(p2)
	p1.add_successor(p3)
	p3.add_successor(p4)
	p2.add_successor(end1)
	p4.add_successor(end1)
	p3.msc = h2

	start2.add_successor(n1)
	n1.add_successor(n2)
	n2.add_successor(n3)
	n3.add_successor(n1)

	return h1

def fill2():
	h1 = HMsc("pennie")
	start1 = StartNode()
	end1 = EndNode()
	p1 = ReferenceNode()
	p2 = ReferenceNode()
	h1.start = start1
	h1.add_node(p1)
	h1.add_node(p2)
	h1.add_node(end1)
	start1.add_successor(p1)
	p1.add_successor(p2)
	p1.add_successor(end1)
	p2.add_successor(p1)
	return h1

print("GOING TO REPRODUCE")
h1 = fill1()
print("Looking for livelock in first input:")
out = lc.checkHMsc(h1, None)
print("Live Lock 1 : ", out)
h2 = fill2()
print("Looking for livelock in second input:")
out = lc.checkHMsc(h2, None)
print("Live Lock 2 : ", out)
