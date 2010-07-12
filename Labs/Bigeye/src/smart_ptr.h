/******************************************************************************
 * This file is part of The AI Sandbox.
 * 
 * Copyright (c) 2008, AiGameDev.com
 * 
 * Credits:         See the PEOPLE file in the base directory.
 * License:         This software may be used for your own personal research
 *                  and education only.  For details, see the LICENSING file.
 *****************************************************************************/

#ifndef CORE_SMART_PTR_H
#define CORE_SMART_PTR_H

/// @file           core/smart_ptr.h
/// @brief          Extended pointer that can manage its memory itself.

#include <stddef.h>
#include <algorithm>
#include "meta_util.h"

template<class P>
struct PtrDeletePolicy_delete {

    static inline P* getDefault() { return (P*) NULL; }
    static inline void doDelete(P* ptr) { delete ptr; }
};

template<class P>
struct PtrDeletePolicy_deleteArray {

    static inline P* getDefault() { return (P*) NULL; }
    static inline void doDelete(P* ptr) { delete[] ptr; }
};

template<class P>
struct PtrDeletePolicy_nothing {

    static inline P* getDefault() { return (P*) NULL; }
    static inline void doDelete(P* ptr) {}
};

template<class P>
struct PtrDeletePolicy_free {

    static inline P* getDefault() { return (P*) NULL; }
    static inline void doDelete(P* ptr) { free(ptr); }
};

template<class T, class DeletePolicy = PtrDeletePolicy_delete<T> > class scoped_ptr : noncopyable {
public:
    typedef T element_type;

    explicit scoped_ptr(T * p = DeletePolicy::getDefault()) : m_Pointer(p) {}
    ~scoped_ptr() { DeletePolicy::doDelete(m_Pointer); }

    void reset(T * p = DeletePolicy::getDefault())
	{
		DeletePolicy::doDelete(m_Pointer);
		m_Pointer = p;
	}

    inline T& operator*() const { return *m_Pointer; }
    inline T* operator->() const { return m_Pointer; }
    inline T* get() const { return m_Pointer; }
        
    inline operator unspecified_bool_type() const {  return m_Pointer == DeletePolicy::getDefault() ? unspecified_bool_false : unspecified_bool_true; }

    void swap(scoped_ptr& b) {

        T* temp = m_Pointer;
        m_Pointer = b.m_Pointer;
        b.m_Pointer = temp;
    }

protected:

    T* m_Pointer;
};

template<class T, class DeletePolicy = PtrDeletePolicy_delete<T> > class auto_scoped_ptr : scoped_ptr<T, DeletePolicy> {
private:

	typedef scoped_ptr<T, DeletePolicy> BaseT;

public:

	explicit auto_scoped_ptr(T * p = DeletePolicy::getDefault()) : BaseT(p) {}

	//T* operator=(T* p) { reset(p); }

	using BaseT::operator*;
	using BaseT::operator->;

	inline operator const T*() { return m_Pointer; }
	inline operator T*() { return m_Pointer; }
};

template<class T, class DeletePolicy> 
void swap(scoped_ptr<T, DeletePolicy> & a, scoped_ptr<T, DeletePolicy> & b) {

    a.swap(b);
}

class smart_ptr_RefCounter {
public:

    typedef short RefCountType;

    RefCountType m_StrongRefCount;
    RefCountType m_WeakRefCount;

    smart_ptr_RefCounter() : m_StrongRefCount(1), m_WeakRefCount(0) {}

    inline bool isUnique() const { return m_StrongRefCount == 1; }
    inline RefCountType getStrongRefCount() const { return m_StrongRefCount; }

    inline void addStrongRef() {

        ++m_StrongRefCount;
    }

    inline void releaseStrongRef() {

        if (--m_StrongRefCount == 0 &&  m_WeakRefCount == 0) {
            delete this;
        }
    }

    inline void addWeakRef() {

        ++m_WeakRefCount;
    }

    inline void releaseWeakRef() {

        if (--m_WeakRefCount == 0 && m_StrongRefCount == 0) {
            delete this;
        }
    }
};

