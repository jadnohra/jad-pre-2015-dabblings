#include <conio.h>
#include <stdio.h>
#include <xmmintrin.h>
#include <smmintrin.h>

// http://www.ccsl.carleton.ca/~jamuir/rdtscpm1.pdf
static unsigned __int64 __forceinline GetTicks()
{
	__asm
	{
		XOR eax, eax
			CPUID
			RDTSC
	}
}
static unsigned __int64 __forceinline CalcTicksFuncOverhead()
{
	int reps = 1000;
	__int64 oh = 0;

	for ( int i = 0; i < reps; ++i )
	{
		__int64 dt = GetTicks();
		dt = GetTicks() - dt;
		oh += dt;
	}

	return oh / reps;
}
static __int64 __forceinline GetTicksFuncOverhead()
{
	static __int64 oh = CalcTicksFuncOverhead();
	return oh;
}
#define START_TIMER(A) __int64 A = GetTicks();
#define END_TIMER(A) A = (GetTicks()-A)-GetTicksFuncOverhead();
#define PRINT_TIMER(A) printf(#A": %d\n", (int) A);
#define END_PRINT_TIMER(A) END_TIMER(A); PRINT_TIMER(A);

#define SSP_FORCEINLINE __forceinline
typedef   signed char      ssp_s8;
typedef unsigned char      ssp_u8;
typedef   signed short     ssp_s16;
typedef unsigned short     ssp_u16;
typedef   signed int       ssp_s32;
typedef unsigned int       ssp_u32;
typedef float              ssp_f32;
typedef double             ssp_f64;
typedef   signed long long ssp_s64;
typedef unsigned long long ssp_u64;
typedef union
{
	__m128  f;
	__m128d d;
	__m128i i;
	__m64       m64[ 2];
	ssp_u64 u64[ 2];
	ssp_s64 s64[ 2];
	ssp_f64 f64[ 2];
	ssp_u32 u32[ 4];
	ssp_s32 s32[ 4];
	ssp_f32 f32[ 4];
	ssp_u16 u16[ 8];
	ssp_s16 s16[ 8];
	ssp_u8  u8 [16];
	ssp_s8  s8 [16];
} ssp_m128;
SSP_FORCEINLINE __m128i ssp_abs_epi8_REF( __m128i a )
{
	ssp_m128 A;
	A.i = a;

	A.s8[0]  = ( A.s8[0] < 0 ) ? -A.s8[0]  : A.s8[0];
	A.s8[1]  = ( A.s8[1] < 0 ) ? -A.s8[1]  : A.s8[1];
	A.s8[2]  = ( A.s8[2] < 0 ) ? -A.s8[2]  : A.s8[2];
	A.s8[3]  = ( A.s8[3] < 0 ) ? -A.s8[3]  : A.s8[3];
	A.s8[4]  = ( A.s8[4] < 0 ) ? -A.s8[4]  : A.s8[4];
	A.s8[5]  = ( A.s8[5] < 0 ) ? -A.s8[5]  : A.s8[5];
	A.s8[6]  = ( A.s8[6] < 0 ) ? -A.s8[6]  : A.s8[6];
	A.s8[7]  = ( A.s8[7] < 0 ) ? -A.s8[7]  : A.s8[7];
	A.s8[8]  = ( A.s8[8] < 0 ) ? -A.s8[8]  : A.s8[8];
	A.s8[9]  = ( A.s8[9] < 0 ) ? -A.s8[9]  : A.s8[9];
	A.s8[10] = ( A.s8[10] < 0 ) ? -A.s8[10] : A.s8[10];
	A.s8[11] = ( A.s8[11] < 0 ) ? -A.s8[11] : A.s8[11];
	A.s8[12] = ( A.s8[12] < 0 ) ? -A.s8[12] : A.s8[12];
	A.s8[13] = ( A.s8[13] < 0 ) ? -A.s8[13] : A.s8[13];
	A.s8[14] = ( A.s8[14] < 0 ) ? -A.s8[14] : A.s8[14];
	A.s8[15] = ( A.s8[15] < 0 ) ? -A.s8[15] : A.s8[15];
	return A.i;
}
__m128i ssp_abs_epi8_SSE2( __m128i a )
{
	__m128i mask = _mm_cmplt_epi8( a, _mm_setzero_si128() );  // FFFF   where a < 0
	__m128i one  = _mm_set1_epi8( 1 );
	a    = _mm_xor_si128( a, mask );                          // Invert where a < 0
	mask = _mm_and_si128( mask, one );                        // 0001   where a < 0
	a    = _mm_add_epi8( a, mask );                           // Add 1  where a < 0
	return a;
}



