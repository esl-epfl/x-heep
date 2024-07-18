#ifndef __X_HEEP_EMUL_H__
#define __X_HEEP_EMUL_H__

typedef   signed short v2s __attribute__((vector_size (4)));
typedef unsigned short v2u __attribute__((vector_size (4)));

typedef   signed char  v4s __attribute__((vector_size (4)));
typedef unsigned char  v4u __attribute__((vector_size (4)));

typedef signed char charV __attribute__((vector_size (4)));
typedef signed short shortV __attribute__((vector_size (4)));

typedef void * rt_pointerT;

#define DMA_COPY_IN 1
#define DMA_COPY_OUT 0

#define L2_MEM
#define L1_CL_MEM
#define L1_FC_MEM

#define RT_L1_DATA
#define RT_L1_GLOBAL_DATA
#define RT_L1_BSS
#define RT_L1_ALIAS_DATA
#define RT_L1_TINY_DATA
#define RT_FC_TINY_DATA
#define RT_FC_GLOBAL_DATA
#define RT_FC_SHARED_DATA
#define RT_L2_DATA
#define RT_L2_RET_DATA

#define RT_FC_DATA RT_FC_GLOBAL_DATA

/* Packing of scalars into vectors */
#define x_heep_pack2(x, y)		((v2s) {(signed short)   (x), (signed short)   (y)})
#define x_heep_packu2(x, y)		((v2u) {(unsigned short) (x), (unsigned short) (y)})

#define x_heep_pack4(x, y, z, t)		((v4s) {(signed char)   (x), (signed char)   (y), (signed char)   (z), (signed char)   (t)})
#define x_heep_packu4(x, y, z, t)		((v4u) {(unsigned char) (x), (unsigned char) (y), (unsigned char) (z), (unsigned char) (t)})

/* Max */
#define x_heep_max2(x, y) 		((v2s) {((signed short)(x)[0]>(signed short)(y)[0])?((signed short)(x)[0]):((signed short)(y)[0]), \
						((signed short)(x)[1]>(signed short)(y)[1])?((signed short)(x)[1]):((signed short)(y)[1])})
#define x_heep_max4(x, y) 		((v4s) {((signed char)(x)[0]>(signed char)(y)[0])?(signed char)(x)[0]:(signed char)(y)[0], \
						((signed char)(x)[1]>(signed char)(y)[1])?(signed char)(x)[1]:(signed char)(y)[1], \
						((signed char)(x)[2]>(signed char)(y)[2])?(signed char)(x)[2]:(signed char)(y)[2], \
						((signed char)(x)[3]>(signed char)(y)[3])?(signed char)(x)[3]:(signed char)(y)[3]})

#define x_heep_maxu2(x, y) 		((v2u) {((unsigned short)(x)[0]>(unsigned short)(y)[0])?(unsigned short)(x)[0]:(unsigned short)(y)[0], \
						((unsigned short)(x)[1]>(unsigned short)(y)[1])?(unsigned short)(x)[1]:(unsigned short)(y)[1]})
#define x_heep_maxu4(x, y) 		((v4u) {((unsigned char)(x)[0]>(unsigned char)(y)[0])?(unsigned char)(x)[0]:(unsigned char)(y)[0], \
						((unsigned char)(x)[1]>(unsigned char)(y)[1])?(unsigned char)(x)[1]:(unsigned char)(y)[1], \
						((unsigned char)(x)[2]>(unsigned char)(y)[2])?(unsigned char)(x)[2]:(unsigned char)(y)[2], \
						((unsigned char)(x)[3]>(unsigned char)(y)[3])?(unsigned char)(x)[3]:(unsigned char)(y)[3]})

/* Min */
#define x_heep_min2(x, y) 		((v2s) {((signed short)(x)[0]<(signed short)(y)[0])?((signed short)(x)[0]):((signed short)(y)[0]), \
						((signed short)(x)[1]<(signed short)(y)[1])?((signed short)(x)[1]):((signed short)(y)[1])})
#define x_heep_min4(x, y) 		((v4s) {((signed char)(x)[0]<(signed char)(y)[0])?(signed char)(x)[0]:(signed char)(y)[0], \
						((signed char)(x)[1]<(signed char)(y)[1])?(signed char)(x)[1]:(signed char)(y)[1], \
						((signed char)(x)[2]<(signed char)(y)[2])?(signed char)(x)[2]:(signed char)(y)[2], \
						((signed char)(x)[3]<(signed char)(y)[3])?(signed char)(x)[3]:(signed char)(y)[3]})

