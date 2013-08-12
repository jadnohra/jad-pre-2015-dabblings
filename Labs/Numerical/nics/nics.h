#ifndef NICS_H
#define NICS_H


#pragma warning( push )
#pragma warning( disable : 4996 )
#pragma warning( disable : 4003 )
#pragma warning( disable : 4291 )	
#include "../flann/src/cpp/flann/flann.hpp"
#if 0
//#pragma comment( lib, "../../flann/build/lib/Release/flann_cpp_s")
#else
#pragma comment( lib, "../../flann/build/lib/Debug/flann_cpp_s")
#endif

#include "../randomc/randomc.h"
#pragma comment( lib, "../../randomc/randomc/release/randomc")

namespace nics
{
	template<typename Graph, typename Vertex, typename DistT>
	struct Rrt
	{
		typedef Vertex(*randConfFunc)(void* ctx);
		typedef Vertex(*newConfFunc)(void* ctx, const Vertex& v, const DistT& dq);
		typedef Vertex(*nearestFunc)(void* ctx, const Vertex& v, Graph& G);
		typedef void(*addFunc)(void* ctx, Graph& G, const Vertex& from, const Vertex& to);

		static Vertex next(void* ctx, Graph& G, DistT dq, randConfFunc randConf, nearestFunc nearest, newConfFunc newConf, addFunc add)
		{
			Vertex qrand = randConf(ctx);
			Vertex qnear = nearest(ctx, qrand, G);
			Vertex qnew = newConf(ctx, qnear, dq);
			add(ctx, G, qnear, qnew);	
			return qnew;
		}
	};

	template<typename T>
	struct Array
	{
		struct ChainEl
		{
			T* el;
			unsigned char* valid;
			ChainEl* next;
			int first_invalid;
		};

		typedef void(*dtorFunc)(void* ctxt, T* el);

		static void _dtor(void* ctxt, ChainEl* cel, int size, dtorFunc dtor)
		{
			if (cel)
			{
				if (cel->next) _dtor(ctxt, cel->next, size, dtor);
				if (dtor) { for (int i=0;i<size;++i) if (cel->valid[i]) dtor(ctxt, cel->el+i); }
				free(cel->el);
				free(cel->valid);
				free(cel);
			}
		}

		void* ctxt;
		ChainEl* cel;
		int chain_count;
		int chain_size;
		dtorFunc dtor;
		
		Array(int chain_size_ = 32,dtorFunc dtor_ = 0) : ctxt(0), cel(0), chain_count(0), chain_size(chain_size_), dtor(dtor_) {}
		~Array() { _dtor(ctxt, cel, chain_size, dtor); }

		ChainEl* alloc()
		{
			ChainEl* cel = (ChainEl*) malloc(sizeof(ChainEl));
			cel->first_invalid = 0;
			cel->el = (T*) malloc(chain_size*sizeof(T));
			cel->valid = (unsigned char*) malloc(chain_size*sizeof(unsigned char));
			for (int i=0;i<chain_size;++i) cel->valid[i]=0;
			cel->next = 0;
			return cel;
		}

		struct Add
		{
			Array& arr;
			ChainEl* cel;
			int i;

			Add(Array& arr_)
			: arr(arr_), cel(arr_.cel)
			{
				if (cel) while (cel->first_invalid >= arr.chain_size && cel->next) cel = cel->next;
				i = (cel ? cel->first_invalid : -1);
			}

			T* add()
			{
				if (cel == 0)
				{
					cel = arr.cel = arr.alloc(); i=0; cel->first_invalid = i+1; cel->valid[i]=1; return &cel->el[i++];
				}
				
				while(1)
				{
					while(i<arr.chain_size && cel->valid[i]) i++;
					if (i+1>=arr.chain_size)
					{
						cel->first_invalid = i+1; cel = cel->next = arr.alloc(); i=0; cel->valid[i]=1; cel->first_invalid = i+1; return &cel->el[i++];
					}
					else
					{
						cel->first_invalid = i+1; cel->valid[i]=1; return &cel->el[i++];
					}
				}
			}
		};

		struct Iter
		{
			Array& arr;
			ChainEl* cel;
			int i;

			Iter(Array& arr_)
			: arr(arr_), cel(arr_.cel), i(0) {}

			T* next()
			{
				while(cel)
				{
					while(i<arr.chain_size && !cel->valid[i]) i++;
					if (i+1>=arr.chain_size)
					{
						cel = cel->next; i=0;
					}
					else
					{
						return &cel->el[i++];
					}
				}
				return 0;
			}
		};
	};

