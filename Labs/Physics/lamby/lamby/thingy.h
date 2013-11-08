#ifndef LAMBY_THINGY_H
#define LAMBY_THINGY_H

template<typename T>
struct ThingiesArr
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
	int size;
	dtorFunc dtor;
	
	ThingiesArr(int chain_size_ = 32,dtorFunc dtor_ = 0) : ctxt(0), cel(0), chain_count(0), chain_size(chain_size_), size(0), dtor(dtor_) {}
	~ThingiesArr() { _dtor(ctxt, cel, chain_size, dtor); }

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

	T* at(int index)
	{
		ChainEl* bl = cel;
		int ci = 0;
		while( bl )
		{
			for (int bi=0; bi<chain_size; ++bi)
			{
				if (bl->valid[bi])
				{
					if (ci == index)
						return &(bl->el[bi]);
					ci++;
				}
			}

			bl = bl->next;
		}
		return 0;
	}

	struct Add
	{
		ThingiesArr& arr;
		ChainEl* cel;
		int i;

		Add(ThingiesArr& arr_)
		: arr(arr_), cel(arr_.cel)
		{
			if (cel) while (cel->first_invalid >= arr.chain_size && cel->next) cel = cel->next;
			i = (cel ? cel->first_invalid : -1);
		}

		T* add()
		{
			arr.size++;

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
		ThingiesArr& arr;
		ChainEl* cel;
		int i;

		Iter(ThingiesArr& arr_)
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

class Thingies;

//
//
//
struct Convex
{
	M3 transf;
	V2* v;
	int count;
	Rl r;
};
Convex* ctor(Thingies* th, Convex* el, M3p transf, const V2* vs, int count, Rlp r)  { el->transf=transf; el->v=(V2*)malloc(count*sizeof(V2)); memcpy(el->v,vs,count*sizeof(V2)); el->count=count; el->r=r; return el; }
void dtor_convex(void* th, Convex* el)  { free(el->v); }


//
//
//
class Thingies
{
public:
	
	typedef ThingiesArr<Convex> Convexes;
	typedef Convexes::Iter ConvexIter;
	typedef Convexes::Add ConvexAdd;
	Convexes convexes;

	Thingies() : convexes(32, dtor_convex) {}

	Convex* addConvex(Convexes::Add& add, M3p transf, const V2* vs, int count, Rlp r)
	{
		Convex* el = add.add(); ctor(this, el, transf, vs, count, r); return el;
	}
	Convex* addConvex(M3p transf, const V2* vs, int count, Rlp r)
	{
		Convexes::Add add(convexes); return addConvex(add, transf, vs, count, r);
	}
	Convex* iterConvex(Convexes::Iter& it)
	{
		return it.next();
	}
};


// if (1)
// 		{
// 			Painty painty;
// 			V2 v[] = { V2(-0.2f,-0.2f), V2(-0.2f,0.2f), V2(0.2f,0.2f), V2(0.2f,-0.2f) };
// 			M3 m = rigid(V2(0.1f, 0.3f), m_rad(20.0f));
// 			draw_convex(painty, m, v, 4, u_ijk());
// 		}

#endif