#define x_heep_minu2(x, y) 		((v2u) {((unsigned short)(x)[0]<(unsigned short)(y)[0])?(unsigned short)(x)[0]:(unsigned short)(y)[0], \
						((unsigned short)(x)[1]<(unsigned short)(y)[1])?(unsigned short)(x)[1]:(unsigned short)(y)[1]})
#define x_heep_minu4(x, y) 		((v4u) {((unsigned char)(x)[0]<(unsigned char)(y)[0])?(unsigned char)(x)[0]:(unsigned char)(y)[0], \
						((unsigned char)(x)[1]<(unsigned char)(y)[1])?(unsigned char)(x)[1]:(unsigned char)(y)[1], \
						((unsigned char)(x)[2]<(unsigned char)(y)[2])?(unsigned char)(x)[2]:(unsigned char)(y)[2], \
						((unsigned char)(x)[3]<(unsigned char)(y)[3])?(unsigned char)(x)[3]:(unsigned char)(y)[3]})

/* Clip */
#define x_heep_clip(x, precision) ((x)<(-(1<<(precision)))?(-(1<<(precision))):(((x)>((1<<(precision))-1))?((1<<(precision))-1):(x)))
#define x_heep_clipu(x, precision)	((x)<0)?0:(((x)>((1<<(precision))-1))?((1<<(precision))-1):(x))

/* Abs */
#define x_heep_abs2(x) 			((v2s) {((x)[0]<0)?-(x)[0]:(x)[0], ((x)[1]<0)?-(x)[1]:(x)[1]})
#define x_heep_abs4(x) 			((v4s) {((x)[0]<0)?-(x)[0]:(x)[0], ((x)[1]<0)?-(x)[1]:(x)[1], \
						((x)[2]<0)?-(x)[2]:(x)[2], ((x)[3]<0)?-(x)[3]:(x)[3]})

/* Mac */
#define	x_heep_macs(Acc, x, y)		((Acc) + ((short int) (x) * (short int) (y)))
#define	x_heep_machhs(Acc, x, y)		((Acc) + ((short int) ((x)>>16) * (short int) ((y)>>16)))
#define	x_heep_macu(Acc, x, y)		((Acc) + ((unsigned short int) (x) * (unsigned short int) (y)))
#define	x_heep_machhu(Acc, x, y)		((Acc) + ((unsigned short int) ((x)>>16) * (unsigned short int) ((y)>>16)))

#define	x_heep_macsN(Acc, x, y, n)	(((Acc) + ((short int) (x) * (short int) (y)))>>(n))
#define	x_heep_macuN(Acc, x, y, n)	(((Acc) + ((unsigned short int) (x) * (unsigned short int) (y)))>>(n))
#define	x_heep_macsRN(Acc, x, y, n)	((((Acc) + ((short int) (x) * (short int) (y))) + (1<<((n)-1))) >> (n))
#define	x_heep_macuRN(Acc, x, y, n)	((((Acc) + ((unsigned short int) (x) * (unsigned short int) (y))) + (1<<((n)-1))) >> (n))

#define	x_heep_machhsN(Acc, x, y, n)	(((Acc) + ((short int) ((x)>>16) * (short int) ((y)>>16))) >> (n))
#define	x_heep_machhuN(Acc, x, y, n)	(((Acc) + ((unsigned short int) ((x)>>16) * (unsigned short int) ((y)>>16))) >> (n))
#define	x_heep_machhsRN(Acc, x, y, n)	((((Acc) + ((short int) ((x)>>16) * (short int) ((y)>>16))) + (1<<((n)-1))) >> (n))
#define	x_heep_machhuRN(Acc, x, y, n)	((((Acc) + ((unsigned short int) ((x)>>16) * (unsigned short int) ((y)>>16))) + (n)))

/* Multiplications */
#define x_heep_muls(x, y)			(((short int) (x) * (short int) (y)))
#define x_heep_mulu(x, y)			(((unsigned short int) (x) * (unsigned short int) (y)))
#define x_heep_mulhhs(x, y)		(((short int) ((x)>>16) * (short int) ((y)>>16)))
#define x_heep_mulhhu(x, y)		(((unsigned short int) ((x)>>16) * (unsigned short int) ((y)>>16)))
#define x_heep_mulsN(x, y, n)		(((short int) (x) * (short int) (y))>>(n))
#define x_heep_mulsRN(x, y, n)		((((short int) (x) * (short int) (y)) + (1<<((n)-1)))>>(n))
#define x_heep_muluN(x, y, n)		(((unsigned short int) (x) * (unsigned short int) (y))>>(n))
#define x_heep_muluRN(x, y, n)		((((unsigned short int) (x) * (unsigned short int) (y)) + (1<<((n)-1)))>>(n))

