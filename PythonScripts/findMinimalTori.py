def isTorus(tri):
	if tri.isOrientable() and tri.eulerChar() == 0:
		return True
	else:
		return False

canonicalEdgeOrders = [[1,2],[0,2],[0,1]]

def findTori(tri):
	found = []
	for x in tri.triangles():
		xi = x.index()
		for y in tri.triangles():
			yi = y.index()
			if xi < yi:
				if x.type() == 3 and x.type() == y.type():
					xverts = [x.vertex(i) for i in range(3)]
					yverts = [y.vertex(i) for i in range(3)]
					if xverts == yverts:
						xedges = [x.edge(i) for i in range(3)]
						yedges = [y.edge(i) for i in range(3)]
						xes = sorted([e.index() for e in xedges])
						yes = sorted([e.index() for e in yedges])
						if xes == yes:
							xrep = x.embedding(0)
							yrep = y.embedding(0)
							xrv = [xrep.vertices()[i] for i in range(3)]
							yrv = [yrep.vertices()[i] for i in range(3)]
							xre = []
							yre = []
							for e in xedges:
								for emb in e.embeddings():
									if emb.simplex() == xrep.simplex():
										ev = [emb.vertices()[i] for i in range(2)]
										if set(ev).issubset(set(xrv)):
											xre.append([e.index(),[emb.vertices()[i] for i in range(2)]])
							for e in yedges:
								for emb in e.embeddings():
									if emb.simplex() == yrep.simplex():
										ev = [emb.vertices()[i] for i in range(2)]
										if set(ev).issubset(set(yrv)):
											yre.append([e.index(),[emb.vertices()[i] for i in range(2)]])

							xd = {xrv[0] : 0, xrv[1] : 1, xrv[2] : 2}
							yd = {yrv[0] : 0, yrv[1] : 1, yrv[2] : 2}

							xe0 = [xd[xre[0][1][0]],xd[xre[0][1][1]]]
							xe1 = [xd[xre[1][1][0]],xd[xre[1][1][1]]]
							xe2 = [xd[xre[2][1][0]],xd[xre[2][1][1]]]

							ye0 = [yd[yre[0][1][0]],yd[yre[0][1][1]]]
							ye1 = [yd[yre[1][1][0]],yd[yre[1][1][1]]]
							ye2 = [yd[yre[2][1][0]],yd[yre[2][1][1]]]

							xce = [[xre[0][0],xe0],[xre[1][0],xe1],[xre[2][0],xe2]]
							yce = [[yre[0][0],ye0],[yre[1][0],ye1],[yre[2][0],ye2]]						

							for i in range(3):
								xel = xce[i][1]
								xvk = xce[i][0]
								if xel not in canonicalEdgeOrders:
									for el in yce:
										if el[0] == xvk:
											el[1].reverse()

							perms = []

							for i in range(3):
								for j in range(3):
									if xce[i][0] == yce[j][0]:
										perm = Perm3()
										if i == 0:
											perm = Perm3(j,yce[j][1][0],yce[j][1][1])
										if i == 1:
											perm = Perm3(yce[j][1][0],j,yce[j][1][1])
										if i == 2:
											perm = Perm3(yce[j][1][0],yce[j][1][1],j)
										perms.append(perm)

							surf = Triangulation2()
							surf.newSimplex()
							surf.newSimplex()
							A = surf.triangle(0)
							B = surf.triangle(1)
							for i in range(3):
								A.join(i,B,perms[i])	
							if isTorus(surf):
								found.append([x.index(), y.index()])
	if found != []:
		return found
	else:
		return None, None