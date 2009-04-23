#ifndef _Array_hpp
#define _Array_hpp

#include "SuperHeader.hpp"
#include "Ptr.hpp"
#include "memory.h"


template<typename _ValueT, typename _IndexT = size_t>
class StaticArray 
{
public:

	typedef _ValueT Value;
	typedef _IndexT Index;
	typedef Value* iterator;

public:

	StaticArray() : mCount(0) 
	{
	}
	
	~StaticArray() 
	{
		destroy();
	}
	
	void destroy() 
	{
		mArray.destroy();
		mCount = 0;		
	}
	
	void create(const _IndexT& newCount) 
	{
		if (newCount != mCount) 
		{
			mArray.destroy();					
			
			if (newCount) 
			{
				mArray.set(new Value[newCount]);
			}
			
			mCount = newCount;
		}
	}
	
	inline Value& operator[](const Index& i) const { return mArray.pPtr[i]; }
	inline Value& el(const Index& i) const { return mArray.pPtr[i]; }
	inline Value* data() const { return mArray.pPtr; }
	inline const Index& getCount() const { return mCount; }
	
	inline Value* begin() const { return data(); }
	inline Value* end() const { return data() + mCount; }
	
protected:	

	Index mCount;
	PtrNewArray<Value> mArray;
};

#endif