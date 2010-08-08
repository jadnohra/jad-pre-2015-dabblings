#include "LoaderBVH.h"
#include "3rdparty/strtk/strtk.hpp"

namespace BF
{

	// http://www.cs.wisc.edu/graphics/Courses/cs-838-1999/Jeff/ASF-AMC.html
	// http://www.cs.wisc.edu/graphics/Courses/cs-838-1999/Jeff/BVH.html
	// http://vipbase.net/amc2bvh/

	struct LoaderBVHLineParser
	{
	private:

		class JointTreeNode;

	public:

		LoaderBVHLineParser(Skeleton& outSkeleton, SkeletonAnimationFrames* outAnimFrames)
			:	mSkeleton(outSkeleton)
			,	mAnimFrames(outAnimFrames)
			,	mPredicate(" \t\r")
			,	mParseState(Parse_START)
			,	mErrorLine(-1)
			,	mCurrLine(0)
			,	mAutoJointNameCount(0)
			,	mFrameChannelCount(0)
			,	mFrameIndex(0)
		{
			mSkeleton.Destroy();

			if (mAnimFrames)
				mAnimFrames->Destroy();
		}

		void buildJointHierarchy(const JointTreeNode& node, int& ioChildIndex)
		{
			if (!node.mChildren.empty())
			{
				mSkeleton.mJointHierarchy.mJointChildrenInfos[node.mJointIndex].mFirstChildIndex = ioChildIndex;
				mSkeleton.mJointHierarchy.mJointChildrenInfos[node.mJointIndex].mChildCount = (int) (node.mChildren.size());

				for (size_t i=0; i<node.mChildren.size(); ++i)
				{
					mSkeleton.mJointHierarchy.mJointChildren[ioChildIndex++] = node.mChildren[i];
				}

				for (size_t i=0; i<node.mChildren.size(); ++i)
				{
					buildJointHierarchy(mJointTree[node.mChildren[i]], ioChildIndex);
				}
			}
			else
			{
				mSkeleton.mJointHierarchy.mJointChildrenInfos[node.mJointIndex].mFirstChildIndex = -1;
				mSkeleton.mJointHierarchy.mJointChildrenInfos[node.mJointIndex].mChildCount = 0;
			}
		}

		void SetError()
		{
			mErrorLine = mCurrLine;
		}

