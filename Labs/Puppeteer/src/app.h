#include "NeHeGL.h"
#include "mesh.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/type_ptr.hpp"
#include "glm/gtx/transform2.hpp"
#include "arb_multisample.h"
#define FREEGLUT_STATIC
#include <gl\freeglut.h>			// Header File For The GLu32 Library
#include "Random.h"
#include <vector>
#include <algorithm>
#include <limits>
#include "Skeleton.h"
#include "LoaderBVH.h"

class App
{
public:

	struct MotionClip
	{
		// z is height in local space

		glm::mat4 mWorldMatrix;
		float mLength;
		float mRadius;
		glm::vec4 mColor;

		MotionClip()
		{
			mRadius = 0.25f;
			mLength = 1.0f;
		}

		void Draw(const glm::mat4& inViewMatrix, bool inFocus, float inFocusCylinderLength)
		{
			glMatrixMode(GL_MODELVIEW);
			glLoadMatrixf(glm::value_ptr(inViewMatrix * mWorldMatrix));

			GLfloat mat_ambient[]= { 0.2f, 0.2f, 0.2f, mColor.w };
			GLfloat mat_diffuse[]= { mColor.x, mColor.y, mColor.z, mColor.w };
			GLfloat mat_specular[]= { 0.2f, 0.2f, 0.2f, mColor.w };
			GLfloat mat_shininess[]= { 1.0f };

			glMaterialfv (GL_FRONT, GL_AMBIENT, mat_ambient);
			glMaterialfv (GL_FRONT, GL_DIFFUSE, mat_diffuse);
			glMaterialfv (GL_FRONT, GL_SPECULAR, mat_specular);
			glMaterialfv (GL_FRONT, GL_SHININESS, mat_shininess);

			glColor4f(mColor.x, mColor.y, mColor.z, mColor.w);
			//glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
			glutSolidCylinder(inFocus ? 1.3f * mRadius : mRadius, mLength, 20, 1);
			//glutWireCylinder(0.5f, 2.0f, 20, 1);
			//glutSolidCube(2.0f);

			if (inFocus)
			{
				float marker_length = 0.2f;
				glm::mat4 marker_matrix = glm::translate(mWorldMatrix, glm::vec3(0.0f, 0.0f, inFocusCylinderLength - (marker_length * 0.5f)));

				glMatrixMode(GL_MODELVIEW);
				glLoadMatrixf(glm::value_ptr(inViewMatrix * marker_matrix));

				glDisable (GL_LIGHTING);
				glColor4f(0.0f, 0.0f, 0.0f, 1.0f);

				glutSolidCylinder(1.5f * mRadius, marker_length, 20, 1);

				glEnable (GL_LIGHTING);
			}
		}

