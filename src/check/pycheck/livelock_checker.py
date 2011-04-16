from pysc import *

def find_node_livelock(node, l = []):
	found = Set([])
	if node.ReferenceNode and node.msc:
		found |= find_node_livelock(node.msc.start, l)
	for n in node.succ:
		if n not in l:
			l.append(n)
			found |= find_node_livelock(n, l)
			l.pop()
		else:
			cycle = l[l.index(n):]
			# cycle + [n]
			ref = False
			for i in cycle:
				if i.ReferenceNode:
					ref = True
					# Modify for end node
					if [x for x in i.reachable if x.EndNode and x.owner == i.msc]:
						# Reachable EndNode was found
						ref = False
			if ref:
				found += n.owner
				print("Livelock was found: ", cycle + [n])
	return found

def find_livelock(h, l = []):
	if h.HMsc:
		l.append(h.start)
		find_node_livelock(h.start, l)
		l.pop()

def checkHMsc(hmsc, chm):
	return list(find_node_livelock(hmsc.start, [hmsc.start]))
