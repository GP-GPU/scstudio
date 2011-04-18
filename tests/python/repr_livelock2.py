from pysc import *
import pycheck.livelock_checker as lc

def fill1():
	print ("Test of livelock:                    ")
	print ("timmie:                jane:         ")
	print ("   start                   start     ")
	print ("     |                       |       ")
	print ("    n1(h2)                  p1      ")
	print ("   / |                       |      ")
	print (" n2--n3(h2)                 end     ")

	h1 = HMsc("timmie")
	start1 = Node("Start")
	h1.start = start1
	end1 = Node("End")
	h1.add_node(end1)
	n1 = ReferenceNode()
	h1.add_node(n1)
	n2 = Node("Reference")
	h1.add_node(n2)
	n3 = Node("Reference")
	h1.add_node(n3)

	h2 = HMsc("jane")
	start2 = StartNode()
	h2.start = start2
	p1 = Node("Reference")
	h2.add_node(p1)
	end2 = EndNode()
	h2.end = end2

	b1 = BMsc("baddie");
	start1.add_successor(n1)
	n1.add_successor(n2)
	n2.add_successor(n3)
	n3.add_successor(n1)
	n1.msc = h2
	n2.msc = b1
	n3.msc = h2


	start2.add_successor(p1)
	p1.add_successor(end2)

	return h1


print("GOING TO REPRODUCE")
h1 = fill1()
print("Looking for livelock in first input:")
out = lc.checkHMsc(h1, None)
print("Livelock: ", out)
