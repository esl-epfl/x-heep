void riscv_scale_q31(
  q31_t * pSrc,
  q31_t scaleFract,
  int8_t shift,
  q31_t * pDst,
  uint32_t blockSize)
{
  int8_t kShift = shift + 1;                     /* Shift to apply after scaling */
  int8_t sign = (kShift & 0x80);
  uint32_t blkCnt;                               /* loop counter */
  q31_t in, out;



  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;


  if(sign == 0)
  {
	  while(blkCnt > 0u)
	  {
		/* C = A * scale */
		/* Scale the input and then store the result in the destination buffer. */
		in = *pSrc++;
		in = ((q63_t) in * scaleFract) >> 32;

		out = in << kShift;
		
		if(in != (out >> kShift))
			out = 0x7FFFFFFF ^ (in >> 31);

		*pDst++ = out;

		/* Decrement the loop counter */
		blkCnt--;
	  }
  }
  else
  {
	  while(blkCnt > 0u)
	  {
		/* C = A * scale */
		/* Scale the input and then store the result in the destination buffer. */
		in = *pSrc++;
		in = ((q63_t) in * scaleFract) >> 32;

		out = in >> -kShift;

		*pDst++ = out;

		/* Decrement the loop counter */
		blkCnt--;
	  }
  
  }
}