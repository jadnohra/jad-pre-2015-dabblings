#ifndef _Ptr_hpp
#define _Ptr_hpp

#include "SuperHeader.hpp"


template <typename _T>
inline _T& dref(_T* pPtr) { if (!pPtr) assert(false); return *pPtr; }
	
/*
	(jn)

	* All pointer classes have no space or performance overhead compared
	to using raw pointers except FlexiblePtr.
	
	* All Pointer classes have the same 'interface' so when changing from PtrWeak to PtrNew
	per example no code changes need to be made
	
	
	example usage:
	
	class Foo {
	
		int bar;
	};
	
	void func(Foo* pFoo) {
	
		PtrWeak<Foo> foo(pFoo);
		
		//isValid checks if pointer is not NULL
		if (foo.isValid()) {
		
			//operator-> allows using the pointer as usual
			++foo->bar;
			
			test(foo); //foo is automatically dereferenced and assert is thrown if it is NULL
			test(pFoo); //foo is automatically converted to the raw pointer
		}
		
		foo.destroy(); //for a PtrWeak this only sets the pointer to NULL
		
		if (foo.isNull()) {
		
			
		}
	}
	
	void func(Foo* pFoo) {
	
		PtrWeak<Foo> foo = pFoo;
		
		//assert(foo.isValid()); //this is not needed, 
		//the operator-> asserts when the pointer is NULL automatically
		
		++foo->bar; 
	}
	
	void test(Foo& foo) {
	}
	
	void test2(Foo* foo) {
	}
	
	void func() {
	
		PtrNew<Foo> foo = new Foo();
		
		//isValid checks if pointer is NULL
		if (foo.isNull()) {
		
			assert(false);
		}
		
		//the allocation is automatically deleted when foo goes out of scope
		//but food.destroy() can be used as well
		
		foo = new Foo(); //old allocation is automatically destroyed
	}
	
	class Test {
	public:
	
		Test() {
		
			//no need to initialize mFoos to NULL
		}
		
		~Test() {
		
			//no need to delete mFoos
		}
		
		void func(int count) {
		
			//if mFoos was already allocated it is automatically decstroyed
			//before the new assignment
			mFoos = new Foo(count);
		}
	
	protected:
	
		PtrNewArray<Foo> mFoos;
	};
	
	* PtrAlloc is similar to PtrNew except that it uses std::free
	
	* FlexiblePtr can change its type at runtime, but it needs to store its type, 
		therefore requiring more storage than the other types

*/

enum PtrType {

	Ptr_weak = 0, Ptr_new, Ptr_newArray, Ptr_alloc
};

template <class T>
struct Ptr {

	T* pPtr;

	Ptr(T* _pObject = NULL) {

		pPtr = _pObject;
	}


	~Ptr() {

		//assrt(pPtr == NULL);
	}

	void destroy(PtrType type) {

		switch (type) {
			case Ptr_new:
				delete pPtr;
				break;
			case Ptr_newArray:
				delete [] pPtr;
				break;
			case Ptr_alloc:
				{
					//using namespace std;
					free(pPtr);
					break;	
				}
			default:
				break;
		}
		
		pPtr = NULL;
	}

	void conditionalDestroy(bool doDestroy, PtrType type) {

		if (doDestroy) {

			destroy(type);
		}
	}

	void setNoDestroy(T* _pObject) {

		pPtr = _pObject;
	}

	inline T& dref() {

		assert(pPtr != NULL);

		return *pPtr;
	}

	inline T* ptr() const {

		return pPtr;
	}

	inline operator T& () {

		return dref();
	}

	inline operator T* () {

		return ptr();
	}

	inline T*& ptrRef() {

		return pPtr;
	}


	inline T* operator->() {

		assert(pPtr != NULL);

		return pPtr;
	}

	inline const T* operator->() const {

		assert(pPtr != NULL);

		return pPtr;
	}