		bool IntersectRay(const glm::vec3& pos, const glm::vec3& dir, float& line_t, float& cylinder_length)
		{
			//http://mrl.nyu.edu/~dzorin/intro-graphics/lectures/lecture11/sld002.htm

			glm::mat4 world_to_cylinder_transform =  glm::inverse(mWorldMatrix);;
			glm::vec3 local_ray_pos = glm::vec3(world_to_cylinder_transform * glm::vec4(pos.x, pos.y, pos.z, 1.0f));
			glm::vec3 local_ray_dir = glm::vec3(world_to_cylinder_transform * glm::vec4(dir.x, dir.y, dir.z, 0.0f));

			float solution_t[4];
			int solution_count = 0;

			// side surface intersect
			{
				float A = local_ray_dir.x*local_ray_dir.x + local_ray_dir.y*local_ray_dir.y;
				float B = 2.0f* (local_ray_pos.x*local_ray_dir.x + local_ray_pos.y*local_ray_dir.y);
				float C = local_ray_pos.x*local_ray_pos.x + local_ray_pos.y*local_ray_pos.y - mRadius*mRadius;
				
				float disc = B*B - 4.0f*A*C;

				if (disc <= 0.0f)
					return false;

				float disc_sqrt = sqrtf(disc);

				float t1 = (-B - disc_sqrt) / (2.0f*A); 
				float t2 = (-B + disc_sqrt) / (2.0f*A); 
				
				float t1_z = local_ray_pos.z + local_ray_dir.z * t1;
				float t2_z = local_ray_pos.z + local_ray_dir.z * t2;

				if (t1_z >= 0.0f && t1_z < mLength)
					solution_t[solution_count++] = t1;

				if (t2_z >= 0.0f && t2_z < mLength)
					solution_t[solution_count++] = t2;
			}

			// caps
			{
				// http://local.wasp.uwa.edu.au/~pbourke/geometry/planeline/

				glm::vec3 plane_normal(0.0f, 0.0f, 1.0f);
				glm::vec3 plane_cap1_pos(0.0f, 0.0f, 0.0f);
				glm::vec3 plane_cap2_pos(0.0f, 0.0f, mLength);

				float denom = glm::dot(plane_normal, local_ray_dir);

				if (denom != 0.0f)
				{
					float cap1_t = glm::dot(plane_normal, plane_cap1_pos-local_ray_pos) / denom;
					float cap2_t = glm::dot(plane_normal, plane_cap2_pos-local_ray_pos) / denom;

					glm::vec3 cap1_t_inter = local_ray_pos + local_ray_dir * cap1_t;
					glm::vec3 cap2_t_inter = local_ray_pos + local_ray_dir * cap2_t;

					if (cap1_t_inter.x*cap1_t_inter.x + cap1_t_inter.y*cap1_t_inter.y < mRadius*mRadius) 
						solution_t[solution_count++] = cap1_t;

					if (cap2_t_inter.x*cap2_t_inter.x + cap2_t_inter.y*cap2_t_inter.y < mRadius*mRadius) 
						solution_t[solution_count++] = cap2_t;

				}
			}

			line_t = solution_t[0];

			for (int i=1; i<solution_count; ++i)
			{
				if (solution_t[i] < line_t)
					line_t = solution_t[i];
			}

			cylinder_length = local_ray_pos.z + local_ray_dir.z * line_t;

			return solution_count > 0;
		}
	};

	typedef std::vector<MotionClip> MotionClips;
	typedef std::vector<int> Integers;

	struct MotionClipCameraSpaceSort
	{
		const glm::mat4& mViewMatrix;
		const MotionClips& mMotionClips;

		MotionClipCameraSpaceSort(const glm::mat4& inViewMatrix, const MotionClips& inMotionClips)
			:	mViewMatrix(inViewMatrix)
			,	mMotionClips(inMotionClips)
		{
		}

		bool operator()(int inComp, int inRef) const
		{
			glm::vec3 comp_pos((mViewMatrix * mMotionClips[inComp].mWorldMatrix)[3]);
			glm::vec3 ref_pos((mViewMatrix * mMotionClips[inRef].mWorldMatrix)[3]);

			return comp_pos.z < ref_pos.z;
		}
	};

	

	Mesh mMesh;
	GL_Window*	mWindow;
	
	glm::mat4 mCameraWorldMatrix;
	glm::mat4 mCameraViewMatrix;
	MotionClips mMotionClips;
	Integers mCameraSortedMotionClips;
	int mFocusClip;
	float mFocusClipCylinderLength;

	BF::Skeleton mTestSkeleton;
	BF::JointTransforms mTestSkeletonJoints;
	BF::SkeletonAnimationFrames mTestSkeletonAnimFrames;
	int mTestAnimFrame;
	float mTestAnimFrameTime;

	void End()
	{
	}

