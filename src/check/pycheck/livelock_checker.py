from pysc import *

def find_node_livelock(node, original, l = []):
	found = Set([]) # Set of HMscs that does not satisfy this condition
	if node.ReferenceNode and node.msc and node.msc.HMsc:
		# Found reference node that references HMsc
		found |= find_node_livelock(node.msc.start, original, l)
	for n in node.succ:
		if n not in l:
			# Node that was not traversed in this path
			l.append(n)
			found |= find_node_livelock(n, original, l)
			l.pop()
		else:
			# Reached same node -> cycle
			cycle = l[l.index(n):]
			ref = False
			for i in cycle:
				if i.ReferenceNode:
					ref = True
					if [x for x in i.reachable if x.EndNode and x.owner == original]:
						# Reachable EndNode was found
						ref = False
			if ref:
				found += n.owner
				print("Livelock was found: ", cycle + [n])
	return found

def find_livelock(h, l = []):
	if h.HMsc:
		l.append(h.start)
		hmscs = find_node_livelock(h.start, h, l)
		l.pop()
	return hmscs

def checkHMsc(hmsc, chm):
	return list(find_node_livelock(hmsc.start, hmsc, [hmsc.start]))