template<class T, class DeletePolicy = PtrDeletePolicy_delete<T> > 
class weak_ptr;

template<class T, class DeletePolicy = PtrDeletePolicy_delete<T> > 
class shared_ptr {
friend class weak_ptr<T, DeletePolicy>;

public:
    
    typedef smart_ptr_RefCounter::RefCountType RefCountType;

public:
    typedef T element_type;

    shared_ptr() : m_Pointer(DeletePolicy::getDefault()), m_pRefCounter(new smart_ptr_RefCounter) {}
    template<class Y> explicit shared_ptr(Y * p) : m_Pointer(p), m_pRefCounter(new smart_ptr_RefCounter) {}
    
    ~shared_ptr() {

        reset<T>();    
    }

    shared_ptr(shared_ptr const & r) : m_Pointer(r.m_Pointer), m_pRefCounter(r.m_pRefCounter) {

        if (m_pRefCounter)
            m_pRefCounter->addStrongRef();
    }

    template<class Y, class DP>
    shared_ptr(shared_ptr<Y, DP> const & r, T* p) : m_Pointer(p), m_pRefCounter(r.m_pRefCounter) {

        if (m_pRefCounter)
            m_pRefCounter->addStrongRef();
    }

    template<class Y> 
    shared_ptr(shared_ptr<Y> const & r) : m_Pointer(r.m_Pointer), m_pRefCounter(r.m_pRefCounter) {

        if (m_pRefCounter)
            m_pRefCounter->addStrongRef();
    }

    template<class Y> 
    explicit shared_ptr(weak_ptr<Y> const & r);

    inline shared_ptr & operator=(shared_ptr const & r) { return shared_ptr(*this); }

    template<class Y> 
    inline shared_ptr & operator=(shared_ptr<Y> const & r) { return shared_ptr(*this); }

    template<class Y> 
    void reset(Y * p = DeletePolicy::getDefault()) {

        if (m_pRefCounter) {
            
            if (m_pRefCounter->isUnique()) {

                DeletePolicy::doDelete(m_Pointer);
            }

            m_pRefCounter->releaseStrongRef();
        }

        m_Pointer = p;
        m_pRefCounter = m_Pointer == DeletePolicy::getDefault() ? NULL : new smart_ptr_RefCounter;
    }

    inline T & operator*() const { return *m_Pointer; }
    T * operator->() const { return m_Pointer; }
    T * get() const { return m_Pointer; }

    inline bool unique() const { return use_count() == 1; } 
    inline RefCountType use_count() const { return m_pRefCounter ? m_pRefCounter->getStrongRefCount() : 0; }

    inline operator unspecified_bool_type() const {  return m_Pointer == DeletePolicy::getDefault() ? unspecified_bool_false : unspecified_bool_true; }

    void swap(shared_ptr & b) {

        std::swap(m_Pointer, b.m_Pointer);
        std::swap(m_pRefCounter, b.m_pRefCounter);
    }

private:

    T* m_Pointer;
    smart_ptr_RefCounter* m_pRefCounter;
};

template<class T, typename DPT, class U, typename DPU>
inline bool operator==(shared_ptr<T, DPT> const & a, shared_ptr<U, DPU> const & b) {

    return a.get() == b.get();
}

template<class T, typename DPT, class U, typename DPU>
inline bool operator!=(shared_ptr<T, DPT> const & a, shared_ptr<U, DPU> const & b) {

    return a.get() != b.get();
}

/*
TODO
template<class T, class DPT, class U, class DPU>
bool operator<(shared_ptr<T, DPT> const & a, shared_ptr<U, DPU> const & b) {
}
*/

template<class T, typename DP> inline void swap(shared_ptr<T, DP> & a, shared_ptr<T, DP> & b) { return a.swap(b); }

template<class T, typename DP> inline T * get_pointer(shared_ptr<T, DP> const & p) { return p.get(); }