	bool Load(GL_Window* pWindow)
	{
		//BF::LoaderBVH::Load("media/test_xyz_rot.bvh", mTestSkeleton);
		//BF::LoaderBVH::Load("media/test.bvh", mTestSkeleton);
		BF::LoaderBVH::Load("media/Male1_C03_Run.bvh", mTestSkeleton, &mTestSkeletonAnimFrames);
		mTestSkeleton.ToModelSpace(mTestSkeleton.mDefaultPose, true, mTestSkeletonJoints);
		mTestAnimFrame = -1;
		mTestAnimFrameTime = 0.0f;

		mFocusClip = -1;
		mWindow = pWindow;
		int argp = 0;
		glutInit(&argp, NULL);
		//mMesh.LoadWavefrontObj("../media/monkey_smooth.obj", false, false, false);

		//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 4 );
		//SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		//SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		//SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		//SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		
		glEnable(GL_POINT_SMOOTH);
		glHint (GL_POINT_SMOOTH_HINT, GL_NICEST);
		glEnable(GL_LINE_SMOOTH);
		glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
		glEnable(GL_POLYGON_SMOOTH);
		glHint (GL_POLYGON_SMOOTH_HINT, GL_NICEST);
		
		//with lighting
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glEnable(GL_COLOR_MATERIAL);
		
		// without lighting
		//glColor4f(1.0f, 1.0f, 1.0f, 0.8f);	// Full brightness, 50% alpha
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// Source and destination blending factors
		
		glEnable(GL_BLEND);

		glEnable( GL_TEXTURE_2D );
		glEnable( GL_DEPTH_TEST );

		glEnable (GL_LIGHTING);

		glEnable(GL_CULL_FACE);

		mCameraWorldMatrix = glm::translate(mCameraWorldMatrix, glm::vec3(0.0f, 0.0f, 5.0f));

		//if (!mTestSkeletonJoints.empty())
		//{
		//	mCameraWorldMatrix = glm::lookAt(glm::vec3(mCameraWorldMatrix[3]), mTestSkeletonJoints[2].mPosition, glm::vec3(0.0f, 1.0f, 0.0f));
		//}

		Random random;
		mMotionClips.resize(100);

		glm::mat4 mat_identity;
		float range_max = (float) mMotionClips.size() * 0.1f;
		float range_min = -range_max;
		
		for (int i=0; i<mMotionClips.size(); ++i)
		{
			mMotionClips[i].mLength = random.randr(1.0f, 8.0f);
			mMotionClips[i].mColor = glm::vec4(random.randr(1.0f), random.randr(1.0f), random.randr(1.0f), random.randr(0.3f, 0.7f));
			mMotionClips[i].mWorldMatrix = glm::translate(mat_identity, glm::vec3(random.randr(range_min, range_max), random.randr(range_min, range_max), random.randr(range_min, range_max)));
		}

		return true;
	}

	bool IsKeyDown(int key)
	{
		return (bool) (mWindow->keys->keyDown[key]);
	}

