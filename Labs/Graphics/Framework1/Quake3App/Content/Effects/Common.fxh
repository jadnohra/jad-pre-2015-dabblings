#define Matrix_rm 0

#define Transform_rm(vectorReg, matrixReg, outVectorReg)\
	dp4 outVectorReg.x, c[matrixReg + 0], vectorReg		\
	dp4 outVectorReg.y, c[matrixReg + 1], vectorReg		\
	dp4 outVectorReg.z, c[matrixReg + 2], vectorReg		\
	dp4 outVectorReg.w, c[matrixReg + 3], vectorReg	

#define Transform_cm(vectorReg, matrixReg, outVectorReg, tempReg1, tempReg2)\
	mul tempReg1, c[matrixReg + 0], vectorReg.x	\
	mul tempReg2, c[matrixReg + 1], vectorReg.y	\
	add tempReg1, tempReg1, tempReg2			\
	mul tempReg2, c[matrixReg + 2], vectorReg.z	\
	add tempReg1, tempReg1, tempReg2			\
	mul tempReg2, c[matrixReg + 3], vectorReg.w	\
	add outVectorReg, tempReg1, tempReg2		