	struct CubeNd
	{
		template <typename T, int Dim, int Di>
		struct Impl
		{
			static void build_cube_rec(T* cube, T radius, int& index)
			{
				int i;
				
				i= index;
				Impl<T, Dim, Di+1>::build_cube_rec(cube, radius, index);
				while(i < index) cube[(i++)*Dim+Di] = -radius;

				i= index;
				Impl<T, Dim, Di+1>::build_cube_rec(cube, radius, index);
				while(i < index) cube[(i++)*Dim+Di] = radius;
			}
		};
		template <typename T, int Dim>
		struct Impl<T, Dim, Dim>
		{
			static void build_cube_rec(T* cube, T radius, int& index) { index++; }
		};

		template <typename T, int Dim>
		static void build_cube(T* cube, T radius)
		{
			int index = 0;
			Impl<T, Dim, 0>::build_cube_rec(cube, radius, index);
		}
	};

	struct ChoiceNd
	{
		template <int Dim, int Di>
		struct Impl
		{
			static void build_choices_rec(int* cube, int& index)
			{
				int i;
				
				i= index;
				Impl<Dim, Di+1>::build_choices_rec(cube, index);
				while(i < index) cube[(i++)*Dim+Di] = -1;

				i= index;
				Impl<Dim, Di+1>::build_choices_rec(cube, index);
				while(i < index) cube[(i++)*Dim+Di] = 0;

				i= index;
				Impl<Dim, Di+1>::build_choices_rec(cube, index);
				while(i < index) cube[(i++)*Dim+Di] = 1;
			}
		};
		template <int Dim>
		struct Impl<Dim, Dim>
		{
			static void build_choices_rec(int* cube, int& index) { index++; }
		};

		template <int Dim>
		static void build_choices(int* cube)
		{
			int index = 0;
			Impl<Dim, 0>::build_choices_rec(cube, index);
		}
	};

	template<typename T, int Dim>
	struct Rrt_Rn
	{
		enum { D = Dim };
		typedef T Type;
		typedef T Scalar;
		typedef Rrt_Rn<T, Dim> This;
		typedef flann::Matrix<T> Matrix;
		typedef flann::L2<T> DistAlgo;
		typedef flann::Index<DistAlgo> Index;
		typedef T* Vertex;
		typedef Rrt<Index, Vertex, T> RrtImpl;

		struct Point { T v[Dim]; };

		Index* index;
		T bound_min[Dim];
		T bound_max[Dim];
		T vertex_rand[Dim];
		T vertex_new[Dim];
		T dq;
		typedef Array<Point> Points;
		Points points;

		flann::Matrix<size_t> query_indices;
		flann::Matrix<T> query_dists;
		flann::SearchParams query_params;

		CRandomMersenne random;

		Rrt_Rn() : index(0), random(0), query_indices(new size_t[1*Dim], 1, Dim), query_dists(new T[1*Dim], 1, Dim), points(1024, 0) {}
		~Rrt_Rn() { delete index; }

		void setBound(int di, T min, T max) { bound_min[di] = min; bound_max[di] = max; }
		void setBounds(T min, T max) { for (int i=0;i<Dim;++i) setBound(i, min, max); }

		Vertex init(T min, T max, T dq_)
		{
			dq = dq_;
			setBounds(min, max);

			//flann::AutotunedIndexParams params; // this causes problems with getPoint
			//flann::KMeansIndexParams params;	// crashes in rtt_test1 at index 2560
			flann::KDTreeIndexParams params;
			//flann::LinearIndexParams params;
			//flann::CompositeIndexParams params;
			
			Points::Add add(points); Vertex v = add.add()->v;
			for (int i=0; i<Dim; ++i) v[i] = bound_min[i] + T(0.5) * (bound_max[i]-bound_min[i]);
			
			Matrix m(v, 1, Dim);
			index = new Index(m, params);
			index->buildIndex();

			return vertex_new;
		}

		Vertex next()
		{
			return RrtImpl::next(this, *index, dq, sRandConf, sNearest, sNewConf, sAdd);
		}

		Vertex nextRand()
		{
			Vertex v = randConf(); add(*index, 0, v); return v;
		}

		static T lenSq(Vertex v) { T l=T(0); for (int i=0; i<Dim; ++i) l += v[i]*v[i]; return l; }
		static void copy(Vertex src, Vertex dest) {  for (int i=0; i<Dim; ++i) dest[i]=src[i]; }
		static void add(Vertex a, Vertex b, Vertex c) {  for (int i=0; i<Dim; ++i) c[i]=a[i]+b[i]; }

