#include "OGLState.h"


namespace BE 
{

OGLStateManager::OGLStateManager() : mState(EINVALID), mStateCount(0)
{
}


OGLStateManager::~OGLStateManager()
{
	for (size_t i=0; i<mStates.size(); ++i)
	{
		delete mStates[i];
		mStates[i] = NULL;
	}
}


void OGLStateManager::StartBuild(int inStateCount)
{
	gAssert(mState == EINVALID);
	mState = EBUILDING;

	mStateCount = inStateCount;
	mStates.resize(inStateCount);
	mStateRelationships.resize(inStateCount*inStateCount);
}


void OGLStateManager::BuildSetState(OGLState* inState, int inIndex, OGLState* inParentState)
{
	gAssert(mStates[inIndex] == NULL);
	gAssert(mState == EBUILDING);
	gAssert(inState->mStateParent == NULL);

	mStates[inIndex] = inState;
	inState->mStateManagerIndex = inIndex;
	
	if (inParentState)
		inState->mStateParent = inParentState;
}


void OGLStateManager::SetParentRelationship(int inParent, int inChild)
{ 
	StateRelationship& parent_rel = mStateRelationships[inParent + mStateCount*inChild];

	gAssert(parent_rel.is_valid==0 || (parent_rel.is_child==0 && parent_rel.is_descendant == 0));

	parent_rel.is_valid = 1;
	parent_rel.is_parent = 1;
	parent_rel.is_ancestor = 1;
	parent_rel.is_child = 0;
	parent_rel.is_descendant = 0;

	StateRelationship& child_rel = mStateRelationships[inChild + mStateCount*inParent];

	gAssert(child_rel.is_valid==0 || (child_rel.is_parent==0 && child_rel.is_ancestor==0));

	child_rel.is_valid = 1;
	child_rel.is_parent = 0;
	child_rel.is_ancestor = 0;
	child_rel.is_child = 1;
	child_rel.is_descendant = 1;
}


void OGLStateManager::SetAncestorRelationship(int inAncestor, int inDescendant)
{
	StateRelationship& parent_rel = mStateRelationships[inAncestor + mStateCount*inDescendant];

	gAssert(parent_rel.is_valid==0);
	parent_rel.is_ancestor = 1;

	StateRelationship& child_rel = mStateRelationships[inDescendant + mStateCount*inAncestor];

	gAssert(child_rel.is_valid==0);
	child_rel.is_descendant = 1;
}


bool OGLStateManager::IsParent(int inParent, int inChild) const
{
	const StateRelationship& parent_rel = mStateRelationships[inParent + mStateCount*inChild];
	return parent_rel.is_valid && parent_rel.is_parent == 1;
}


bool OGLStateManager::IsAncestor(int inParent, int inChild) const
{
	const StateRelationship& parent_rel = mStateRelationships[inParent + mStateCount*inChild];
	return parent_rel.is_valid && parent_rel.is_ancestor == 1;
}


void OGLStateManager::DepthWalkAndSetBranchRelationships(aStatePtr& ioWalkStack, OGLState* inNode)
{
	if (inNode != NULL)
	{
		inNode->mStateTreeLevel = (int) ioWalkStack.size();
		int node_index = inNode->mStateManagerIndex;

		if (ioWalkStack.size() > 1)
		{
			for (size_t i=1; i<ioWalkStack.size(); ++i)
			{
				SetAncestorRelationship(ioWalkStack[i]->mStateManagerIndex, node_index);
			}
		}

		ioWalkStack.push_back(inNode);
		for (int i=0; i<mStateCount; ++i)
		{
			if (IsParent(node_index, i))
			{
				DepthWalkAndSetBranchRelationships(ioWalkStack, mStates[i]);
			}
		}

		ioWalkStack.pop_back();
	}
}


void OGLStateManager::BreadthWalkAndAddChildren(aStateIndex& ioIndices,  OGLState* inNode)
{
	int node_index = inNode->mStateManagerIndex;

	for (int i=0; i<mStateCount; ++i)
	{
		if (IsParent(node_index, i))
		{
			if (inNode->mStateTreeStartChildIndex == -1)
			{
				inNode->mStateTreeStartChildIndex = (int) ioIndices.size();
				inNode->mStateTreeChildCount = 0;
			}

			ioIndices.push_back(i);
			++inNode->mStateTreeChildCount;
		}
	}

	if (inNode->mStateTreeStartChildIndex >= 0)
	{
		int child_index = inNode->mStateTreeStartChildIndex;
		int end_index = ioIndices.size();

		for (int i=child_index; i<end_index; ++i)
		{
			BreadthWalkAndAddChildren(ioIndices, mStates[ioIndices[i]]);
		}
	}
}


void OGLStateManager::EndBuild()
{
	gAssert(mState == EBUILDING);
	
	int root_count = 0;

	for (size_t i=0; i<mStates.size(); ++i)
	{
		if (mStates[i] != NULL)
		{
			mStates[i]->mStateTreeLevel = -1;
			mStates[i]->mStateTreeStartChildIndex = -1;
			mStates[i]->mStateTreeChildCount = 0;

			if (mStates[i]->mStateParent != NULL)
			{
				SetParentRelationship(*mStates[i]->mStateParent, *mStates[i]);
			}
			else
			{
				++root_count;
			}
		}
	}

	if (root_count > 0)
	{
		mRootStates.resize(root_count);

		int root_index = 0;
		for (size_t i=0; i<mStates.size(); ++i)
		{
			if (mStates[i] != NULL)
			{
				if (mStates[i]->mStateParent == NULL)
				{
					mRootStates[root_index++] = mStates[i];
				}
			}
		}

		aStatePtr walk_stack;

		for (size_t i=0; i<mRootStates.size(); ++i)
		{
			DepthWalkAndSetBranchRelationships(walk_stack, mRootStates[i]);
			gAssert(walk_stack.empty());
		}

		mStateChildren.reserve(mStates.size());
		for (size_t i=0; i<mRootStates.size(); ++i)
		{
			BreadthWalkAndAddChildren(mStateChildren, mRootStates[i]);
		}
	}

	mState = EBUILT;
}


OGLState* OGLStateManager::FindCloseAncestor(OGLState* inAncestor, int inTarget) const
{
	for (int i=0, j=inAncestor->mStateTreeStartChildIndex; i<inAncestor->mStateTreeChildCount; ++i, ++j)
	{
		if (IsParent(mStateChildren[j], inTarget))
			return mStates[mStateChildren[j]];
	}

	return NULL;
}


OGLState* OGLStateManager::Enable(OGLState* inState) const
{
	int state_index = inState->mStateManagerIndex;
	
	if (inState->mStateTreeLevel != 0)
	{
		OGLState* closest_ancestor = NULL;

		if (!mCurrentStateStack.empty())
		{
			OGLState* top_state = mCurrentStateStack.back();

			if (top_state == inState)
				return inState;


			if (IsParent(top_state->mStateManagerIndex, state_index))
			{
				inState->Set();
				mCurrentStateStack.push_back(inState);
				return inState;
			}

			if (IsAncestor(state_index, top_state->mStateManagerIndex))
			{
				return inState;
			}
			else
			{
				if (inState->mStateTreeLevel > 0)
				{
					mCurrentStateStack.resize(inState->mStateTreeLevel);

					while (!mCurrentStateStack.empty())
					{
						if (IsAncestor(mCurrentStateStack.back()->mStateManagerIndex, state_index))
							break;
					} 

					if (!mCurrentStateStack.empty())
					{
						closest_ancestor = mCurrentStateStack.back();
					}
				}
			}
		}

		if (closest_ancestor == NULL)
		{
			for (size_t i=0; i<mRootStates.size(); ++i)
			{
				if (IsAncestor(mRootStates[i]->mStateManagerIndex, state_index))
				{
					closest_ancestor = mRootStates[i];
					break;
				}
			}
		}

		while (!IsParent(closest_ancestor->mStateManagerIndex, state_index))
		{
			closest_ancestor = FindCloseAncestor(closest_ancestor, state_index);
			closest_ancestor->Set();
			mCurrentStateStack.push_back(closest_ancestor);
		}

		inState->Set();
		mCurrentStateStack.push_back(inState);
		
		return inState;
	}
	else
	{
		mCurrentStateStack.clear();
		inState->Set();
		mCurrentStateStack.push_back(inState);

		return inState;
	}
}

}