/* Vectorial product and sum of products */
#define x_heep_dotp2(x, y)		(    (x)[0]*(y)[0] + (x)[1]*(y)[1])
#define x_heep_dotpu2(x, y)		(    (x)[0]*(y)[0] + (x)[1]*(y)[1])
#define x_heep_dotpus2(x, y)		(    (x)[0]*(y)[0] + (x)[1]*(y)[1])

#define x_heep_sumdotp2(x, y, z)		((z)+(x)[0]*(y)[0] + (x)[1]*(y)[1])
#define x_heep_sumdotpu2(x, y, z)		((z)+(x)[0]*(y)[0] + (x)[1]*(y)[1])
#define x_heep_sumdotpus2(x, y, z)	((z)+(x)[0]*(y)[0] + (x)[1]*(y)[1])

#define x_heep_dotp4(x, y)		(    (x)[0]*(y)[0] + (x)[1]*(y)[1] + (x)[2]*(y)[2] + (x)[3]*(y)[3])
#define x_heep_dotpu4(x, y)		(    (x)[0]*(y)[0] + (x)[1]*(y)[1] + (x)[2]*(y)[2] + (x)[3]*(y)[3])
#define x_heep_dotpus4(x, y)		(    (x)[0]*(y)[0] + (x)[1]*(y)[1] + (x)[2]*(y)[2] + (x)[3]*(y)[3])

#define x_heep_sumdotp4(x, y, z)		((z)+(x)[0]*(y)[0] + (x)[1]*(y)[1] + (x)[2]*(y)[2] + (x)[3]*(y)[3])
#define x_heep_sumdotpu4(x, y, z)		((z)+(x)[0]*(y)[0] + (x)[1]*(y)[1] + (x)[2]*(y)[2] + (x)[3]*(y)[3])
#define x_heep_sumdotpus4(x, y, z)	((z)+(x)[0]*(y)[0] + (x)[1]*(y)[1] + (x)[2]*(y)[2] + (x)[3]*(y)[3])


/* Complex Multiplication, Q15x15 into Q15, with optional post scaling by 1 or 2 */
#define x_heep_cplxmuls(x, y)		((v2s) {(signed short) ((((long long) (x)[0]*(long long) (y)[0]) - ((long long) (x)[1]*(long long) (y)[1]))>>15),  \
						(signed short) ((((long long) (x)[0]*(long long) (y)[1]) + ((long long) (x)[1]*(long long) (y)[0]))>>15)})
#define x_heep_cplxmulsdiv2(x, y)		(x_heep_cplxmuls(x, y)>>(v2s){1,1})
#define x_heep_cplxmulsdiv4(x, y)		(x_heep_cplxmuls(x, y)>>(v2s){2,2})

/* Complex conjugate */
#define x_heep_cplxconj(x)		((v2s) {(x)[0], -(x)[1]})

/* Complex rotation by -pi/2 */
#define x_heep_cplxmj(x)			((v2s) {(x)[1], -(x)[0]})

/* Complex substration, result rotated by -pi/2 */
#define x_heep_sub2rotmj(x, y)		((v2s) {(x)[1]-(y)[1], (y)[0]-(x)[0]})

/* Complex addition with post scaling by 1 or 2 */
#define x_heep_add2div2(x, y)		(((x)+(y))>>(v2s) {1, 1})
#define x_heep_add2div4(x, y)		(((x)+(y))>>(v2s) {2, 2})

/* Complex subtraction with post scaling by 1 or 2 */
#define x_heep_sub2div2(x, y)		(((x)-(y))>>(v2s) {1, 1})
#define x_heep_sub2div4(x, y)		(((x)-(y))>>(v2s) {2, 2})

/* Viterbi Max and Viterbi Select, pair of Q15 */
#define x_heep_vitmax(x, y) 		(_VitT1_Flag=((x)[1]<=(y)[1])?1:0, _VitT0_Flag=((x)[0]<=(y)[0])?1:0,\
				 	(v2s) {((x)[0]>(y)[0])?(x)[0]:(y)[0], ((x)[1]>(y)[1])?(x)[1]:(y)[1]})
