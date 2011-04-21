from pysc import *
from pycheck.deadlock_checker import *

def buildVisOrder(bmsc):
	order = Set()
	for e1 in bmsc.events:
		for e2 in bmsc.events:
			if (e1 == e2) or (e1.area and e1.area.StrictOrderArea and e1.successor == e2) or ((e1, e2) in bmsc.CoregionEventRelation) or (e1.message and e1.message.CompleteMessage and e1.is_send and e1.message.receive_event == e2) or (e1.area and e1.area.next == e2.area):
				order += (e1, e2)
	return order.transitive()

def is_acyclic(bmsc):
	visOrdRel = buildVisOrder(bmsc)
	# Test on antisymmetry
	for (i, j) in visOrdRel:
		if (j, i) in visOrdRel and i != j:
			return False
	return True

def hmsc_is_not_acyclic(h):
	failed = Set()
	for h1 in referenced(h):
		for n in h1.nodes:
			if n and n.ReferenceNode and n.msc and n.msc.BMsc:
				print("Running is_acyclic")
				if not is_acyclic(n.msc):
					failed += n.owner
					#return False
	return failed

def checkBMsc(bmsc, chm):
	if is_acyclic(bmsc):
		print("Pass")
		return []
	print("Fail")
	return [bmsc]

def checkHMsc(hmsc, chm):
	hina = hmsc_is_not_acyclic(hmsc)
	if hina:
		print("Fail")
	else:
		print("Pass")
	return hina