	void Update(unsigned int milliseconds)
	{
		if (IsKeyDown(VK_ESCAPE))
			TerminateApplication(mWindow);

		float dt = (float) milliseconds / 1000.0f;

		glm::vec3 cam_translate;
		float translate_scale = 10.0f;

		if (IsKeyDown('A'))
		{
			cam_translate.x -= translate_scale;
		}
		else if (IsKeyDown('D'))
		{
			cam_translate.x += translate_scale;
		}

		if (IsKeyDown('W'))
		{
			cam_translate.z -= translate_scale;
		}
		else if (IsKeyDown('S'))
		{
			cam_translate.z += translate_scale;
		}

		if (IsKeyDown('R'))
		{
			cam_translate.y += translate_scale;
		}
		else if (IsKeyDown('F'))
		{
			cam_translate.y -= translate_scale;
		}

		mCameraWorldMatrix = glm::translate(mCameraWorldMatrix, cam_translate * dt);

		float rotate_scale = 60.0f;
		glm::vec3 cam_rotate_euler;

		if (IsKeyDown(VK_LEFT))
		{
			cam_rotate_euler.y += rotate_scale;
		}
		else if (IsKeyDown(VK_RIGHT))
		{
			cam_rotate_euler.y -= rotate_scale;
		}

		if (IsKeyDown(VK_UP))
		{
			cam_rotate_euler.x += rotate_scale;
		}
		else if (IsKeyDown(VK_DOWN))
		{
			cam_rotate_euler.x -= rotate_scale;
		}

		if (IsKeyDown('Q'))
		{
			cam_rotate_euler.z += rotate_scale;
		}
		else if (IsKeyDown('E'))
		{
			cam_rotate_euler.z -= rotate_scale;
		}

		if (IsKeyDown('B'))
		{
			glDisable(GL_BLEND);
		}
		else
		{
			glEnable(GL_BLEND);
		}

		/*
		if (IsKeyDown('W'))
		{
			cam_translate.z -= scale;
		}
		else if (IsKeyDown('S'))
		{
			cam_translate.z += scale;
		}

		if (IsKeyDown('R'))
		{
			cam_translate.y -= scale;
		}
		else if (IsKeyDown('F'))
		{
			cam_translate.y += scale;
		}
		*/

		mCameraWorldMatrix = glm::rotate(mCameraWorldMatrix, cam_rotate_euler.x * dt, glm::vec3(1.0f, 0.0f, 0.0f));
		mCameraWorldMatrix = glm::rotate(mCameraWorldMatrix, cam_rotate_euler.y * dt, glm::vec3(0.0f, 1.0f, 0.0f));
		mCameraWorldMatrix = glm::rotate(mCameraWorldMatrix, cam_rotate_euler.z * dt, glm::vec3(0.0f, 0.0f, 1.0f));

		mCameraViewMatrix = glm::inverse(mCameraWorldMatrix);

		if (mCameraSortedMotionClips.size() != mMotionClips.size())
		{
			mCameraSortedMotionClips.resize(mMotionClips.size());

			for (int i=0; i<mCameraSortedMotionClips.size(); ++i)
				mCameraSortedMotionClips[i] = i;
		}

		std::sort(mCameraSortedMotionClips.begin(), mCameraSortedMotionClips.end(), MotionClipCameraSpaceSort(mCameraViewMatrix, mMotionClips));

		mTestAnimFrameTime += dt;
		if (mTestAnimFrameTime >= mTestSkeletonAnimFrames.mFrameTime)
		{
			while (mTestAnimFrameTime >= mTestSkeletonAnimFrames.mFrameTime)
				mTestAnimFrameTime -= mTestSkeletonAnimFrames.mFrameTime;

			mTestAnimFrame = ((mTestAnimFrame + 1) % mTestSkeletonAnimFrames.mSkeletonAnimationFrames.size());
		}
	}

	void DrawTestSkeleton(int inJointIndex)
	{
		glLoadMatrixf(glm::value_ptr(mCameraViewMatrix * glm::translate(glm::mat4(), mTestSkeletonJoints[inJointIndex].mPosition)));
		glutSolidSphere(0.25f, 10.0f, 10.0f);
		
		int child_count = mTestSkeleton.mJointHierarchy.mJointChildrenInfos[inJointIndex].mChildCount;

		if (child_count > 0)
		{
			int child_index = mTestSkeleton.mJointHierarchy.mJointChildrenInfos[inJointIndex].mFirstChildIndex;

			for (int i=0; i<child_count; ++i)
			{
				int child_joint_index = mTestSkeleton.mJointHierarchy.mJointChildren[child_index++];

				glLoadMatrixf(glm::value_ptr(mCameraViewMatrix));
				glBegin(GL_LINES);
				glVertex3f(mTestSkeletonJoints[inJointIndex].mPosition.x, mTestSkeletonJoints[inJointIndex].mPosition.y, mTestSkeletonJoints[inJointIndex].mPosition.z); 
				glVertex3f(mTestSkeletonJoints[child_joint_index].mPosition.x, mTestSkeletonJoints[child_joint_index].mPosition.y, mTestSkeletonJoints[child_joint_index].mPosition.z); 
				glEnd( );

				DrawTestSkeleton(child_joint_index);
			}
		}
	}