#define x_heep_vitsel(x, y) 		(v2s) {(_VitT0_Flag?(y)[0]:(x)[0])<<1|_VitT0_Flag, (_VitT1_Flag?(y)[1]:(x)[1])<<1|_VitT1_Flag}

/* Position of the most significant bit of x */
#define x_heep_fl1(x)			(31 - __builtin_clz((x)))

/* Number of sign bits */
#define x_heep_clb(x)			(__builtin_clrsb((x)))

/* Bit Extraction */
#define x_heep_bitextract(x, size, off) 		(((((x)>>(off))&((unsigned int)(1<<(size))-1))<<(32-(size)))>>(32-(size)))
#define x_heep_bitextractu(x, size, off)		(((x)>>(off))&((unsigned int)(1<<(size))-1))

#define x_heep_bitextract_r(x, size, off) 	(((((x)>>(off))&((unsigned int)(1<<(size))-1))<<(32-(size)))>>(32-(size)))
#define x_heep_bitextractu_r(x, size, off)	(((x)>>(off))&((unsigned int)(1<<(size))-1))

#define x_heep_bitextract_r_safe(x, size, off) 	(((((x)>>((off)&0x1F))&((unsigned int)(1<<((((size)>32)?32:(size))))-1))<<(32-((((size)>32)?32:(size)))))>>(32-((((size)>32)?32:(size)))))
#define x_heep_bitextractu_r_safe(x, size, off)	(((x)>>((off)&0x1F))&((unsigned int)(1<<((((size)>32)?32:(size))))-1))

/* Bit insertion */
#define x_heep_bitinsert(dst, src, size, off) 	(((dst) & ~(((1<<(size))-1)<<(off))) | (((src) & ((1<<(size))-1))<<(off)))
#define x_heep_bitinsert_r(dst, src, size, off) 	(((dst) & ~(((1<<(size))-1)<<(off))) | (((src) & ((1<<(size))-1))<<(off)))
#define x_heep_bitinsert_r_safe(dst, src, size, off) 	(((dst) & ~(((1<<(((size)>32)?32:(size)))-1)<<((off)&0x1F))) | (((src) & ((1<<(((size)>32)?32:(size)))-1))<<((off)&0x1F)))

/* 1 bit rotation to the right, 32 bits input */
#define x_heep_rotr(x)			((((x)>>1)&0x7FFFFFFF) | ((x)<<31))

/* Add with normalization and rounding */
#define x_heep_addroundnormu(x, y, scale)		((unsigned int)((x) + (y) + (1<<((scale)-1)))>>(scale))
#define x_heep_addroundnormu_reg(x, y, scale)	((unsigned int)((x) + (y) + (1<<((scale)-1)))>>(scale))
#define x_heep_addroundnorm(x, y, scale)		((int)((x) + (y) + (1<<((scale)-1)))>>(scale))
#define x_heep_addroundnorm_reg(x, y, scale)	((int)((x) + (y) + (1<<((scale)-1)))>>(scale))

/* Normalization and rounding */
#define x_heep_roundnormu(x, scale)		((unsigned int)((x) + (1<<((scale)-1)))>>(scale))
#define x_heep_roundnormu_reg(x, scale)		((unsigned int)((x) + (1<<((scale)-1)))>>(scale))
#define x_heep_roundnorm(x, scale)		((int)((x) + (1<<((scale)-1)))>>(scale))
#define x_heep_roundnorm_reg(x, scale)		((int)((x) + (1<<((scale)-1)))>>(scale))

/*add*/
#define x_heep_add2(x,y) ((shortV) {(x)[0]+(y)[0], (x)[1]+(y)[1]})
/*sub*/
#define x_heep_sub2(x,y) ((shortV) {(x)[0]-(y)[0], (x)[1]-(y)[1]})

/*Neg*/
#define x_heep_neg2(x)(shortV) {-(x)[0], -(x)[1]}
/*SRA*/
#define x_heep_sra2(x,y) (shortV){((signed short)(x)[0]>>(signed short)(y)[0]), ((signed short)(x)[1]>>(signed short)(y)[1])}
#define x_heep_sra4(x,y) (v4s) {((signed char)(x)[0]>>(signed char)(y)[0]), ((signed char)(x)[1]>>(signed char)(y)[1]), ((signed char)(x)[2]>>(signed char)(y)[2]), ((signed char)(x)[3]>>(signed char)(y)[3])}
 /*sll2*/
 #define x_heep_sll2(x,y)	(v2s){(x)[0]<<(y)[0], (x)[1]<<(y)[1]}
 
#endif