	inline bool isNull() const {

		return (pPtr == NULL);
	}

	inline bool isValid() const {

		return (pPtr != NULL);
	}
};

template<class T, PtrType typeT>
struct TypedPtr : Ptr<T> {

	TypedPtr(T* _pObject = NULL) : Ptr<T>(_pObject){
	}

	~TypedPtr() {

		destroy();
	}

	inline void destroy() {

		Ptr<T>::destroy(typeT);
	}

	inline void conditionalDestroy(bool doDestroy) {

		Ptr<T>::conditionalDestroy(doDestroy, typeT);
	}

	void set(T* _pObject) {

		Ptr<T>::destroy(typeT);

		pPtr = _pObject;
	}

	inline T* operator=(T* pObj) {
	
		set(pObj);
		return pObj;
	};
};

template<class T>
struct PtrWeak : TypedPtr<T, Ptr_weak> {

	PtrWeak(T* _pObject = NULL) : TypedPtr(_pObject) {
	}

	inline T* operator=(T* pObj) {
	
		set(pObj);
		return pObj;
	};
};

template<class T>
struct PtrAlloc : TypedPtr<T, Ptr_alloc> {

	PtrAlloc(T* _pObject = NULL) : TypedPtr(_pObject){
	}

	inline T* operator=(T* pObj) {
	
		set(pObj);
		return pObj;
	};

	PtrAlloc(const PtrAlloc<T>& ref) : TypedPtr(ref.pPtr) {
	}

	inline PtrAlloc<T>& operator=(const PtrAlloc<T>& ref) {
	
		set(ref.pPtr);
		return *this;
	};
};

template<class T>
struct PtrNew : TypedPtr<T, Ptr_new> {

	PtrNew(T* _pObject = NULL) : TypedPtr(_pObject){
	}

	inline T* operator=(T* pObj) {
	
		set(pObj);
		return pObj;
	};

	PtrNew(const PtrNew<T>& ref) : TypedPtr(ref.pPtr) {
	}

	inline PtrNew<T>& operator=(const PtrNew<T>& ref) {
	
		set(ref.pPtr);
		return *this;
	};
};


template<class T>
struct PtrNewArray : TypedPtr<T, Ptr_newArray> {

	PtrNewArray(T* _pObject = NULL) : TypedPtr(_pObject){
	}

	inline T* operator=(T* pObj) {
	
		set(pObj);
		return pObj;
	};

	PtrNewArray(const PtrNewArray<T>& ref) : TypedPtr(ref.pPtr) {
	}

	inline PtrNewArray<T>& operator=(const PtrNewArray<T>& ref) {
	
		set(ref.ptr());
		return *this;
	};
};

template<class T>
struct FlexiblePtr : Ptr<T> {

	PtrType type;

	FlexiblePtr(T* _pObject = NULL, PtrType _type = Ptr_weak) : Ptr<T>(_pObject){

		type = _type;
	}

	~FlexiblePtr() {

		destroy();
	}

	inline void destroy() {

		Ptr<T>::destroy(type);
	}

	inline void conditionalDestroy(bool doDestroy) {

		Ptr<T>::conditionalDestroy(doDestroy, type);
	}

	void setType(PtrType _type, bool destroyObject = true) {

		if (destroyObject && pPtr) {

			destroy();
		}

		type = _type;
	}


	void set(T* _pObject) {

		Ptr<T>::destroy(type);

		pPtr = _pObject;
	}

	void set(T* _pObject, PtrType _type) {

		destroy();
		type = _type;
		pPtr = _pObject;
	}

	inline T* operator=(T* pObj) {
	
		set(pObj);
		return pObj;
	}

	FlexiblePtr(const FlexiblePtr<T>& ref) : FlexiblePtr(ref.pPtr) {
	}

	inline FlexiblePtr<T>& operator=(const FlexiblePtr<T>& ref) {
	
		set(ref.ptr());
		return *this;
	};
	
};


#endif