		void _rand_0_1(Vertex v) { for (int i=0; i<Dim; ++i) v[i] = T(random.Random()); }
		void rand_0_1(Vertex v) { do _rand_0_1(v); while(lenSq(v)==T(0)); }
		void rand_m1_1(Vertex v) { rand_0_1(v); for (int i=0; i<Dim; ++i) v[i] = T((random.IRandom(0,1)*2)-1) * v[i]; }
		void rand_nv(Vertex v) { rand_m1_1(v); T in = T(1)/lenSq(v); for (int i=0; i<Dim; ++i) v[i] *= in; }
		

		Vertex randConf()
		{
			for (int i=0; i<Dim; ++i) vertex_rand[i] = bound_min[i] + T(random.Random()) * (bound_max[i]-bound_min[i]);
			return vertex_rand;	
		}

		Vertex newConf(const Vertex& v, const T& dq)
		{
			// TODO: should this not be clipped to the boundaries? check the paper.
			T dv[Dim]; rand_nv(dv); 
			for (int i=0; i<Dim; ++i) 
				vertex_new[i] = v[i]+(dv[i]*dq); 
			return vertex_new;
		}
		
		Vertex nearest(const Vertex& v, Index& G)
		{
			Matrix query(v, 1, Dim);
			index->knnSearch(query, query_indices, query_dists, Dim, query_params);
			return G.getPoint(*query_indices[0]);
		}

		void add(Index& G, const Vertex& from, const Vertex& to)
		{
			Points::Add add(points); Vertex v = add.add()->v;
			copy(to, v);
			Matrix m(v, 1, Dim);
			G.addPoints(m);
		}

		static Vertex sRandConf(void* ctx)  { return ((This*) ctx)->randConf(); }
		static Vertex sNewConf(void* ctx, const Vertex& v, const T& dq) { return ((This*) ctx)->newConf(v, dq); }
		static Vertex sNearest(void* ctx, const Vertex& v, Index& G) { return ((This*) ctx)->nearest(v, G); }
		static void sAdd(void* ctx, Index& G, const Vertex& from, const Vertex& to) { return ((This*) ctx)->add(G, from, to); }


		static void build_cube(T* cube, T radius)	{ CubeNd::build_cube<Type, D>(cube, radius); }
		static void build_choices(int* cube)		{ ChoiceNd::build_choices<D>(cube); }
	};

	void flann_test1()
	{
		typedef float T;
		int Dim = 1;
		typedef flann::Matrix<T> Matrix;
		typedef flann::L2<T> DistAlgo;
		typedef flann::Index<DistAlgo> Index;
		typedef Rrt<Index, void*, T> RrtImpl;

		flann::AutotunedIndexParams params;
		T data[] = { 0.0f, 1.0f, 10.0f, 20.0f, 100.0f };
		Matrix m(data, 5, Dim);
		Index index(m, params);
		index.buildIndex();
		
		{
			flann::AutotunedIndexParams params;
			T data[] = { 101.0f };
			Matrix m(data, 1, Dim);
			index.addPoints(m);
		}
		
		
		{
			T qdata[] = { 8.0f, 101.5f };
			Matrix mq(qdata, 2, Dim);
			
			flann::Matrix<size_t> indices(new size_t[mq.rows*Dim], mq.rows, Dim);
			flann::Matrix<T> dists(new T[mq.rows*Dim], mq.rows, Dim);

			flann::SearchParams sparams;
			index.knnSearch(mq, indices, dists, Dim, sparams);
			int x=0;x;

			for (size_t i=0;i<indices.rows;++i)
			{
				size_t qi = *indices[i];
				T di = *dists[i];
				int x=0;x;
			}
		}
	}