int main1()
{
	printf( "OH: %d\n", ( int ) GetTicksFuncOverhead() );
	START_TIMER( OH_CHECK );
	END_PRINT_TIMER( OH_CHECK );

	__m128 vcrc;
	//vcrc = _mm_add_ss(v3, vcrc);

	__m128 v0 = _mm_setr_ps( 0.0f, 0.0f, 0.0f, 0.0f );
	__m128 v1 = _mm_setr_ps( 2.0f, 3.0f, 4.0f, 5.0f );
	__m128 v2 = _mm_setr_ps( 10.0f, 20.0f, 30.0f, 40.0f );

	{
		__m128 v3 = _mm_shuffle_ps( v2, v1, _MM_SHUFFLE( 3, 2, 1, 0 ) );
		__m128 v4 = _mm_shuffle_ps( v1, v2, _MM_SHUFFLE( 0, 1, 2, 3 ) );

		__m128 v5 = _mm_move_ss( v1, v2 );

		__m128 v9 = _mm_shuffle_ps( v1, v2, _MM_SHUFFLE( 2, 2, 3, 3 ) );
		__m128 v10 = _mm_shuffle_ps( v2, v9, _MM_SHUFFLE( 0, 2, 1, 0 ) );

		__m128 v11 = _mm_blend_ps( v2, v1, 1 << 3 );

		v1 = v1;
	}



	{
		__m128 v3 = _mm_shuffle_ps( v1, v2, _MM_SHUFFLE( 0, 0, 3, 3 ) );
		__m128 v4 = _mm_shuffle_ps( v3, v2, _MM_SHUFFLE( 2, 1, 2, 1 ) );

		v1 = v1;
	}


	getch();
	float mret[4];
	_mm_store_ss( mret, vcrc );
	return ( int )( mret[0] + mret[1] + mret[2] + mret[3] );
}



int main2()
{

	int cnt = 30;
	int i1 = 1 << cnt;

	cnt = 32;
	int i2 = 1 << cnt;

	int i2_ = 1;
	for (int c=0; c< cnt; ++c)
		i2_ = i2_ << 1;


	cnt = 33;
	int i3 = 1 << cnt;

	int i3_ = 1;
	for (int c=0; c< cnt; ++c)
		i3_ = i3_ << 1;


	return 0;
}


struct x86FPUState
{
	typedef unsigned short WORD;
	typedef unsigned int DWORD;

	struct TBYTE
	{
		unsigned char bytes[10];
	};

	DWORD controlWord;
	DWORD statusWord;
	DWORD tagWord;
	DWORD instructionPointer;
	DWORD codeSegment;
	DWORD operandAddress;
	DWORD dataSegment;
	TBYTE registers[8];
};


x86FPUState getx86FPUState()
{
	x86FPUState state;
	__asm
	{
		fsave state
	}

	return state;
}

const char* hex2bin(unsigned char hex)
{
	char bstr[9]; bstr[8] = 0;

	int i = 0;
	while (hex != 0)
	{
		bstr[i++] = (hex & 0x80) ? '1' : '0';
		hex = hex << 1;
	}
	while (i < 8)
		bstr[i++] = '0';

	return bstr;
}

const char* x86tbyte2bin(x86FPUState::TBYTE& tbyte)
{
	char bstr[81]; bstr[80] = 0;

	int bi = 0;
	for (int i = 9; i >= 0; --i)
	{
		const char* temp = hex2bin( tbyte.bytes[i] );

		for (int j = 0; j < 8; ++j)
			bstr[bi++] = temp[j];
	}

	return bstr;
}

