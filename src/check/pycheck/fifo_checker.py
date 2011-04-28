from pysc import *
from pycheck.acyclic_checker import *

def is_fifo(bmsc, chm):
	if not is_acyclic(bmsc):
		# BMsc is not acyclic, hence false
		return False
	visOrder = buildVisOrder(bmsc)
	for e1 in bmsc.events:
		for e2 in bmsc.events:
			if e1.is_receive and e2.is_receive and chm.same_channel(e1, e2) and (e1, e2) in visOrder and e1.message and e2.message and e1.message.CompleteMessage and e2.message.CompleteMessage and not ((e1.matching_event, e2.matching_event) in visOrder):
				return False
	return True

def hmsc_is_not_fifo(hmsc, chm):
	failed = Set()
	for h1 in referenced(hmsc):
		for n in h1.nodes:
			if n and n.ReferenceNode and n.msc and n.msc.BMsc and not is_fifo(n.msc, chm):
				failed += n.owner
	return list(failed)

def checkBMsc(bmsc, chm):
	if is_fifo(bmsc, chm):
		print("Pass")
		return []
	print("Fail")
	return [bmsc]

def checkHMsc(hmsc, chm):
	hina = hmsc_is_not_fifo(hmsc, chm)
	if hina:
		print("Fail")
	else:
		print("Pass")
	return hina