	void DrawTestSkeleton()
	{
		if (mTestSkeletonJoints.empty())
			return;

		if (!mTestSkeletonAnimFrames.mSkeletonAnimationFrames.empty()
			&& mTestAnimFrame >= 0)
		{
			mTestSkeleton.ToModelSpace(mTestSkeletonAnimFrames.mSkeletonAnimationFrames[mTestAnimFrame], true, mTestSkeletonJoints);
		}

		glEnable(GL_COLOR_MATERIAL);
		glMatrixMode(GL_MODELVIEW);
		glColor4f(0.0f, 0.0f, 0.0f, 0.5f);

		DrawTestSkeleton(0);
		
		/*
		for (int i=0; i<mTestSkeletonJoints.size(); ++i)
		{
			glLoadMatrixf(glm::value_ptr(mCameraViewMatrix * glm::translate(glm::mat4(), mTestSkeletonJoints[i].mPosition)));

			glutSolidSphere(1.0f, 10.0f, 10.0f);
		}
		*/

		glDisable(GL_COLOR_MATERIAL);
	}

	bool Draw()
	{
		POINT mouse;						// Stores The X And Y Coords For The Current Mouse Position
		GetCursorPos(&mouse);					// Gets The Current Cursor Coordinates (Mouse Coordinates)
		ScreenToClient(mWindow->hWnd, &mouse);

		if (IsKeyDown(VK_PAUSE))
		{
			mWindow->keys->keyDown[VK_PAUSE] = false;
			DebugBreak();
		}

		glEnable(GL_SCISSOR_TEST);

		RECT	rect;										// Holds Coordinates Of A Rectangle

		GetClientRect(mWindow->hWnd, &rect);							// Get Window Dimensions
		int window_width=rect.right-rect.left;							// Calculate The Width (Right Side-Left Side)
		int window_height=rect.bottom-rect.top;		

		float viewport_width=(float) window_width;
		float viewport_height=(float) window_height-(float) ((window_height*1)/4);
		glViewport(0,(window_height*1)/4,viewport_width, viewport_height);
		glScissor(0,(window_height*1)/4,viewport_width, viewport_height);

		glMatrixMode (GL_PROJECTION);										// Select The Projection Matrix
		glLoadIdentity ();													// Reset The Projection Matrix
		gluPerspective(50, (float)viewport_width/(float)viewport_height, 0.1,  2000);

		glClearColor(49.0f/255.0f, 140.0f/255.0f, 231.0f / 255.0f, 1.0f);
		//glClearColor(100.0f/255.0f, 149.0f/255.0f, 237.0f / 255.0f, 1.0f);
		//glClearColor(75.0f/255.0f, 146.0f/255.0f, 219.0f / 255.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
		
		glMatrixMode(GL_MODELVIEW);
		//glLoadIdentity();									// Reset The Current Modelview Matrix
		glLoadMatrixf(glm::value_ptr(glm::inverse(mCameraWorldMatrix)));

		glm::mat4 view_matrix = mCameraViewMatrix;
		
		GLfloat light_ambient[]= { 0.1f, 0.1f, 0.1f, 0.1f };
		GLfloat light_diffuse[]= { 1.0f, 1.0f, 1.0f, 0.0f };
		GLfloat light_specular[]= { 1.0f, 1.0f, 1.0f, 0.0f };

		GLfloat light_position[]= { 0.0f, 0.0f, 0.0f, 1.0f };

		glLightfv (GL_LIGHT1, GL_AMBIENT, light_ambient);
		glLightfv (GL_LIGHT1, GL_DIFFUSE, light_diffuse);
		glLightfv (GL_LIGHT1, GL_SPECULAR, light_specular);
		//glLightfv(GL_LIGHT1, GL_POSITION, light_position);
		glLightfv(GL_LIGHT1, GL_POSITION, glm::value_ptr(mCameraWorldMatrix[3]));

		glEnable (GL_LIGHT1);
		

		for (int i=0; i<mCameraSortedMotionClips.size(); ++i)
			mMotionClips[mCameraSortedMotionClips[i]].Draw(view_matrix, mFocusClip == mCameraSortedMotionClips[i], mFocusClipCylinderLength);

		DrawTestSkeleton();

		{
		
			GLint viewport[4];
			GLdouble modelview[16];
			GLdouble projection[16];
			GLdouble posX, posY, posZ;

			
			glLoadMatrixf(glm::value_ptr(view_matrix));
			glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
			glGetDoublev( GL_PROJECTION_MATRIX, projection );
			glGetIntegerv( GL_VIEWPORT, viewport );
			

			GLfloat winX, winY, winZ;				// Holds Our X, Y and Z Coordinates

			winX = (float)mouse.x;					// Holds The Mouse X Coordinate
			winY = (float)window_height -(float) mouse.y;		
			
			gluUnProject( winX, winY, 0.0f/*mCameraWorldMatrix[3].z*/, modelview, projection, viewport, &posX, &posY, &posZ);

			glm::vec3 unproj = glm::vec3(posX, posY, posZ);
			glm::vec3 dir = unproj - glm::vec3(mCameraWorldMatrix[3]);
			dir = glm::normalize(dir);

			/*
			glEnable(GL_COLOR_MATERIAL);
			glMatrixMode(GL_MODELVIEW);
			glLoadMatrixf(glm::value_ptr(view_matrix * glm::translate(glm::mat4(), glm::vec3(mCameraWorldMatrix[3]) + (dir * 1.0f))));
			glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
			glutSolidSphere(0.02f, 10.0f, 10.0f);
			glDisable(GL_COLOR_MATERIAL);
			*/
		
			{
				float min_t = std::numeric_limits<float>::max();
				mFocusClip = -1;

				

				for (int i=0; i<mMotionClips.size(); ++i)
				{
					float t;
					float cylinder_length;

					if (mMotionClips[i].IntersectRay(glm::vec3(mCameraWorldMatrix[3]), dir, t, cylinder_length)
						&& t < min_t)
					{
						min_t = t;
						mFocusClip = i;
						mFocusClipCylinderLength = cylinder_length;
					}
				}
			}

			//glMatrixMode(GL_MODELVIEW);
			//glLoadMatrixf(glm::value_ptr(view_matrix * glm::translate(glm::mat4(), glm::vec3(mCameraWorldMatrix[3]))));

			//glutWireSphere(1.0f, 10.0f, 10.0f);
		}

		//mMesh.Draw();
		//glColor4f(1.0f, 0.0f, 0.0f, 0.7f);
		//glutWireCylinder(0.5f, 2.0f, 20, 1);

		//DefaultDrawGLScene();

		{
			float viewport_width=(float) window_width;
			float viewport_height=(float) ((window_height*1)/4);
			glViewport(0,0,viewport_width, viewport_height);
			glScissor(0,0,viewport_width, viewport_height);
 			
			glMatrixMode (GL_PROJECTION);										// Select The Projection Matrix
			glLoadIdentity ();													// Reset The Projection Matrix
			gluPerspective(50, (float)viewport_width/(float)viewport_height, 0.1,  2000);

			//glClearColor(49.0f/255.0f, 140.0f/255.0f, 231.0f / 255.0f, 1.0f);
			//glClearColor(100.0f/255.0f, 149.0f/255.0f, 237.0f / 255.0f, 1.0f);
			glClearColor(75.0f/255.0f, 146.0f/255.0f, 219.0f / 255.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
		}

		return true;
	}

};