const char* hex2bin80(const char* hexPtr)
{
	struct bit80
	{
		unsigned long long hex1;
		unsigned short hex2;
	};

	char bstr[81]; bstr[80] = 0;
	unsigned long long mask1 = 0x8000000000000000;
	unsigned long long mask2 = 0x8000;

	bit80 hex; hex = *((bit80*) hexPtr);

	int i = 0;
	while (hex.hex2 != 0)
	{
		bstr[i++] = (hex.hex2 & mask2) ? '1' : '0';
		hex.hex2 = hex.hex2 << 1;
	}
	while (i < 16)
		bstr[i++] = '0';

	while (hex.hex1 != 0)
	{
		bstr[i++] = (hex.hex1 & mask1) ? '1' : '0';
		hex.hex1 = hex.hex1 << 1;
	}
	while (i < 80)
		bstr[i++] = '0';

	return bstr;
}


const char* hex2bin64(unsigned long long hex)
{
	char bstr[65]; bstr[64] = 0;
	unsigned long long mask = 0x8000000000000000;

	int i = 0;
	while (hex != 0)
	{
		bstr[i++] = (hex & mask) ? '1' : '0';
		hex = hex << 1;
	}
	while (i < 64)
		bstr[i++] = '0';

	return bstr;
}


const char* hex2bin(unsigned int hex)
{
	char bstr[33]; bstr[32] = 0;

	int i = 0;
	while (hex != 0)
	{
		bstr[i++] = (hex & 0x80000000) ? '1' : '0';
		hex = hex << 1;
	}
	while (i < 32)
		bstr[i++] = '0';

	return bstr;
}

unsigned int bin2hex(const int* setBits, int negOffset = 0)
{
	unsigned int hh = 0; int i = 0;

	while (setBits != NULL && setBits[i] != 0)
	{
		int shift = setBits[i] >= 0 ? setBits[i] : setBits[i] + negOffset;
		hh = hh | 1 << shift;
		++i;
	}

	return hh;
}

unsigned int bin2hex(const char* bstr)
{
	unsigned int hh = 0; int i = 0; 
	int bit = 1;

	while (bstr && bstr[i++] != 0);
	i -= 2;
	while (i >= 0)
	{
		if (bstr[i--] == '1')
			hh = hh | bit;
		bit = bit << 1;
	}

	return hh;
}

float hex2f(unsigned int hex)
{
	float f; *((unsigned int*) ((void*) &f)) = hex;
	return f;
}


float bin2f32(int* setBits)
{
	return hex2f(bin2hex(setBits));
}

float bin2f32(const char* bstr)
{
	return hex2f(bin2hex(bstr));
}

float bin2f32(const char* sign, const char* exp, const char* mant)
{
	const char* sign2 = sign; if (sign[0] == '+') sign2 = "0"; if (sign[0] == '-') sign2 = "1";

	return hex2f( (bin2hex(sign2) << 31) |  (bin2hex(exp) << 23) |  (bin2hex(mant)) );
}

float bin2f32(const int* setBitsSign, const int* setBitsExp, const int* setBitsMant)
{
	return hex2f( (bin2hex(setBitsSign) << 31) |  (bin2hex(setBitsExp) << 23) |  (bin2hex(setBitsMant, 23)) );
}

float bin2f32(int sign, int exp, const int* setBitsMant)
{
	sign = (sign >= 0 ? 1 : 0);

	return hex2f( (sign << 31) |  ( ((exp+127) & 0xFF) << 23) |  (bin2hex(setBitsMant, 23)) );
}

float bin2f32(const char* sign, int exp, const int* setBitsMant)
{
	const char* sign2 = sign; if (sign[0] == '+') sign2 = "0"; if (sign[0] == '-') sign2 = "1";

	return hex2f( (bin2hex(sign2) << 31) |  ( ((exp+127) & 0xFF) << 23) |  (bin2hex(setBitsMant, 23)) );
}

float bin2f32(int sign, int exp, const char* mant)
{
	sign = (sign >= 0 ? 1 : 0);

	return hex2f( (sign << 31) |  ( ((exp+127) & 0xFF) << 23) |  (bin2hex(mant)) );
}

