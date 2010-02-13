
namespace cpuid
{
	// http://en.wikipedia.org/wiki/CPUID
	// http://softpixel.com/~cwright/programming/simd/cpuid.php

	struct VendorId
	{
		VendorId()
		{
			int b,c,d;
#ifdef _WIN32
				__asm 
				{
					mov eax, 0
					cpuid
					mov b, ebx
					mov c, ecx
					mov d, edx
				}
#else
			  //__asm ( "mov %1, %%eax; " // a into eax
			  //	 "cpuid;"
			  //  "mov %%eax, %0;" // eeax into b
			  //	  :"=r"(b) /* output */
			  //	  :"r"(a) /* input */
			  //	  :"%eax" /* clobbered register */
			  //	 );
#endif

			*((int*) mChars) = b;
			*((int*) ((char*)mChars+sizeof(int))) = c;
			*((int*) ((char*)mChars+2*sizeof(int))) = d;
			mChars[(sizeof(mChars) / sizeof(char))-1] = 0;
		}

		const char* GetId() { return mChars; }

		char mChars[13];
	};

	struct ProcessorInfoAndFeatures
	{
		int mBits[4];

		ProcessorInfoAndFeatures()
		{
#ifdef _WIN32
				__asm 
				{
					mov eax, 1
					cpuid
					mov mBits[0], eax
					mov mBits[1], ebx
					mov mBits[2], ecx
					mov mBits[3], edx
				}
#else
		}

		// int GetFamily();

	};
}

