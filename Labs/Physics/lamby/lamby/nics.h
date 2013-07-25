#ifndef LAMBY_NICS_H
#define LAMBY_NICS_H

#define NOMINMAX
#include <windows.h>	// Timer

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

#pragma warning( disable : 4996 )
#pragma warning( disable : 4018 )
#include "../lodepng/lodepng.h"


#include "gaussy.h" 
#include "touchy.h" // gjk
#include "thingy.h" // array

namespace nics
{
	typedef double Time;

	struct Timer
	{
		LARGE_INTEGER start_t;
		
		Timer() { start_t.QuadPart = 0; }

		void start() { QueryPerformanceCounter(&start_t); }

		Time stop() 
		{ 
			LARGE_INTEGER end_t;
			QueryPerformanceCounter(&end_t);

			LARGE_INTEGER frequency;
			QueryPerformanceFrequency(&frequency);

			return Time(end_t.QuadPart-start_t.QuadPart) / Time(frequency.QuadPart);
		}
	};

	Time getGjkRunTime()
	{
		using namespace gjk;

		int test_count = 0;
		GjkScratch gjk;
		
		Timer timer;
		timer.start();
		{
			int err = 0;
			int step = 4;
			if (1)
			{
				V2 v1[] = { V2(0.0f, 1.0f), V2(0.0f, 2.0f), V2(-1.0f, 1.0f) };
				V2 v2[] = { V2(0.0f, 1.5f) };

				for (int x=0; x<512; x+=step) 
					for (int y=0; y<512; y+=step)
				{
					Rl l[] = { Rl(x)/Rl(512), Rl(y)/Rl(512), Rl(0)};
					l[2] = Rl(1) - l[0] - l[1];
					if (l[2] >= Rl(0))
					{
						v2[0] = lin_comb(v1, l, 3);

						test_count++;
						Out_gjk_distance dist = gjk_distance(gjk, m3_id(), v1, 3, Rl(0), m3_id(), v2, 1, Rl(0), 1.e-7f);
						if (!dist.success || dist.dist > dist.eps) 
						{ 
							err++; 
						}
					}

				}
			}

			if (1)
			{
				V2 v1[] = { V2(0.0f, 1.0f), V2(0.0f, 2.0f), V2(-1.0f, 1.0f) };
				V2 v2[] = { V2(0.0f, 1.5f) };

				for (int x=0; x<512; x+=step) 
					for (int y=0; y<512; y+=step)
				{
					Rl l[] = { Rl(x)/Rl(512), Rl(1.1) + Rl(y)/Rl(512), Rl(0)};
					l[2] = Rl(1) - l[0] - l[1];
					v2[0] = lin_comb(v1, l, 3);

					test_count++;
					Out_gjk_distance dist = gjk_distance(gjk, m3_id(), v1, 3, Rl(0), m3_id(), v2, 1, Rl(0), 1.e-7f);
					if (!dist.success || dist.dist <= dist.eps) 
					{ 
						err++; 
					}
				}
			}
		}
		Time dt = timer.stop()/Time(test_count);
		return dt;
	}


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

	template<typename T, int Dim>
	struct Rrt_Rn
	{
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
		typedef ThingiesArr<Point> Points;
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

		void _rand_0_1(Vertex v) { for (int i=0; i<Dim; ++i) v[i] = T(random.Random()); }
		T lenSq(Vertex v) { T l=T(0); for (int i=0; i<Dim; ++i) l += v[i]*v[i]; return l; }
		void rand_0_1(Vertex v) { do _rand_0_1(v); while(lenSq(v)==T(0)); }
		void rand_m1_1(Vertex v) { rand_0_1(v); for (int i=0; i<Dim; ++i) v[i] = T((random.IRandom(0,1)*2)-1) * v[i]; }
		void rand_nv(Vertex v) { rand_m1_1(v); T in = Rl(1)/lenSq(v); for (int i=0; i<Dim; ++i) v[i] *= in; }
		void copy(Vertex src, Vertex dest) {  for (int i=0; i<Dim; ++i) dest[i]=src[i]; }

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

