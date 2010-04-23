#include "NeHeGL.h"
#include "mesh.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/type_ptr.hpp"
#include "arb_multisample.h"
#define FREEGLUT_STATIC
#include <gl\freeglut.h>			// Header File For The GLu32 Library
#include "Random.h"
#include <vector>

class App
{
public:

	struct MotionClip
	{
		glm::mat4 mWorldMatrix;
		float mLength;
		glm::vec4 mColor;

		MotionClip()
		{
			mLength = 1.0f;
		}

		void Draw(const glm::mat4& inViewMatrix)
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
			glutSolidCylinder(0.25f, mLength, 20, 1);
			//glutWireCylinder(0.5f, 2.0f, 20, 1);
			//glutSolidCube(2.0f);
		}
	};

	typedef std::vector<MotionClip> MotionClips;

	Mesh mMesh;
	GL_Window*	mWindow;
	
	glm::mat4 mCameraWorldMatrix;
	MotionClips mMotionClips;


	void End()
	{
	}

	bool Load(GL_Window* pWindow)
	{
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

		Random random;

		mMotionClips.resize(120);

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

		
	}

	bool Draw()
	{
		glClearColor(49.0f/255.0f, 140.0f/255.0f, 231.0f / 255.0f, 1.0f);
		//glClearColor(100.0f/255.0f, 149.0f/255.0f, 237.0f / 255.0f, 1.0f);
		//glClearColor(75.0f/255.0f, 146.0f/255.0f, 219.0f / 255.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
		
		glMatrixMode(GL_MODELVIEW);
		//glLoadIdentity();									// Reset The Current Modelview Matrix
		glLoadMatrixf(glm::value_ptr(glm::inverse(mCameraWorldMatrix)));

		glm::mat4 view_matrix = glm::inverse(mCameraWorldMatrix);
		
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
		

		for (int i=0; i<mMotionClips.size(); ++i)
			mMotionClips[i].Draw(view_matrix);


		{
		
			GLint viewport[4];
			GLdouble modelview[16];
			GLdouble projection[16];
			GLdouble posX, posY, posZ;

			
			glLoadMatrixf(glm::value_ptr(view_matrix));
			glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
			glGetDoublev( GL_PROJECTION_MATRIX, projection );
			glGetIntegerv( GL_VIEWPORT, viewport );

			
			POINT mouse;						// Stores The X And Y Coords For The Current Mouse Position
			GetCursorPos(&mouse);					// Gets The Current Cursor Coordinates (Mouse Coordinates)
			ScreenToClient(mWindow->hWnd, &mouse);

			GLfloat winX, winY, winZ;				// Holds Our X, Y and Z Coordinates

			winX = (float)mouse.x;					// Holds The Mouse X Coordinate
			winY = (float)mouse.y;		
			winY = (float)viewport[3] - (float)winY;

			gluUnProject( winX, winY, 0.0f/*mCameraWorldMatrix[3].z*/, modelview, projection, viewport, &posX, &posY, &posZ);

			glm::vec3 unproj = glm::vec3(posX, posY, posZ);
			glm::vec3 dir = unproj - glm::vec3(mCameraWorldMatrix[3]);
			dir = glm::normalize(dir);

			glEnable(GL_COLOR_MATERIAL);

			glMatrixMode(GL_MODELVIEW);
			glLoadMatrixf(glm::value_ptr(view_matrix * glm::translate(glm::mat4(), glm::vec3(mCameraWorldMatrix[3]) + (dir * 1.0f))));

			

			glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
			glutSolidSphere(0.02f, 10.0f, 10.0f);

			glDisable(GL_COLOR_MATERIAL);

			//glMatrixMode(GL_MODELVIEW);
			//glLoadMatrixf(glm::value_ptr(view_matrix * glm::translate(glm::mat4(), glm::vec3(mCameraWorldMatrix[3]))));

			//glutWireSphere(1.0f, 10.0f, 10.0f);
		}

		//mMesh.Draw();
		//glColor4f(1.0f, 0.0f, 0.0f, 0.7f);
		//glutWireCylinder(0.5f, 2.0f, 20, 1);

		//DefaultDrawGLScene();

		return true;
	}

};

