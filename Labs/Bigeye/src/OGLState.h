#ifndef _INCLUDED_BIGEYE_OGLSTATE_H
#define _INCLUDED_BIGEYE_OGLSTATE_H

#include "OGL.h"
#include "Assert.h"
#include <vector>


namespace BE
{
	class OGLState
	{
	public:

								OGLState()			: mStateManagerIndex(-1), mStateParent(NULL) {}

		virtual					~OGLState()			{}

	private:

		virtual void			Set()				{}

		virtual const OGLState*	GetParent() const	{ return mStateParent; }
		virtual OGLState*		GetParent()			{ return mStateParent; }

		
		friend class OGLStateManager;

		int			mStateManagerIndex;
		int			mStateTreeLevel;
		int			mStateTreeStartChildIndex;
		int			mStateTreeChildCount;
		OGLState*	mStateParent;
	};

	class OGLStateManager
	{
	public:

							OGLStateManager();
							~OGLStateManager();

		OGLState*			Enable(OGLState* inState) const;		
		OGLState*			Enable(int inIndex) const				{ return Enable(mStates[inIndex]); }
		OGLState*			GetState(int inIndex)					{ return mStates[inIndex]; }
		const OGLState*		GetState(int inIndex) const				{ return mStates[inIndex]; }
		
		void				StartBuild(int inStateCount);
		void				BuildSetState(OGLState* inState, int inIndex, OGLState* inParentState = NULL);
		void				BuildSetState(OGLState* inState, int inIndex, int inParentIndex)
																	{ gAssert(mStates[inParentIndex] != NULL); BuildSetState(inState, inIndex, mStates[inParentIndex]); }
		void				EndBuild();
		
	protected:

		#pragma pack(push)  /* push current alignment to stack */
		#pragma pack(1)     /* set alignment to 1 byte boundary */
		struct StateRelationship
		{
			unsigned int is_valid:1;
			unsigned int is_parent:1;
			unsigned int is_child:1;
			unsigned int is_ancestor:1;
			unsigned int is_descendant:1;

			StateRelationship() : is_valid(0) {}
		};
		#pragma pack(pop)   /* restore original alignment from stack */

		typedef std::vector<OGLState*> aStatePtr;
		typedef std::vector<StateRelationship> aStateRelationship;
		typedef std::vector<int> aStateIndex;


		void				SetParentRelationship(OGLState& inParent, OGLState& inChild)
																	{ SetParentRelationship(inParent.mStateManagerIndex, inChild.mStateManagerIndex); }

		void				SetParentRelationship(int inParent, int inChild);
		void				SetAncestorRelationship(int inAncestor, int inDescendant);
		bool				IsParent(int inParent, int inChild) const;
		bool				IsAncestor(int inParent, int inChild) const;
		OGLState*			FindCloseAncestor(OGLState* inAncestor, int inTarget) const;

		void				DepthWalkAndSetBranchRelationships(aStatePtr& ioWalkStack, OGLState* inNode);
		void				BreadthWalkAndAddChildren(aStateIndex& ioIndices,  OGLState* inNode);
																	

		enum EState
		{
			EINVALID,
			EBUILDING,
			EBUILT
		};

		EState				mState;
		int					mStateCount;
		aStatePtr			mStates;
		aStatePtr			mRootStates;
		aStateIndex			mStateChildren;
		aStateRelationship	mStateRelationships;
		mutable aStatePtr	mCurrentStateStack;
	};
}

#endif