template<class T, class U, typename DP>
shared_ptr<T, DP> inline static_pointer_cast(shared_ptr<U, DP> const & r) { return shared_ptr<T, DP>(r, static_cast<T*>(r.get())); }

template<class T, class U, typename DP>
shared_ptr<T, DP> inline const_pointer_cast(shared_ptr<U, DP> const & r) { return shared_ptr<T, DP>(r, const_cast<T*>(r.get())); }

template<class T, class U, typename DP>
shared_ptr<T, DP> inline dynamic_pointer_cast(shared_ptr<U, DP> const & r) { return shared_ptr<T, DP>(r, dynamic_cast<T*>(r.get())); }

template<class T, class DeletePolicy> 
class weak_ptr {
public:

    typedef smart_ptr_RefCounter::RefCountType RefCountType;

public:
    typedef T element_type;

    weak_ptr() : m_Pointer(DeletePolicy::getDefault()), m_pRefCounter(NULL) {}

    template<class Y> 
    weak_ptr(shared_ptr<Y, DeletePolicy> const & r) : m_Pointer(r.m_Pointer), m_pRefCounter(r.m_pRefCounter) {

        if (m_pRefCounter)
            m_pRefCounter->addWeakRef();
    }

    weak_ptr(weak_ptr const & r) : m_Pointer(r.m_Pointer), m_pRefCounter(r.m_pRefCouner) {

        if (m_pRefCounter)
            m_pRefCounter->addWeakRef();
    }

    template<class Y> 
    weak_ptr(weak_ptr<Y> const & r) : m_Pointer(r.m_Pointer), m_pRefCounter(r.m_pRefCouner) {

        if (m_pRefCounter)
            m_pRefCounter->addWeakRef();
    }

    ~weak_ptr() {

        reset();
    }

    weak_ptr & operator=(weak_ptr const & r) { 
        
        if (m_pRefCounter)
            m_pRefCounter->releaseWeakRef();

        m_Pointer = r.m_Pointer;
        m_pRefCounter = r.m_pRefCounter;

        if (m_pRefCounter)
            m_pRefCounter->addWeakRef();

        return *this;
    }

    template<class Y> 
    weak_ptr & operator=(weak_ptr<Y> const & r)  { 

        if (m_pRefCounter)
            m_pRefCounter->releaseWeakRef();

        m_Pointer = r.m_Pointer;
        m_pRefCounter = r.m_pRefCounter;

        if (m_pRefCounter)
            m_pRefCounter->addWeakRef();

        return *this;
    }

    template<class Y> 
    weak_ptr & operator=(shared_ptr<Y> const & r) {

        if (m_pRefCounter)
            m_pRefCounter->releaseWeakRef();

        m_Pointer = r.m_Pointer;
        m_pRefCounter = r.m_pRefCounter;

        if (m_pRefCounter)
            m_pRefCounter->addWeakRef();

        return *this;
    }

    inline RefCountType use_count() const { return m_pRefCounter ? m_pRefCounter->getStrongRefCount() : 0; }
    inline bool expired() const { return m_pRefCounter && m_pRefCounter->getStrongRefCount() == 0; }
    
    shared_ptr<T, DeletePolicy> lock() const {

        //INFO: an assert might be useful here?
        return shared_ptr<T, DeletePolicy>(*this);
    }

    void reset() {

        if (m_pRefCounter) {

            m_pRefCounter->releaseWeakRef();
            m_pRefCounter = NULL;
            m_Pointer = NULL;
        }
    }

    void swap(weak_ptr<T> & b) {

        std::swap(m_Pointer, b.m_Pointer);
        std::swap(m_pRefCounter, b.m_pRefCounter);
    }

private:

    T* m_Pointer;
    smart_ptr_RefCounter* m_pRefCounter;
};

/*
template<class T, class U>
bool operator<(weak_ptr<T> const & a, weak_ptr<U> const & b);
*/

template<class T>
inline void swap(weak_ptr<T> & a, weak_ptr<T> & b) {

    a.swap(b);
}


#endif // CORE_SMART_PTR_H
