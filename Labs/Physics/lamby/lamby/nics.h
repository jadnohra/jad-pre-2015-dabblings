#ifndef LAMBY_NICS_H
#define LAMBY_NICS_H

#define NOMINMAX
#include <windows.h>	// Timer

#pragma warning( push )
#pragma warning( disable : 4996 )
#pragma warning( disable : 4003 )
#pragma warning( disable : 4291 )	// Check if this is a real problem.
#include "../flann/src/cpp/flann/flann.hpp"
#pragma comment( lib, "../../flann/build/lib/Release/flann_cpp_s")
#pragma warning( pop )

#include "gaussy.h" 
#include "touchy.h" // gjk

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

		Timer timer;
		
		int test_count = 0;
		GjkScratch gjk;
		
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
		typedef Vertex(*nearestFunc)(void* ctx, const Vertex& v, const Graph& G);
		typedef void(*addFunc)(void* ctx, const Graph& G, const Vertex& from, const Vertex& to);

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

		Index* index;
		T bound_min[Dim];
		T bound_max[Dim];
		T vertex_rand[Dim];

		Rrt_Rn() : index(0) {}


		~Rrt_Rn() { delete index; }

		void setBound(int di, Rl min, Rl max)
		{
			bound_min[di] = min; bound_max[di] = max;
		}
		void rand(int di)
		{
			//http://www.agner.org/random/
		}

		static Vertex randConf(void* ctx)
		{
			This* this = (This*) ctx;
			return this->vertex_rand;
		}

		//typedef Vertex(*newConfFunc)(const Vertex& v, const DistT& dq);
		//typedef Vertex(*nearestFunc)(const Vertex& v, const Graph& G);

// 		static Vertex next(Graph& G, DistT dq, randConfFunc randConf, nearestFunc nearest, newConfFunc newConf)
// 		{
// 		}
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

		flann::AutotunedIndexParams params;
		T data[] = { 0.0f,0.0f, 5.0f,5.0f, 10.0f,10.0f, 20.0f,20.0f, 100.0f,100.0f };
		Matrix m(data, 5, Dim);
		Index index(m, params);
		index.buildIndex();
		
		{
			flann::AutotunedIndexParams params;
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
				int x=0;x;
			}
		}
	}
}

#endif // LAMBY_NICS_H