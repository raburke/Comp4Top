def chainPlumbing(n):
	# Constructs a triangulation of a linear plumbing
	# with all framings equal to -2 (referred to as A_n(-2)).
	
	if n<1:
		return None
	
	TSB2 = Triangulation4('bIaa2a5a')
	base = Triangulation3()
	
	# Build a LST(1,n+2,n+3).
	base.insertLayeredSolidTorus(1,n+2)
	
	t0 = base.tetrahedron(0)
	
	# Snap shut the LST into an (n+1)-tetrahedra layered 
	# triangulation of L(n+1,1).
	t0.join(2,t0,Perm4(0,1,3,2))
	
	# Cone the L(n+1,1).
	X = Example4.singleCone(base)
	
	for i in range(n+1):
		X.insertTriangulation(TSB2)
		pnxt = X.pentachoron(n+1+i)
		pprv = X.pentachoron(n-i)
		pnxt.join(1,pprv,Perm5(2,4,3,0,1))

	return X