	#ifdef LODEPNG_H
	Time rtt_test1()
	{
		int pt_count = 1024*64;
		typedef Rrt_Rn<float, 2> Rrt;
		Rrt rrt;
		float min=-1.0f; float max=1.0f;
		rrt.init(min, max, 0.1f);

		Timer timer;
		timer.start();
		if (1)
		{
			for (int i=0;i<pt_count; ++i) rrt.next();
		} 
		else
		{
			for (int i=0;i<pt_count; ++i) 
			{
				Rrt::Vertex v = rrt.randConf();
				rrt.add(*rrt.index, 0, v);
			}
		}
		Time dt = timer.stop() / Time(pt_count);


		int w=641; int h=641;
		float scalex = float(w);
		float scaley = float(w);
		unsigned char* image = (unsigned char*) malloc(w*h);;
		for (int i=0;i<w*h; ++i) image[i] = 255;

		for (int i=0;i<rrt.index->size(); ++i)
		{
			Rrt::Vertex v = rrt.index->getPoint(i);
			float x = ((v[0]-min)/(max-min))*scalex;
			float y = ((v[1]-min)/(max-min))*scaley;
			x = m_clamp(x, 0.0f, float(w-1)); y = m_clamp(y, 0.0f, float(h-1));

			image[int(x+0.5f)+int(y+0.5f)*(w)] = 0;
		}

 		lodepng_encode_file("rtt_test1.png", image, w, h, LCT_GREY, 8);		return dt;	}
	#endif

	Time rtt_test2()
	{
		int pt_count = 1024*64;
		typedef Rrt_Rn<float, 8> Rrt;
		Rrt rrt;
		float min=-1.0f; float max=1.0f;
		rrt.init(min, max, 0.1f);

		using namespace gjk;
		GjkScratch gjk(false); // true crashes! fix.
		int err = 0;

		typedef ThingiesArr<int> Ints;
		Ints failures;

		Timer timer;
		timer.start();
		Rl eps = 1.e-7f;
		#if 0
		{
			for (int i=0;i<pt_count; ++i) 
			{
				Rrt::Vertex v = rrt.next();

				Out_gjk_distance dist = gjk_distance(gjk, m3_id(), (V2*) v, 3, Rl(0), m3_id(), ((V2*) v)+3, 1, Rl(0), eps);
				if (!dist.success) 
				{ 
					err++; 
					Ints::Add add(failures); *add.add() = i;
				}
			}
			err;
		} 
		#else
		{
			for (int i=0;i<pt_count; ++i) 
			{
				Rrt::Vertex v = rrt.randConf();
				rrt.add(*rrt.index, 0, v);

				Out_gjk_distance dist = gjk_distance(gjk, m3_id(), (V2*) v, 3, Rl(0), m3_id(), ((V2*) v)+3, 1, Rl(0), eps);
				if (!dist.success) 
				{ 
					err++; 
					Ints::Add add(failures); *add.add() = i;
				}
				err;
			}
		}
		#endif
		Time dt = timer.stop() / Time(pt_count);
		printf("err: %d\n", err);

		int w=641; int h=641;
		float scalex = float(w);
		float scaley = float(w);
		unsigned char* image = (unsigned char*) malloc(w*h*3);
		for (int i=0;i<w*h*3; ++i) image[i] = 255;

		Ints::Iter fit(failures); int* fi = fit.next();
		for (int i=0;i<rrt.index->size(); ++i)
		{
			Rrt::Vertex v = rrt.index->getPoint(i);
			float x = ((v[0]-min)/(max-min))*scalex;
			float y = ((v[1]-min)/(max-min))*scaley;
			x = m_clamp(x, 0.0f, float(w-1)); y = m_clamp(y, 0.0f, float(h-1));

			unsigned char r,g,b; r=0;b=0;g=0;

			if (fi && (*fi == i))
			{
				r = 255;
				fi = fit.next();
			}

			int ii = int(x+0.5f)+int(y+0.5f)*(w);
			image[ii*3+0] = r;
			image[ii*3+1] = g;
			image[ii*3+2] = b;
		}

 		lodepng_encode_file("rtt_test1.png", (unsigned char*) image, w, h, LCT_RGB, 8);		return dt;	}
}
#pragma warning( pop )

#endif // LAMBY_NICS_H