float bin2f32(const char* sign, int exp, char* mant)
{
	const char* sign2 = sign; if (sign[0] == '+') sign2 = "0"; if (sign[0] == '-') sign2 = "1";

	return hex2f( (bin2hex(sign2) << 31) |  ( ((exp+127) & 0xFF) << 23) |  (bin2hex(mant)) );
}

const char* f2bin(float f)
{
	return hex2bin( *((unsigned int*) &f) );
}




#include <float.h>
int main3()
{

	float f1 = bin2f32("0", "01111111", "");
	float f2 = bin2f32("0", "01111111", "1");
	float f3 = bin2f32("0", "01111111", "01");
	float f4 = bin2f32(1, 0, "");
	int mant[] = {22, 0};
	float f5 = bin2f32(1, 0, mant);
	float f6 = bin2f32("+", -1, "");

	const char* bstr1 = hex2bin((unsigned int)255);
	const char* bstr2 = f2bin(-0.5f);

	{
		float minf = bin2f32("+", -127, "1");
		float t1 = minf * 0.5f;
		float t2 = t1 * 2.0f;

		float minfp = t2;
	}

	{
		//_controlfp(_PC_64, _MCW_PC );
		//_controlfp(_PC_53, _MCW_PC );
		//_controlfp(_PC_24, _MCW_PC );



		float minf = bin2f32("+", -127, "1");
		float half = 0.5f;
		float two = 2.0f;
		float outf = 0.0f;
		float zero = 0.0f;
		float temp = -0.0f;

		__asm
		{
			fld temp
				fld zero
				fld two
				fld half
				fld minf
		}

		// 		__asm
		// 		{
		// 			fmul st, st(1)
		// 			//fmul st, st(2)
		// 		}
		// 
		//  		for (int i = 0; i < 56; ++i)
		//  		{
		//  			__asm
		//  			{
		//  				fmul st, st(1)
		// 				fadd st, st(3)
		// 				fst st(4)	// save in st(4)
		// 				//fld st(4)
		// 				
		// 				//fadd st(4), st(3)	// force rounding, by adding to zero and moving to st(4)
		// 				//fxch st				// move rounded result back to st
		//  			}
		//  		}

		//32: -127-22=-149
		//64: 
		//79: -16383-22=-16405
		//80: -32767-22=-32789
		outf = -1.0f;
		int i = 0;
		while (outf != 0.0f)
		{
			__asm
			{
				fmul st, st(1)
			}


			_controlfp(_RC_UP, _MCW_RC );
			__asm
			{
				fst outf
			}
			_controlfp(_RC_CHOP, _MCW_RC );


			if (outf == 0.0f)
			{
				printf("%d\n", i);
			}
			++i;
		}

		__asm
		{
			fstp outf
				fstp st
				fstp st
				fstp st
				fstp st
		}

		float t1 = minf * 0.5f;
		float t2 = t1 * 2.0f;

		float minfp = t2;
	}

	{
		float outs[3][40000];
		int max[3];

		for (int ti = 0; ti < 3; ++ti)
		{
			if (ti == 0)
				_controlfp(_PC_64, _MCW_PC );
			if (ti == 1)
				_controlfp(_PC_53, _MCW_PC );
			if (ti == 2)
				_controlfp(_PC_24, _MCW_PC );

			float minf = bin2f32("+", -127, "1");
			float half = 0.5f;
			float outf = 0.0f;

			__asm
			{
				fld half
					fld minf
			}

			outf = -1.0f;
			int i = 0;
			while (outf != 0.0f)
			{
				__asm
				{
					fmul st, st(1)
				}
				_controlfp(_RC_UP, _MCW_RC );
				__asm
				{
					fst outf
				}
				_controlfp(_RC_CHOP, _MCW_RC );

				outs[ti][i] = outf;
				if (outf == 0.0f)
				{
					max[ti] = i;
				}
				++i;
			}

			__asm
			{
				fstp outf
					fstp st
			}
		}

		{
			x86FPUState st1 = getx86FPUState();
			const char* tt = x86tbyte2bin(st1.registers[4]);
			tt;
		}


		_fpreset();
	}




	return 0;
}


int main()
{
	return main3();
}