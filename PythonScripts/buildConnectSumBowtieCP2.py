def cb(n):
	# Constructs a triangulation of the "Chain Ball" of size n.
	base = Triangulation3()
	base.insertLayeredSolidTorus(n,n+1)
	base.tetrahedron(0).join(2,base.tetrahedron(0),Perm4(0,1,3,2))
	res = Example4.singleCone(base)
	return res
	
cb2 = cb(2)	
tsb1 = Triangulation4('baa')
tsb1.pentachoron(0).join(0,tsb1.pentachoron(0),Perm5(1,0,2,3,4))
tsb1.pentachoron(0).join(2,tsb1.pentachoron(0),Perm5(0,1,3,2,4))
tsb2 = Triangulation4('baa')
tsb2.pentachoron(0).join(0,tsb2.pentachoron(0),Perm5(1,2,3,0,4))
tsb2.pentachoron(0).join(2,tsb2.pentachoron(0),Perm5(0,1,3,2,4))

def csBowtieCP2(n):
	X = Triangulation4(tsb2)
	X.insertTriangulation(cb2)
	X.pentachoron(2).join(4,X.pentachoron(0),Perm5(1,0,3,2,4))
	
	# Since we've already added 1 CP^2 summand, we only loop up to n-1.
	for i in range(n-1):
		X.insertTriangulation(cb2)
		X.pentachoron(2*i+4).join(4,X.pentachoron(2*i+1),Perm5(1,0,3,2,4))
	
	p = X.size()
	X.insertTriangulation(tsb1)
	X.pentachoron(p).join(4,X.pentachoron(p-2),Perm5(1,0,3,2,4))
	# ^ Cap off with a tsb2.
	
	return X