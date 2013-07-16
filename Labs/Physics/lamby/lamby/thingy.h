#ifndef LAMBY_THINGY_H
#define LAMBY_THINGY_H

class Thingies;

template<typename T>
struct ThingiesArr
{
	struct ChainEl
	{
		T* el;
		unsigned char* valid;
		ChainEl* next;
	};

	static void _dtor(Thingies* th, ChainEl* cel, int size)
	{
		if (cel)
		{
			if (cel->next) _dtor(th, cel->next, size);

			for (int i=0;i<size;++i)
			{
				if (cel->valid[i]) dtor(th, cel->el+i);
			}
			free(cel->el);
			free(cel->valid);
			free(cel);
		}
	}

	Thingies* th;
	ChainEl* cel;
	int chain_count;
	int chain_size;
	
	ThingiesArr() : cel(0), chain_count(0), chain_size(32) {}
	~ThingiesArr() { _dtor(th, cel, chain_size); }

	ChainEl* alloc()
	{
		ChainEl* cel = (ChainEl*) malloc(sizeof(ChainEl));
		cel->el = (T*) malloc(chain_size*sizeof(T));
		cel->valid = (unsigned char*) malloc(chain_size*sizeof(unsigned char));
		for (int i=0;i<chain_size;++i) cel->valid[i]=0;
		cel->next = 0;
		return cel;
	}

	struct Add
	{
		ThingiesArr& arr;
		ChainEl* cel;
		int i;

		Add(ThingiesArr& arr_)
		: arr(arr_), cel(arr_.cel), i(-1) {}

		T* add()
		{
			if (cel == 0)
			{
				cel = arr.cel = arr.alloc(); i=0; cel->valid[i]=1; return &cel->el[i++];
			}
			
			while(1)
			{
				while(i<arr.chain_size && !cel->valid[i]) i++;
				if (i+1>=arr.chain_size)
				{
					cel = cel->next = arr.alloc(); i=0; cel->valid[i]=1; return &cel->el[i++];
				}
				else
				{
					cel->valid[i]=1; return &cel->el[i++];
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


struct Convex
{
	M3 transf;
	V2* v;
	int count;
	Rl r;
};
Convex* ctor(Thingies* th, Convex* el, M3p transf, const V2* vs, int count, Rlp r)  { el->transf=transf; el->v=(V2*)malloc(count*sizeof(V2)); memcpy(el->v,vs,count*sizeof(V2)); el->count=count; el->r=r; return el; }
void dtor(Thingies* th, Convex* el)  { free(el->v); }

class Thingies
{
public:
	
	typedef ThingiesArr<Convex> Convexes;
	typedef Convexes::Iter ConvexIter;
	typedef Convexes::Add ConvexAdd;
	Convexes convexes;

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