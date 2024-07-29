def Bp(p):
	# Constructs a triangulation of a rational ball B_p.
	# \pi_1(B_p) = Z_p
	# \partial B_p = L(p^2,-1-p).
	
	base = Example3.lens(p**2,p-1)
	X = Example4.singleCone(base)
	
	tsb1 = Triangulation4('bIaa2aba')
	X.insertTriangulation(tsb1)
	
	s = X.size()
	top = X.pentachoron(s-1)
	midIndx = int(s/2)-1
	mid = X.pentachoron(midIndx)
	
	top.join(1,mid,Perm5(0,4,2,3,1))
	
	for i in range(1,p-1):
		nxtIndx = p+i-2
		prvIndx = p-i-2
		nxt = X.pentachoron(nxtIndx)
		prv = X.pentachoron(prvIndx)
		nxt.join(4,prv,Perm5(3,2,1,0,4))
		
	return X