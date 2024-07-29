def connectSumS2xS2(n):
	base = Triangulation4('fALAIcaacdcdee+aoawbaaGa+ayaGa')
	block = Triangulation4('eAMHcaabcdd1ava1aDaaaJa')
	tsb2 = Triangulation4('bIaa2a5a')
	
	if n < 2:
		return Triangulation4()
	
	X = Triangulation4(base)
	
	for i in range(2,n+2):
		if i == 2:
			X.insertTriangulation(block)
			X.pentachoron(7).join(3,X.pentachoron(3),Perm5(1,2,0,4,3))
	
		if 2 < i and i < n+1:
			X.insertTriangulation(block)
			X.pentachoron(4*i-1).join(3,X.pentachoron(4*i-4),Perm5(0,1,2,3,4))
	
		if i == n+1:
			X.insertTriangulation(tsb2)
			X.pentachoron(4*n+1).join(1,X.pentachoron(4*n),Perm5(0,3,4,2,1))
	
	return X