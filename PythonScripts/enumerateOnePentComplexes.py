T = Triangulation4()
T.newSimplex()
p = T.pentachoron(0)
goodSeen = []

for i in range(1,120):
	perm = Perm5.S5[i]
	isBad = False
	try:
		p.join(0,p,perm)
		for f in T.triangles():
			if f.hasBadIdentification():
				isBad = True
		for e in T.edges():
			if e.hasBadIdentification():
				isBad = True
			if e.hasBadLink():
				isBad = True
		if not isBad and T.isOrientable():
			sig = T.isoSig()
			if sig not in goodSeen:
				goodSeen.append(sig)
		p.unjoin(0)
	except:
		continue

for sig in goodSeen:
	print(sig)