		inline void operator()(const std::string& s)
		{
			if (mErrorLine >= 0)
			{
				++mCurrLine;		 
				return;
			}

			printf(s.c_str());
			printf("\n");

			mStringList.clear();
			strtk::split(mPredicate,s, strtk::range_to_type_back_inserter(mStringList), strtk::split_options::default_mode);

			{
				std::vector<std::string>::iterator it = mStringList.begin();

				while (it!=mStringList.end())
				{
					if (it->empty())
						it = mStringList.erase(it);
					else
						++it;
				}
			}

			if (mStringList.empty())
				return;

			if (mStringList.size() == 1
				&& strtk::imatch(mStringList[0], "}"))
			{
				if (!mJointStack.empty())
				{
					mJointStack.pop_back();

					if (mJointStack.empty())
					{
						mSkeleton.mJointHierarchy.mJointChildrenInfos.resize(mJointTree.size());
						mSkeleton.mJointHierarchy.mJointChildren.resize(mJointTree.size());

						if (!mJointTree.empty())
						{
							int child_index = 0;
							buildJointHierarchy(mJointTree.front(), child_index);
						}

						mParseState = Parse_MOTION;
					}
					else
					{
						mParseState = Parse_JOINT;
					}
				}
				else
				{
					SetError();
				}
			}
			else
			{

				switch (mParseState)
				{
					case Parse_START:
					{
						for (size_t i=0; i<mStringList.size(); ++i)
						{
							if (strtk::imatch(mStringList[i], "HIERARCHY"))
							{
								mParseState = Parse_ROOT;
							}
						}
					}
					break;

					case Parse_ROOT:
					{
						bool name_was_set = false;

						for (size_t i=0; i<mStringList.size(); ++i)
						{
							if (strtk::imatch(mStringList[i], "ROOT"))
							{
								mParseState = Parse_ROOT_Open;
							}
							else if (mParseState == Parse_ROOT_Open)
							{
								if (mSkeleton.mJoints.size() == 0)
								{
									mSkeleton.mJointInfos.resize(1);
									mSkeleton.mJointInfos[0].mName = mStringList[i];
									name_was_set = true;
								}
							}
						}

						if (mSkeleton.mJoints.size() == 0)
						{
							if (!name_was_set)
							{
								mSkeleton.mJointInfos.resize(1);
								mSkeleton.mJointInfos[0].mName = "auto_joint_" + strtk::type_to_string(mAutoJointNameCount++);
							}

							mJointStack.push_back(0);

							mJointTree.resize(1);
							mJointTree[mJointStack.back()].mJointIndex = 0;

							mJointToTreeIndex.resize(1);
							mJointToTreeIndex[0] = 0;
						}

						if (mParseState != Parse_ROOT_Open)
							SetError();
					}
					break;

					case Parse_ROOT_Open:
					{
						for (size_t i=0; i<mStringList.size(); ++i)
						{
							if (strtk::imatch(mStringList[i], "{"))
							{
								mParseState = Parse_OFFSET;
							}
						}

						if (mParseState != Parse_OFFSET)
							SetError();
					}
					break;

					case Parse_OFFSET:
					{
						if (strtk::imatch(mStringList[0], "OFFSET"))
						{
							glm::vec3 offset;

							for (size_t i=1, dim=0; i<mStringList.size() && dim<3; ++i, ++dim)
							{
								if (!strtk::string_to_type_converter(mStringList[i], offset[dim]))
									SetError();
							}

							if (mErrorLine < 0)
							{
								if (mJointStack.back() != JOINT_END_SITE)
								{
									if (mSkeleton.mJoints.size()+1 != mSkeleton.mJointInfos.size())
									{
										SetError();
									}
									else
									{
										mSkeleton.mJoints.resize(mSkeleton.mJoints.size()+1);
										mSkeleton.mJoints.back().mLocalTransform.mPosition =  LoaderBVH::ToOpenGLTranslation(offset);
										mParseState = Parse_CHANNELS;
									}
								}
								else
								{
									mParseState = Parse_CHANNELS;
								}
							}
						}
						else
						{
							SetError();
						}
					}
					break;

					case Parse_CHANNELS:
					{
						int channel_count = 0;

						if ((strtk::imatch(mStringList[0], "CHANNELS"))
							&& (mStringList.size() >= 2)
							&& (strtk::string_to_type_converter(mStringList[1], channel_count))
							)
						{
							int channel_index = 0;
							EChannelType channels[6] = {Channel_Invalid, Channel_Invalid, Channel_Invalid, Channel_Invalid, Channel_Invalid, Channel_Invalid };

							for (size_t i=2, channel=0; i<mStringList.size() && channel<6; ++i, ++channel)
							{
								if (strtk::imatch(mStringList[i], "Xposition"))
									channels[channel_index++] = Channel_TranslationX;
								else if (strtk::imatch(mStringList[i], "Yposition"))
									channels[channel_index++] = Channel_TranslationY;
								else if (strtk::imatch(mStringList[i], "Zposition"))
									channels[channel_index++] = Channel_TranslationZ;
								else if (strtk::imatch(mStringList[i], "Xrotation"))
									channels[channel_index++] = Channel_RotationEulerX;
								else if (strtk::imatch(mStringList[i], "Yrotation"))
									channels[channel_index++] = Channel_RotationEulerY;
								else if (strtk::imatch(mStringList[i], "Zrotation"))
									channels[channel_index++] = Channel_RotationEulerZ;
								else
								{
									// We don't support user channels for now
									SetError();
								}
							}

							if (mErrorLine < 0)
							{
								int channel_info_index = mSkeleton.FindChannelInfo(channels);

								if (channel_info_index < 0)
								{
									channel_info_index = mSkeleton.CreateChannelInfo(channels);
								}

								mFrameChannelCount += (int) (mSkeleton.mChannelInfos[channel_info_index].mChannelTypes.size());
								mSkeleton.mJointInfos.back().mChannelInfoIndex = channel_info_index;
								mParseState = Parse_JOINT;
							}
						}
						else
						{
							SetError();
						}
					}
					break;

					case Parse_JOINT:
					{
						if (mStringList.size() == 2
							&& (strtk::imatch(mStringList[0], "End"))
							&& (strtk::imatch(mStringList[1], "Site"))
							)
						{
							mParseState = Parse_JOINT_Open;
							mJointStack.push_back(JOINT_END_SITE);
						}
						else
						{
							int joint_index = mSkeleton.mJointInfos.size();
							bool name_was_set = false;

							for (size_t i=0; i<mStringList.size(); ++i)
							{
								if (strtk::imatch(mStringList[i], "JOINT"))
								{
									mParseState = Parse_JOINT_Open;
								}
								else if (mParseState == Parse_JOINT_Open)
								{
									mSkeleton.mJointInfos.resize(mSkeleton.mJointInfos.size()+1);
									mSkeleton.mJointInfos.back().mName = mStringList[i];
									name_was_set = true;
								}
							}

							if (!name_was_set)
							{
								mSkeleton.mJointInfos.resize(joint_index+1);
								mSkeleton.mJointInfos.back().mName = "auto_joint_" + strtk::type_to_string(mAutoJointNameCount++);
							}

							mJointTree[mJointStack.back()].mChildren.push_back(joint_index);

							mJointStack.push_back(joint_index);

							mJointTree.resize(mJointTree.size()+1);
							mJointTree[mJointStack.back()].mJointIndex = joint_index;

							mJointToTreeIndex.resize(mJointToTreeIndex.size()+1);
							mJointToTreeIndex[joint_index] = (int) (mJointTree.size()-1);

							if (mParseState != Parse_JOINT_Open)
								SetError();
						}
					}
					break;

					case Parse_JOINT_Open:
					{
						for (size_t i=0; i<mStringList.size(); ++i)
						{
							if (strtk::imatch(mStringList[i], "{"))
							{
								mParseState = Parse_OFFSET;
							}
						}

						if (mParseState != Parse_OFFSET)
							SetError();
					}
					break;

					case Parse_MOTION:
					{
						for (size_t i=0; i<mStringList.size(); ++i)
						{
							if (strtk::imatch(mStringList[i], "MOTION"))
							{
								mParseState = Parse_FRAMES;
							}
						}

						if (mParseState != Parse_FRAMES)
							SetError();
					}
					break;

					case Parse_FRAMES:
					{
						for (size_t i=0; i<mStringList.size(); ++i)
						{
							if (strtk::imatch(mStringList[i], "FRAMES"))
							{
							}
							else if (strtk::imatch(mStringList[i], "FRAMES:"))
							{
								mParseState = Parse_FrameTime;
							}
							else if (strtk::imatch(mStringList[i], ":"))
							{
								mParseState = Parse_FrameTime;
							}
							else if (mParseState == Parse_FrameTime)
							{
								if (!strtk::string_to_type_converter(mStringList[i], mFrameCount))
									SetError();
							}
						}

						if (mParseState != Parse_FrameTime)
							SetError();
					}
					break;

					case Parse_FrameTime:
					{
						for (size_t i=0; i<mStringList.size(); ++i)
						{
							if (strtk::imatch(mStringList[i], "Frame"))
							{
							}
							else if (strtk::imatch(mStringList[i], "Time"))
							{
							}
							else if (strtk::imatch(mStringList[i], "FrameTime"))
							{
							}
							else if (strtk::imatch(mStringList[i], "Time:"))
							{
								mParseState = Parse_FrameDataLine;
							}
							else if (strtk::imatch(mStringList[i], "FrameTime:"))
							{
								mParseState = Parse_FrameDataLine;
							}
							else if (strtk::imatch(mStringList[i], ":"))
							{
								mParseState = Parse_FrameDataLine;
							}
							else if (mParseState == Parse_FrameDataLine)
							{
								if (!strtk::string_to_type_converter(mStringList[i], mFrameTime))
									SetError();
							}
						}

						if (mParseState != Parse_FrameDataLine)
							SetError();
					}
					break;

					case Parse_FrameDataLine:
					{
						if (mStringList.size() == mFrameChannelCount)
						{
							if ((mFrameIndex == 0) || (mAnimFrames != NULL))
							{
								if (mAnimFrames)
								{
									mAnimFrames->mSkeletonAnimationFrames.resize(mAnimFrames->mSkeletonAnimationFrames.size()+1);
									mAnimFrames->mSkeletonAnimationFrames.back().mJointOrientations.resize(mJointTree.size());
								}

								if (mFrameIndex == 0)
								{
									if (mAnimFrames)
										mAnimFrames->mFrameTime = mFrameTime;
									mSkeleton.mDefaultPose.mJointOrientations.resize(mJointTree.size());
								}

								int data_index = 0;

								for (size_t joint_index=0; joint_index<mJointTree.size(); ++joint_index)
								{
									const JointTreeNode& node = mJointTree[joint_index];

									const JointInfo& joint_info = mSkeleton.mJointInfos[node.mJointIndex];
									const ChannelInfo& channel_info = mSkeleton.mChannelInfos[joint_info.mChannelInfoIndex];

									if (!channel_info.mChannelTypes.empty())
									{
										bool translation_was_set = false;
										glm::vec3 translation;
										bool rotation_was_set = false;
										glm::vec3 euler_angles_deg;

										for (int channel_index=0; channel_index<channel_info.mChannelTypes.size(); ++channel_index)
										{
											switch (channel_info.mChannelTypes[channel_index])
											{
												case Channel_TranslationX:
												{
													if (!strtk::string_to_type_converter(mStringList[data_index++], translation.x))
														SetError();
													else
														translation_was_set = true;
												}
												break;

												case Channel_TranslationY:
												{
													if (!strtk::string_to_type_converter(mStringList[data_index++], translation.y))
														SetError();
													else
														translation_was_set = true;
												}
												break;

												case Channel_TranslationZ:
												{
													if (!strtk::string_to_type_converter(mStringList[data_index++], translation.z))
														SetError();
													else
														translation_was_set = true;
												}
												break;

												case Channel_RotationEulerX:
												{
													if (!strtk::string_to_type_converter(mStringList[data_index++], euler_angles_deg.x))
														SetError();
													else
														rotation_was_set = true;
												}
												break;

												case Channel_RotationEulerY:
												{
													if (!strtk::string_to_type_converter(mStringList[data_index++], euler_angles_deg.y))
														SetError();
													else
														rotation_was_set = true;
												}
												break;

												case Channel_RotationEulerZ:
												{
													if (!strtk::string_to_type_converter(mStringList[data_index++], euler_angles_deg.z))
														SetError();
													else
														rotation_was_set = true;
												}
												break;
											}
										}

										if (mErrorLine < 0)
										{
											if (translation_was_set)
											{
												if (node.mJointIndex == 0 && mAnimFrames)
													mAnimFrames->mSkeletonAnimationFrames.back().mRootTranslation = LoaderBVH::ToOpenGLTranslation(translation);

												if (mFrameIndex == 0)
												{
													//mSkeleton.mJoints[node.mJointIndex].mLocalTransform.mPosition += LoaderBVH::ToOpenGLTranslation(translation);
												}
											}
											
											if (rotation_was_set)
											{
												glm::quat orientation = LoaderBVH::ToOpenGLOrientation(euler_angles_deg);

												if (mFrameIndex == 0)
												{
													mSkeleton.mJoints[node.mJointIndex].mLocalTransform.mOrientation = orientation;
													mSkeleton.mDefaultPose.mJointOrientations[node.mJointIndex] = orientation;
												}

												if (mAnimFrames)
													mAnimFrames->mSkeletonAnimationFrames.back().mJointOrientations[node.mJointIndex] = orientation;
											}
										}
									}
								}
							}

							++mFrameIndex;
						}
						else
						{
							SetError();
						}
					}
					break;
				}
			}

			if (mErrorLine >= 0)
				mSkeleton.Destroy();

			++mCurrLine;
		}