	void flann_test2()
	{
		typedef float T;
		int Dim = 2;
		typedef flann::Matrix<T> Matrix;
		typedef flann::L2<T> DistAlgo;
		typedef flann::Index<DistAlgo> Index;
		typedef Rrt<Index, void*, T> RrtImpl;

		//flann::AutotunedIndexParams params;
		flann::KDTreeIndexParams params;
		T data[] = { 0.0f,0.0f, 5.0f,5.0f, 10.0f,10.0f, 20.0f,20.0f, 100.0f,100.0f };
		Matrix m(data, 5, Dim);
		Index index(m, params);
		index.buildIndex();
		
		{
			T data[] = { 101.0f,101.0f };
			Matrix m(data, 1, Dim);
			index.addPoints(m);
		}
		
		
		{
			T qdata[] = { 1.0f,1.0f, 101.5f,101.5f };
			Matrix mq(qdata, 2, Dim);
			
			flann::Matrix<size_t> indices(new size_t[mq.rows*Dim], mq.rows, Dim);
			flann::Matrix<T> dists(new T[mq.rows*Dim], mq.rows, Dim);

			flann::SearchParams sparams;
			index.knnSearch(mq, indices, dists, Dim, sparams);
			int x=0;x;

			for (size_t i=0;i<indices.rows;++i)
			{
				size_t qi = *indices[i];
				T di = *dists[i];
				T* v = index.getPoint(qi);
				int x=0;x;
			}
		}
	}

}
#pragma warning( pop )

namespace nics
{
	namespace fp754
	{
		enum
		{
			FLTMASK_SGN		= 0x80000000,
			FLTMASK_EXP		= 0x7F800000,
			FLTMASK_MANT	= 0x007FFFFF,
			FLTSHIFT_EXP	= 23,
			FLTCT_EXPm23	= 0x34000000,
			FLTCT_EXPm24	= 0x33800000,
			FLTCT_MANTLP	= 0x00000001,
			FLTCT_EXP127	= 0x7F000000,
			FLTCT_EXPm126	= 0x00800000,
			FLTCT_EXP0		= 0x3F800000,
			FLTCT_NAN		= 0xFFFFFFFF,
			FLTCT_pINF		= 0x7F800000,
			FLTCT_nINF		= 0xFF800000,
		};

		float h2f(unsigned int hex) { float f; *((unsigned int*) ((void*) &f)) = hex; return f; }
		unsigned int f2h(float f) { unsigned int hex = *((unsigned int*) ((void*) &f)); return hex; }

		float nextFloat(float f)
		{
			const unsigned int hex = f2h(f);
			const unsigned int sgn = (hex & FLTMASK_SGN);
			const unsigned int exp = (hex & FLTMASK_EXP);
			const unsigned int mant = (hex & FLTMASK_MANT);
			if (hex == 0) return h2f(0 | FLTCT_EXPm126 | (0) );
			if (mant != FLTMASK_MANT) return h2f(sgn | exp | (mant+1) );
			if (exp < FLTCT_EXP127) return h2f(sgn | (exp+ (1<<FLTSHIFT_EXP) ) | (0) );
			return f;
		}

		float prevFloat(float f)
		{
			const unsigned int hex = f2h(f);
			const unsigned int sgn = (hex & FLTMASK_SGN);
			const unsigned int exp = (hex & FLTMASK_EXP);
			const unsigned int mant = (hex & FLTMASK_MANT);
			if (hex == 0) return h2f( (unsigned int) ( FLTMASK_SGN | FLTCT_EXPm126 | 0 ) );
			if (mant != 0) return h2f(sgn | exp | (mant-1) );
			if (exp != 0) return h2f(sgn | (exp- (1<<FLTSHIFT_EXP)) | (FLTMASK_MANT) );
			return f;
		}

		float prevFloats(float f, int cnt)
		{
			float x = f;
			for (int i=0;i<cnt;++i)	 x = prevFloat(x);
			return x;
		}

		float nextFloats(float f, int cnt)
		{
			float x = f;
			for (int i=0;i<cnt;++i)	
				x = nextFloat(x);
			return x;
		}

		float walkFloats(float f, int cnt)
		{
			if (cnt > 0) return nextFloats(f, cnt);
			return prevFloats(f, -cnt);
		}

		float floatMachineEps()
		{
			return 0.5f * h2f(FLTCT_EXPm23);
		}

		bool isDenorm(float f)
		{
			unsigned int hex = f2h(f);
			return (hex & FLTMASK_EXP) == 0;
		}

		bool isNegBitSet(float f)
		{
			unsigned int hex = f2h(f);
			return (hex & FLTMASK_SGN) == FLTMASK_SGN;
		}

		float negBitf(float f)
		{
			return isNegBitSet(f) ? -1.0f : 1.0f;
		}

	// 	#include <float.h>
	// 	void getX87Precision()
	// 	{
	// 		unsigned int fp87 = _control87( 0, 0 );
	// 		if ( (fp87 & _MCW_PC) == _PC_64)
	// 			return 64;
	// 		if ( (fp87 & _MCW_PC) == _PC_53)
	// 			return 53;
	// 		if ( (fp87 & _MCW_PC) == _PC_24)
	// 			return 24;
	// 
	// 		return 0;
	// 	}

	}

