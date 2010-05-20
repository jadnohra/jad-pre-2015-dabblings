#include "OGLState.h"

namespace BE 
{

OGLStateManager::~OGLStateManager()
{
	for (size_t i=0; i<mStates.size(); ++i)
	{
		delete mStates[i];
		mStates[i] = NULL;
	}
}


void OGLStateManager::SetState(OGLState* inState) const
{
	if (inState != mCurrState)
	{
		if (mCurrState != NULL)
			mCurrState->Unset();

		if (inState != NULL)
			inState->Set();

		mCurrState = inState;
	}
}


void OGLStateManager::SetState(int inIndex) const
{
	SetState(mStates[inIndex]);
}


int OGLStateManager::AddState(OGLState* inState)
{
	inState->mStateManagerIndex = (int) mStates.size();
	mStates.push_back(inState);

	return inState->mStateManagerIndex;
}

void OGLStateManager::Reserve(int inEndIndex)
{
	if (inEndIndex+1 > (int) mStates.size())
	{
		int start_index = (int) mStates.size();
		mStates.resize(inEndIndex+1);

		for (size_t i=start_index; i<mStates.size(); ++i)
		{
			mStates[i] = NULL;
		}
	}
}

bool OGLStateManager::InsertState(OGLState* inState, int inIndex)
{
	if (mStates[inIndex] != NULL)
		return false;

	inState->mStateManagerIndex = inIndex;
	Reserve(inIndex);
	mStates[inIndex] = inState;

	return true;
}


}
