#ifndef _INCLUDED_BIGEYE_OGLSTATE_H
#define _INCLUDED_BIGEYE_OGLSTATE_H

#include "OGL.h"
#include <vector>

namespace BE
{
	class OGLState
	{
	public:

		OGLState() : mStateManagerIndex(-1) {}

		virtual ~OGLState() {}
		virtual void Set()	{}
		virtual void Unset(){}

	private:

		friend class OGLStateManager;

		int mStateManagerIndex;
	};

	class OGLStateManager
	{
	public:

		OGLStateManager() : mCurrState(NULL) {}
		~OGLStateManager();

		void SetState(OGLState* inState) const;
		void SetState(int inIndex) const;
		OGLState* GetState(int inIndex)			{ return mStates[inIndex]; }
		int AddState(OGLState* inState);
		void Reserve(int inEndIndex);
		bool InsertState(OGLState* inState, int inIndex);
		
	protected:

		typedef std::vector<OGLState*> aStatePtr;
		aStatePtr mStates;
		mutable OGLState* mCurrState;
	};
}

#endif