	namespace fp754
	{
		template<int MantissaBits>
		struct float_reduce_chop
		{
			enum { Mask = (unsigned int(-1))<< (23-MantissaBits) };
			static float reduce(float f) 
			{ 
				static int guard[(23-MantissaBits)+1];

				using namespace fp754;
				return h2f(f2h(f)&Mask); 
			}
		};

		template<int MantissaBits>
		struct float_reduce_round
		{
			enum { Mask = (unsigned int(-1))<< (23-MantissaBits) };
			enum { RoundEps = (unsigned int(1))<< (23-MantissaBits) };
			
			static float reduce(float f) 
			{ 
				using namespace fp754;
				
				static int guard[(23-MantissaBits)-1];

				if (f < 0.0f) return -reduce(-f);
				float rnd_mul = h2f(FLTCT_EXP0 | (RoundEps>>1));
				float r = h2f(f2h(f * rnd_mul) & Mask); 
				return r;
			}
		};

		template<typename Reduce>
		struct float_reduce
		{
			float v;

			static float reduce(float f) { return Reduce::reduce(f); }

			float_reduce() {}
			float_reduce(const float f) { v = reduce(f); }

			//operator float() { return v; }
			operator double() { return double(v); }

			float_reduce operator-() { return float_reduce(-v); }
 			float_reduce operator+(const float_reduce& v1) { float o = reduce(v+v1.v); return float_reduce(o); }
 			float_reduce operator-(const float_reduce& v1) { float o = reduce(v-v1.v); return float_reduce(o); }
 			float_reduce operator*(const float_reduce& v1) { float o = reduce(v*v1.v); return float_reduce(o); }
			float_reduce operator/(const float_reduce& v1) { float o = reduce(v/v1.v); return float_reduce(o); }

			float_reduce& operator*=(const float_reduce& v1) { v = reduce(v*v1.v); return *this; }
			float_reduce& operator+=(const float_reduce& v1) { v = reduce(v+v1.v); return *this; }
			float_reduce& operator/=(const float_reduce& v1) { v = reduce(v/v1.v); return *this; }
			
			bool operator==(const float_reduce& v1) const { return v >= v1.v; }
			bool operator>=(const float_reduce& v1) const { return v >= v1.v; }
			bool operator>(const float_reduce& v1) const { return v > v1.v; }
			bool operator<(const float_reduce& v1) const { return v < v1.v; }
		};
		template<typename Reduce>
		float_reduce<Reduce> abs(float_reduce<Reduce> v) { return v.0 >= 0.0f ? v : -v; }
	}

	template<typename T> T m_abs(T v) { return v >= T(0) ? v : -v; }
	template<typename T> T m_max(T v1, T v2) { return v1 >= v2 ? v1 : v2; }
	template<typename T> T m_min(T v1, T v2) { return v1 >= v2 ? v1 : v2; }

	struct bint
	{
		typedef unsigned int ui;

		ui m_sign;
		ui m_1;

		unsigned int size() const { return 1; }
		void setSize(unsigned int s) {}
		
		ui& sign() { return m_sign; }
		const ui& sign() const { return m_sign; }
		ui& part(unsigned int i) { return m_1; }
		ui& epart(unsigned int i) { i >= size() ? 0 : m_1; }
		const ui& part(unsigned int i) const { return m_1; }
		const ui& epart(unsigned int i) const { i >= size() ? 0 : m_1; }

		void add(const bint& a, const bint& b)
		{
			if (a.sign() == b.sign())
			{
				bool al = a.size() <= b.size();
				const bint& sl = al ? a : b;
				const bint& sg = al ? b : a;

				unsigned int ms = m_max(a.size(), b.size());
				setSize(ms);

				ui carry = 0;
				sign() = a.sign();
				for (unsigned int i=0; i<ms; ++i)
				{
					ui ai = sl.epart(i); ui bi = sg.part(i);
					ui s = ai+bi+carry;
					carry = s >= ai ? 1 : 0;
					part(i) = s;
				}
			}
			else
			{

			}
		}

		void sub_pos_gl(const bint& g, const bint& l)
		{
			unsigned int ms = g.size();
			setSize(ms);
			
			ui carry = 0;
			sign() = 1;
			for (unsigned int i=0; i<ms; ++i)
			{
				ui ai = l.epart(i); ui bi = g.part(i);
				ui s = (bi-(ai+carry));
				carry = s >= bi ? 1 : 0;
				part(i) = s;
			}
		}
	};
}

#endif // NICS_H