	private:

		Skeleton& mSkeleton;
		SkeletonAnimationFrames* mAnimFrames;

		std::string mDelimiters;
		strtk::multiple_char_delimiter_predicate mPredicate;
		std::vector<std::string> mStringList;

		enum EParseState
		{
			Parse_START, Parse_HIERARCHY, Parse_ROOT, Parse_ROOT_Open, Parse_OFFSET, Parse_CHANNELS, Parse_JOINT, Parse_JOINT_Open, Parse_MOTION, Parse_FRAMES, Parse_FrameTime, Parse_FrameDataLine
		};

		enum { JOINT_END_SITE = -2 };

		int mCurrLine;
		int mErrorLine;

		EParseState mParseState;
		int mAutoJointNameCount;
		int mFrameChannelCount;
		int mFrameCount;
		int mFrameIndex;
		float mFrameTime;
		
		class JointTreeNode
		{
		public:

			JointTreeNode(int index = -1)
				:	mJointIndex(index)
			{
			}

			int mJointIndex;
			std::vector<int> mChildren;
		};

		std::deque<JointTreeNode> mJointTree;
		std::deque<int> mJointStack;
		std::vector<int> mJointToTreeIndex;
	};

	bool LoaderBVH::Load(const char* inFilePath, Skeleton& outSkeleton, SkeletonAnimationFrames* outAnimFrames)
	{
		if (inFilePath == NULL)
			return false;

		LoaderBVHLineParser line_parser(outSkeleton, outAnimFrames);

		printf(inFilePath);
		printf("\n");

		strtk::for_each_line(std::string(inFilePath), line_parser);

		return !outSkeleton.mJoints.empty();	
	}


}