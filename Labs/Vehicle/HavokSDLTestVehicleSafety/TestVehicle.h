#ifndef TEST_VEHICLE_H
#define TEST_VEHICLE_H

#include <Physics/Vehicle/hkpVehicleInstance.h>
#include <Physics/Vehicle/Steering/Default/hkpVehicleDefaultSteering.h>
#include <Physics/Collide/Shape/Convex/ConvexVertices/hkpConvexVerticesShape.h>

#include "VehicleApiUtils.h"
#include "VehicleSetup.h"
#include "SDL.h"

#define HAVOC_2D_X 2
#define HAVOC_2D_Y 0

#define HAVOC_3D_X 2
#define HAVOC_3D_Y 0

inline Vector2D to2D(const hkVector4& vect)
{
	return Vector2D(-vect(HAVOC_2D_X), vect(HAVOC_2D_Y));
}

inline Vector2D to2DRight(const hkRotation& rot)
{
	return to2D(rot.getColumn(HAVOC_2D_X));
}

inline Vector2D to2DFwd(const hkRotation& rot)
{
	return to2D(rot.getColumn(HAVOC_2D_Y));
}

inline hkVector4 toHavok(const Vector2D& val, const hkVector4& currVal)
{
	return hkVector4(val.y, currVal.getSimdAt(1), -val.x, currVal.getSimdAt(3));
}

inline hkRotation toHavocFwd(const Vector2D& fwd)
{
	hkRotation ret;
	
	hkVector4 axis(0,1,0,0);
	float angle = acosf(Dot(fwd, Vector2D(0.0f, 1.0f)));

	ret.setAxisAngle(axis, angle);
	
	return ret;
}


class TestVehicle
{
public:

	hkpVehicleInstance* m_vehicle;
	hkpWorld* m_world;

	Vector2D m_steer;

	TestVehicle()
	{
		m_vehicle = NULL;	
	}

	~TestVehicle()
	{
		m_world->lock();
		//
		// Remove the wheel display geometries.
		//
		/*
		for (int i = 0; i < m_vehicle->m_data->m_numWheels; i++)
		{
			m_env->m_displayHandler->removeGeometry(m_displayWheelId[i], m_tag, 0);
		}
		*/


		// Remove the vehicle phantom from the world.	
		m_world->removePhantom( (hkpPhantom*)(static_cast< hkpVehicleRaycastWheelCollide*>(m_vehicle->m_wheelCollide)->m_phantom) );

		m_vehicle->removeReference();
		m_world->unlock();
	}
	


	class Steering : public hkpVehicleSteering 
	{
	public:

		hkpVehicleSteering* m_pOrig;
		 

		Steering( hkpVehicleSteering * pOrig)
		{
			m_pOrig = pOrig;
			
		}

		virtual void calcSteering (const hkReal deltaTime, const hkpVehicleInstance *vehicle, const hkpVehicleDriverInput::FilteredDriverInputOutput &filteredInfoOutput, SteeringAnglesOutput &steeringOutput)
		{
			m_pOrig->calcSteering (deltaTime, vehicle, filteredInfoOutput, steeringOutput);

			if (steeringOutput.m_mainSteeringAngle != 0.0f)
			{
				printf("wheel-steer: %f, %f, %f\n", steeringOutput.m_mainSteeringAngle, steeringOutput.m_wheelsSteeringAngle[0], steeringOutput.m_wheelsSteeringAngle[1]);
				int x = 0;
			}
		}
	};

	class TestAngleSteering : public hkpVehicleSteering 
	{
	public:

		hkpVehicleDefaultSteering * m_pOrig;
		hkpVehicleDefaultSuspension* m_pSusp;
		hkVector4 mCenterPos;
		float m_OnSpotAngle[16];

		TestAngleSteering(hkpVehicleInstance* pVehicle, hkpVehicleDefaultSteering* pOrig)
		{
			m_pOrig = pOrig;
			m_pOrig->m_maxSteeringAngle = 2.0f * MATH_PIf;
			m_pOrig->m_maxSpeedFullSteeringAngle = 2.0f * MATH_PIf;

			m_pSusp = (hkpVehicleDefaultSuspension*) pVehicle->m_suspension;

			Init();
		}

		void Init()
		{
			hkVector4 center_pos_3D;
			center_pos_3D.setZero4();

			float wheel_factor = 1.0f / ((float) m_pSusp->m_wheelParams.getSize());
			for (int i = 0; i < m_pSusp->m_wheelParams.getSize(); ++i)
			{
				hkVector4 pos = m_pSusp->m_wheelParams[i].m_hardpointChassisSpace;
				pos.mul4(wheel_factor);
				center_pos_3D.add4(pos);
			}

			mCenterPos = center_pos_3D;
			Vector2D center_pos = to2D(mCenterPos);
			
			for (int i = 0; i < m_pSusp->m_wheelParams.getSize(); ++i)
			{
				Vector2D pos = to2D(m_pSusp->m_wheelParams[i].m_hardpointChassisSpace);
				Vector2D normal = pos - center_pos;
								
				Vector2D dir = rotate90(normal);
				Vector2D ref_dir(0.0f, 1.0f);
				
				m_OnSpotAngle[i] = SignedAngle(dir, ref_dir);
			}
		}

		virtual void calcSteering (const hkReal deltaTime, const hkpVehicleInstance *vehicle, const hkpVehicleDriverInput::FilteredDriverInputOutput &filteredInfoOutput, SteeringAnglesOutput &steeringOutput)
		{
			m_pOrig->calcSteering (deltaTime, vehicle, filteredInfoOutput, steeringOutput);

			//for (int i = 0; i < m_pSusp->m_wheelParams.getSize(); ++i)
			//{
			//	steeringOutput.m_wheelsSteeringAngle[i] = m_OnSpotAngle[i];
			//}

			//static int cnt = 0;
			//steeringOutput.m_wheelsSteeringAngle[1] = (float) cnt / 50.0f;
			//++cnt;

			//if (steeringOutput.m_mainSteeringAngle != 0.0f)
			//{
			//	printf("wheel-steer: %f, %f, %f\n", steeringOutput.m_mainSteeringAngle, steeringOutput.m_wheelsSteeringAngle[0], steeringOutput.m_wheelsSteeringAngle[1]);
			//	int x = 0;
			//}
		}
	};

	class DifferentialSteeringSteering : public hkpVehicleSteering 
	{
	public:

		hkpVehicleSteering* m_pOrig;

		DifferentialSteeringSteering(hkpVehicleSteering* pOrig)
		{
			m_pOrig = pOrig;
		}

		virtual void calcSteering (const hkReal deltaTime, const hkpVehicleInstance *vehicle, const hkpVehicleDriverInput::FilteredDriverInputOutput &filteredInfoOutput, SteeringAnglesOutput &steeringOutput)
		{
			m_pOrig->calcSteering (deltaTime, vehicle, filteredInfoOutput, steeringOutput);

			//if (steeringOutput.m_mainSteeringAngle != 0.0f)
			{
				steeringOutput.m_wheelsSteeringAngle[0] = 0.0f;
				steeringOutput.m_wheelsSteeringAngle[1] = 0.0f;
				steeringOutput.m_wheelsSteeringAngle[2] = 0.0f;
				steeringOutput.m_wheelsSteeringAngle[3] = 0.0f;
				int x = 0;
			}
		}
	};

	

	class DifferentialSteeringTransmission : public hkpVehicleTransmission  
	{
	public:

		hkpVehicleDriverInputAnalogStatus* m_pDevStatus;
		hkpVehicleTransmission* m_pOrig;

		DifferentialSteeringTransmission(hkpVehicleTransmission* pOrig, hkpVehicleDriverInputAnalogStatus* pDevStatus)
		{
			m_pOrig = pOrig;
			m_pDevStatus = pDevStatus;
		}
		
		virtual void  calcTransmission (const hkReal deltaTime, const hkpVehicleInstance *vehicle, TransmissionOutput &transmissionOut)
		{
			 m_pOrig->calcTransmission (deltaTime, vehicle, transmissionOut);

			 if (m_pDevStatus->m_positionX != 0.0f)
			 {
				 //transmissionOut.m_numWheelsTramsmittedTorque = 4;

				 //transmissionOut.m_wheelsTransmittedTorque[0] = -transmissionOut.m_wheelsTransmittedTorque[0];
				 //transmissionOut.m_wheelsTransmittedTorque[1] = -transmissionOut.m_wheelsTransmittedTorque[1];
				 //transmissionOut.m_wheelsTransmittedTorque[2] = -transmissionOut.m_wheelsTransmittedTorque[2];
				 //transmissionOut.m_wheelsTransmittedTorque[3] = -transmissionOut.m_wheelsTransmittedTorque[3];

				 //transmissionOut.m_wheelsTransmittedTorque[2] = transmissionOut.m_wheelsTransmittedTorque[0];
				 //transmissionOut.m_wheelsTransmittedTorque[1] = -transmissionOut.m_wheelsTransmittedTorque[0];
				 //transmissionOut.m_wheelsTransmittedTorque[3] = transmissionOut.m_wheelsTransmittedTorque[1];
			 }
		}
	};

	

	void Create(hkpWorld* world)
	{

		m_world = world;
		hkpConvexVerticesShape* chassisShape = VehicleApiUtils::createCarChassisShape(); 
		hkpRigidBody* chassisRigidBody;
		{
			hkpRigidBodyCinfo chassisInfo;

			// NB: The inertia value is reset by the vehicle SDK.  However we give it a
			// reasonable value so that the hkpRigidBody does not assert on construction. See
			// VehicleSetup for the yaw, pitch and roll values used by hkVehicle.
			chassisInfo.m_mass = 750.0f;	
			chassisInfo.m_shape = chassisShape;
			chassisInfo.m_friction = 0.4f;

			// The chassis MUST have m_motionType hkpMotion::MOTION_BOX_INERTIA to correctly simulate
			// vehicle roll, pitch and yaw.
			chassisInfo.m_motionType = hkpMotion::MOTION_BOX_INERTIA;
			chassisInfo.m_position.set(0.0f, 1.0f, 0.0f);
			hkpInertiaTensorComputer::setShapeVolumeMassProperties(chassisInfo.m_shape,
				chassisInfo.m_mass,
				chassisInfo);

			chassisRigidBody = new hkpRigidBody(chassisInfo);

			// No longer need reference to shape as the hkpRigidBody holds one.
			chassisShape->removeReference();

			world->addEntity(chassisRigidBody);
		}

		createVehicle(world, chassisRigidBody );
		chassisRigidBody->removeReference();
	}

	void createVehicle(hkpWorld* world, hkpRigidBody* chassis)
	{

		// Create the basic vehicle.
		m_vehicle = new hkpVehicleInstance( chassis );
		VehicleSetup setup;
		setup.buildVehicle(world, *m_vehicle );

		//m_vehicle->m_steering			= new TestAngleSteering(m_vehicle, (hkpVehicleDefaultSteering*) m_vehicle->m_steering);
		//m_vehicle->m_steering			= new DifferentialSteeringSteering(m_vehicle->m_steering);
		//m_vehicle->m_transmission		= new DifferentialSteeringTransmission(m_vehicle->m_transmission, (hkpVehicleDriverInputAnalogStatus*)m_vehicle->m_deviceStatus);

		///[integrationWithSDK]
		/// Actions are the interface between user controllable behavior of the physical simulation and the Havok core. 
		/// You can easily integrate the Vehicle Kit with the Havok physical simulation using the hkpVehicleInstance action, 
		/// which establishes the connection between the two SDKs. 
		///
		///	To simulate a vehicle, you first need to create a hkpVehicleInstance instance in your game. 
		/// You then add it to the actions of your core physical simulation, just like any other user action:
		///<

		world->addAction(m_vehicle);
		///>
		/// Once you have added the action to the simulation, no extra work is required to simulate the vehicle. 
		/// On each call to step the core physical simulation, the vehicle action will be updated automatically.
	}

	void HK_CALL steer(float steer, float throttle, bool handbrake)
	{
		m_world->lock();

		hkpVehicleDriverInputAnalogStatus*	deviceStatus = (hkpVehicleDriverInputAnalogStatus*)m_vehicle->m_deviceStatus;
		deviceStatus->m_positionX = -steer;
		deviceStatus->m_positionY = -throttle;
		deviceStatus->m_handbrakeButtonPressed = handbrake;

		{
			m_vehicle->getChassis()->activate();
		}

		m_world->unlock();
	}

	void Teleport(const Vector2D& pos, const Vector2D* pOrientFwd)
	{
		hkVector4 curr_pos = m_vehicle->getChassis()->getPosition();

		if (pOrientFwd)
		{
			hkRotation rot = toHavocFwd(*pOrientFwd);
			m_vehicle->getChassis()->setPositionAndRotation(toHavok(pos, curr_pos), hkQuaternion(rot));
		}
		else
		{
			m_vehicle->getChassis()->setPosition(toHavok(pos, curr_pos));
		}
	}

	Vector2D GetPos()
	{
		hkpRigidBody* chassisRigidBody = m_vehicle->getChassis();
		const hkTransform& transform = chassisRigidBody->getTransform();
		
		Vector2D center = to2D(transform.getTranslation());

		return center;
	}


	Vector2D GetDir()
	{
		hkpRigidBody* chassisRigidBody = m_vehicle->getChassis();
		const hkTransform& transform = chassisRigidBody->getTransform();
		
		hkRotation rot = transform.getRotation();

		return to2DFwd(rot);
	}

	void Draw(Renderer& renderer, float t, float dt)
	{
		if (m_vehicle)
		{
			hkpRigidBody* chassisRigidBody = m_vehicle->getChassis();
			const hkTransform& transform = chassisRigidBody->getTransform();
			hkpRigidBodyCinfo info;
			chassisRigidBody->getCinfo(info); 

			hkAabb aabb;
			info.m_shape->getAabb(hkTransform::getIdentity(), 0.0f, aabb);

			Vector2D quad[4];
			Vector2D center = to2D(transform.getTranslation());
			hkRotation rot = transform.getRotation();
			Vector2D right = to2DRight(rot);
			Vector2D fwd = to2DFwd(rot);

			Vector2D ext_min = to2D(aabb.m_min);
			Vector2D ext_max = to2D(aabb.m_max);

			quad[0] = center + (right * ext_min.x) + (fwd * ext_min.y);
			quad[1] = center + (right * ext_max.x) + (fwd * ext_min.y);
			quad[2] = center + (right * ext_max.x) + (fwd * ext_max.y);
			quad[3] = center + (right * ext_min.x) + (fwd * ext_max.y);

			renderer.DrawQuad(renderer.WorldToScreen(quad[0]), 
							  renderer.WorldToScreen(quad[1]), 
							  renderer.WorldToScreen(quad[2]), 
							  renderer.WorldToScreen(quad[3]), Color::kWhite);

			renderer.DrawQuad(renderer.WorldToScreen(quad[0] + right * 0.25f * (ext_max.x - ext_min.x) + fwd * 0.25f * (ext_max.y - ext_min.y)), 
								renderer.WorldToScreen(quad[1] + right * -0.25f * (ext_max.x - ext_min.x) + fwd * 0.25f * (ext_max.y - ext_min.y)), 
								renderer.WorldToScreen((quad[2]+quad[3] + fwd * -0.2f * (ext_max.y - ext_min.y)) * 0.5f), 
								renderer.WorldToScreen((quad[2]+quad[3]+ fwd * -0.2f * (ext_max.y - ext_min.y)) * 0.5f), Color::kWhite);

			for (int i = 0; i < m_vehicle->m_wheelsInfo.getSize(); ++i)
			{
				const hkpVehicleInstance::WheelInfo& info = m_vehicle->m_wheelsInfo[i];
				Vector2D pos = to2D(info.m_hardPointWs);
				Vector2D spin_axis = to2D(info.m_spinAxisWs);
				Vector2D dir = rotate90(spin_axis) * 1.0f;
				//Vector2D dir = spin_axis;
				float radius = m_vehicle->m_data->m_wheelParams[i].m_radius;
				
				renderer.DrawLine(renderer.WorldToScreen(pos - (dir * radius)), 
								  renderer.WorldToScreen(pos + (dir * radius)),
								  Color::kWhite);
			}
		}
	}
};

class VehicleController
{
public:

	VehicleController() : mpVehicle(NULL) {}

	virtual void SetVehicle(TestVehicle* pVehicle) { mpVehicle = pVehicle; }
	virtual void Update(float t, float dt) {}
	virtual void Draw(Renderer& renderer, float t) {}
	virtual void HandleEvent(const SDL_Event& evt) {}

	TestVehicle* mpVehicle;
};

class VehicleController_Keyb : public VehicleController
{
public:

	Vector2D m_steer;

	virtual void HandleEvent(const SDL_Event& evt)
	{
		if (evt.type == SDL_KEYDOWN)
		{
			if (evt.key.keysym.sym == SDLK_UP)
			{
				m_steer.y = 1.0f;
			}
			else if (evt.key.keysym.sym == SDLK_DOWN)
			{
				m_steer.y = -1.0f;
			}
			


			if (evt.key.keysym.sym == SDLK_RIGHT)
			{
				m_steer.x = -1.0f;
			}
			else if (evt.key.keysym.sym == SDLK_LEFT)
			{
				m_steer.x = 1.0f;
			}
			

		}

		if (evt.type == SDL_KEYUP)
		{
			if (evt.key.keysym.sym == SDLK_UP && m_steer.y == 1.0f)
			{
				m_steer.y = 0.0f;
			}
			else if (evt.key.keysym.sym == SDLK_DOWN && m_steer.y == -1.0f)
			{
				m_steer.y = 0.0f;
			}


			if (evt.key.keysym.sym == SDLK_RIGHT && m_steer.x == -1.0f)
			{
				m_steer.x = 0.0f;
			}
			else if (evt.key.keysym.sym == SDLK_LEFT && m_steer.x == 1.0f)
			{
				m_steer.x = 0.0f;
			}
		}
	}

	virtual void Update(float t, float dt)
	{
		mpVehicle->steer(m_steer.x, m_steer.y, false);
	}
};

class SimpleVehicleSpeedThrottleController
{
public:
	
	float mSpeed;
	float mThrottle;
	float mLastThrottle;
	float mLastSpeed;
	float mThrottleProportional;
	bool mHasReachedSpeed;

	SimpleVehicleSpeedThrottleController::SimpleVehicleSpeedThrottleController()
	{
		mSpeed = 0.0f;
		mLastSpeed = -1.0f;
		mThrottleProportional = 1.0f;
	}


	void SetSpeed(float speed)
	{
		mSpeed = speed;
		mLastSpeed = -1.0f;
		mThrottleProportional = 1.0f;
		mHasReachedSpeed = false;
	}

	float GetThrottle(TestVehicle& vehicle)
	{
		float curr_speed = vehicle.m_vehicle->calcKMPH() / 3.6f;
		mThrottle = mThrottleProportional * ((mSpeed - curr_speed));

		if (mThrottle < 0.0f)
			mThrottle = 0.0f;

		if (mThrottle > 1.0f)
			mThrottle = 1.0f;

		if (curr_speed > mSpeed)
		{
			mThrottleProportional -= 0.05f;
			mHasReachedSpeed = true;
		}
		if (curr_speed < mSpeed)
		{
			mThrottleProportional += 0.05f;
		}

		mLastSpeed = curr_speed;

		return mThrottle;
	}
};

class VehicleController_TurnRadiusTest : public VehicleController
{
public:

	enum State
	{
		INVALID, SETTING_SPEED, TESTING_DELAY, TESTING
	};

	State mState;
	SimpleVehicleSpeedThrottleController mThrottle;
	float mSteer;

	int mSampleCount;
	bool mHasLastSample;
	Vector2D mLastPos;
	Vector2D mLastDir;

	Vector2D mPreLastPos;
	Vector2D mPreLastDir;

	Vector2D mPos;
	Vector2D mDir;

	Vector2D mLastRadiusCenter;

	float mTestEndDot;

	std::vector<Vector2D> mHistory;

	float mRadius;
	float mError;
	
	Vector2D mCenterBoxMin;
	Vector2D mCenterBoxMax;

	float mStartTestTime;

	VehicleController_TurnRadiusTest() 
	{
		mState = INVALID;
		mTestEndDot = -0.5f;
	}

	void Init(TestVehicle* pVehicle, float speed, float steer)
	{
		mThrottle.SetSpeed(speed);
		mSteer = steer;

		VehicleController::SetVehicle(pVehicle);
		mThrottle.SetSpeed(mThrottle.mSpeed);
		
		mState = INVALID;
		mHistory.clear();
	}

	bool IsFinished(float t)
	{
		Vector2D fwdDir(0.0f, 1.0f);

		return (mState == TESTING) && (mSampleCount > 10) && (Dot(fwdDir, mLastDir) <= mTestEndDot);
	}

	float GetCenterBoxError()
	{
		return Distance(mCenterBoxMax, mCenterBoxMin);
	}

	virtual void Update(float t, float dt)
	{
		switch (mState)
		{
		case INVALID:
			{
				mSampleCount = 0;
				float speed = mpVehicle->m_vehicle->calcKMPH() / 3.6f;
				mpVehicle->steer(0.0f, 0.0f, true);

				mHistory.clear();

				// why oh why does the speed not get to exact zero.
				if (speed <= 0.01f)
				{
					Vector2D fwd(0.0f, 1.0f);
					mpVehicle->Teleport(Vector2D(), &fwd);
					mState = SETTING_SPEED;
					mHistory.clear();
				}
			}
			break;

		case SETTING_SPEED:
			{
				mpVehicle->steer(0.0f, mThrottle.GetThrottle(*mpVehicle), false);
				float speed = mpVehicle->m_vehicle->calcKMPH() / 3.6f;
				
				if (mThrottle.mHasReachedSpeed)
				{
					mState = TESTING_DELAY;
					mStartTestTime = t;
				}
			}
			break;

		case TESTING_DELAY:
		{
			mpVehicle->steer(mSteer, mThrottle.GetThrottle(*mpVehicle), false);

			Vector2D fwdDir(0.0f, 1.0f);

			if (Dot(fwdDir, mpVehicle->GetDir()) < 0.9f)
			{
				mRadius = 0.0f;
				mError = 0.0f;
				mState = TESTING;
			}
		}
		break;

		case TESTING:
			{
				// we should gradually increase steer...
				mpVehicle->steer(mSteer, mThrottle.GetThrottle(*mpVehicle), false);

				{
					Vector2D pos = mpVehicle->GetPos();
					Vector2D dir = mpVehicle->GetDir();

					if (mSampleCount > 0)
					{
						Vector2D radius_line_dir = rotate90(dir);
						Vector2D prev_radius_line_dir = rotate90(mLastDir);

						float r, s;

						if (IntersectLines(pos, pos+radius_line_dir, mLastPos, mLastPos+prev_radius_line_dir, r, s))
						{
							Vector2D radius_center = pos + (radius_line_dir*r);
							mLastRadiusCenter = radius_center;

							float radius = Distance(pos, radius_center);
							float prev_radius = Distance(mLastPos, radius_center);
							float avg_radius = 0.5f*(radius + prev_radius);
							float err = fabs(radius - prev_radius);

							if (mSampleCount<=1)
							{
								mRadius = avg_radius;
								mError = err;

								mCenterBoxMin = radius_center;
								mCenterBoxMax = radius_center;
							}
							else
							{
								mRadius = mRadius + ((avg_radius - mRadius) / (mSampleCount+1));
								mError = mError + ((err - mError) / (mSampleCount+1));

								if (radius_center.x < mCenterBoxMin.x)
									mCenterBoxMin.x = radius_center.x;

								if (radius_center.y < mCenterBoxMin.y)
									mCenterBoxMin.y = radius_center.y;

								if (radius_center.x > mCenterBoxMax.x)
									mCenterBoxMax.x = radius_center.x;

								if (radius_center.y < mCenterBoxMax.y)
									mCenterBoxMax.y = radius_center.y;
							}

							++mSampleCount;
						}

						mPreLastPos = mLastPos;
						mPreLastDir = mLastDir;

						mLastPos = pos;
						mLastDir = dir;

						/*
						if (mSampleCount % 60 == 0)
						{
							printf("%f,%f\n", mRadius, mError);
						}
						*/
					}
					else
					{
						mRadius = 0.0f;
						mError = 0.0f;
						mLastPos = pos;
						mLastDir = dir;
						++mSampleCount;
					}
				}

				mHistory.push_back(mpVehicle->GetPos());
			}
			break;
		}
	}

	virtual void Draw(Renderer& renderer, float t) 
	{
		for (int i=0; i+1<mHistory.size(); ++i)
		{
			renderer.DrawLine(renderer.WorldToScreen(mHistory[i]), renderer.WorldToScreen(mHistory[i+1]), Color::kWhite, 0.25f, 0.5f);
		}

		if (mSampleCount >= 2)
		{
			renderer.DrawCircle(renderer.WorldToScreen(mLastRadiusCenter), mRadius*0.1f, Color::kWhite, 0.5f, true);
		}

		/*
		Vector2D radius_line_dir = rotate90(mLastDir);
		Vector2D prev_radius_line_dir = rotate90(mPreLastDir);

		float r, s;

		if (IntersectLines(mLastPos, mLastPos+radius_line_dir, mPreLastPos, mPreLastPos+prev_radius_line_dir, r, s))
		{
			Vector2D radius_center = mLastPos + (radius_line_dir*r);

			renderer.DrawLine(renderer.WorldToScreen(mPreLastPos), renderer.WorldToScreen(radius_center), Color::kBlue);
			renderer.DrawLine(renderer.WorldToScreen(mLastPos), renderer.WorldToScreen(radius_center), Color::kBlue);
		}
		*/
	}
};


class VehicleController_HardBrakingDistanceTest : public VehicleController
{
public:

	enum State
	{
		INVALID, SETTING_SPEED, TESTING, STOPPED
	};

	State mState;
	SimpleVehicleSpeedThrottleController mThrottle;

	Vector2D mStartBrakePos;
	float mBrakeTime;
	float mBrakeDist;

	
	std::vector<Vector2D> mHistory;
	float mStartTestTime;

	VehicleController_HardBrakingDistanceTest() 
	{
		mState = INVALID;
	}

	void Init(TestVehicle* pVehicle, float speed)
	{
		mThrottle.SetSpeed(speed);
		
		VehicleController::SetVehicle(pVehicle);
		mThrottle.SetSpeed(mThrottle.mSpeed);
		
		mState = INVALID;
		mHistory.clear();
	}

	bool IsFinished(float t)
	{
		return (mState == STOPPED);
	}

	virtual void Update(float t, float dt)
	{
		switch (mState)
		{
		case INVALID:
			{
				float speed = mpVehicle->m_vehicle->calcKMPH() / 3.6f;
				mpVehicle->steer(0.0f, 0.0f, true);

				mHistory.clear();

				// why oh why does the speed not get to exact zero.
				if (speed <= 0.01f)
				{
					Vector2D fwd(0.0f, 1.0f);
					mpVehicle->Teleport(Vector2D(), &fwd);
					mState = SETTING_SPEED;
					mHistory.clear();
				}
			}
			break;

		case SETTING_SPEED:
			{
				mpVehicle->steer(0.0f, mThrottle.GetThrottle(*mpVehicle), false);
				float speed = mpVehicle->m_vehicle->calcKMPH() / 3.6f;
				
				if (mThrottle.mHasReachedSpeed)
				{
					mState = TESTING;
					mStartTestTime = t;
					mStartBrakePos = mpVehicle->GetPos();
				}
			}
			break;

		case TESTING:
			{
				mpVehicle->steer(0.0f, 0.0f, true);
				float speed = mpVehicle->m_vehicle->calcKMPH() / 3.6f;

				// havok ....
				if (speed < 0.01f)
				{
					mBrakeTime = t - mStartTestTime;
					mBrakeDist = Distance(mStartBrakePos, mpVehicle->GetPos());
					mState = STOPPED;
				}

				mHistory.push_back(mpVehicle->GetPos());
			}
			break;
		}
	}

	virtual void Draw(Renderer& renderer, float t) 
	{
		for (int i=0; i+1<mHistory.size(); ++i)
		{
			renderer.DrawLine(renderer.WorldToScreen(mHistory[i]), renderer.WorldToScreen(mHistory[i+1]), Color::kWhite, 0.25f, 0.5f);
		}
	}
};


class VehicleController_TurnRadiusLearn : public VehicleController
{
public:

	float mFromSpeed;
	float mToSpeed;
	int mSpeedCount;
	int mCurrSpeed;
	float mSteer;

	VehicleController_TurnRadiusTest mTester;

	VehicleController_TurnRadiusLearn()
	{
		mSpeedCount = 0;
		mCurrSpeed = -1;
	}

	void Init(TestVehicle* pVehicle, float fromSpeed, float toSpeed, int speedCount, float steer)
	{
		mCurrSpeed = -1;
		mFromSpeed = fromSpeed;
		mToSpeed = toSpeed;
		mSpeedCount = speedCount;
		mSteer = steer;
	}

	virtual void Update(float t, float dt) 
	{
		if (mCurrSpeed < mSpeedCount)
		{
			if (mCurrSpeed == -1 || mTester.IsFinished(t))
			{
				if (mCurrSpeed >= 0)
				{
					printf("Turn Radius for speed: %f is %f, instability: %f\n", mTester.mThrottle.mSpeed, mTester.mRadius, mTester.GetCenterBoxError());
				}

				if (mCurrSpeed < mSpeedCount)
				{
					++mCurrSpeed;
					mTester.Init(mpVehicle, mFromSpeed + ((mToSpeed-mFromSpeed)*((float) mCurrSpeed/(float) mSpeedCount)), mSteer);
				}
			}
			
			if (mCurrSpeed >= 0 && mCurrSpeed<mSpeedCount)
			{
				mTester.Update(t, dt);
			}
		}
	}

	virtual void Draw(Renderer& renderer, float t) 
	{
		mTester.Draw(renderer, t);
	}
};


class VehicleController_HardBrakingDistanceLearn : public VehicleController
{
public:

	float mFromSpeed;
	float mToSpeed;
	int mSpeedCount;
	int mCurrSpeed;

	VehicleController_HardBrakingDistanceTest mTester;

	VehicleController_HardBrakingDistanceLearn()
	{
		mSpeedCount = 0;
		mCurrSpeed = -1;
	}

	void Init(TestVehicle* pVehicle, float fromSpeed, float toSpeed, int speedCount)
	{
		mCurrSpeed = -1;
		mFromSpeed = fromSpeed;
		mToSpeed = toSpeed;
		mSpeedCount = speedCount;
	}

	virtual void Update(float t, float dt) 
	{
		if (mCurrSpeed < mSpeedCount)
		{
			if (mCurrSpeed == -1 || mTester.IsFinished(t))
			{
				if (mCurrSpeed >= 0)
				{
					printf("Braking for speed: %f is %f m. %f sec.\n", mTester.mThrottle.mSpeed, mTester.mBrakeDist, mTester.mBrakeTime);
				}

				if (mCurrSpeed < mSpeedCount)
				{
					++mCurrSpeed;
					mTester.Init(mpVehicle, mFromSpeed + ((mToSpeed-mFromSpeed)*((float) mCurrSpeed/(float) mSpeedCount)));
				}
			}
			
			if (mCurrSpeed >= 0 && mCurrSpeed<mSpeedCount)
			{
				mTester.Update(t, dt);
			}
		}
	}

	virtual void Draw(Renderer& renderer, float t) 
	{
		mTester.Draw(renderer, t);
	}
};

class VehicleController_StableSteerLearn : public VehicleController
{
public:

	float mSpeed;
	float mMinSteer;
	float mMaxSteer;
	float mSteer;
	float mMinStability;
	float mGranularity;

	bool mIsTesting;

	VehicleController_TurnRadiusTest mTester;

	VehicleController_StableSteerLearn()
	{
		mIsTesting = false;
	}

	void Init(TestVehicle* pVehicle, float speed, float granularity, float minStability)
	{
		VehicleController::SetVehicle(pVehicle);
		mSpeed = speed;
		mGranularity = granularity;
		mMinStability = minStability;
		mMinSteer = 0.0f;
		mMaxSteer = 1.0f;
		mSteer = 0.5f * (mMinSteer+mMaxSteer);
		mIsTesting = true;
		mTester.Init(mpVehicle, mSpeed, mSteer);
		printf("testing stable-steer speed: %f, steer: %f\n", mSpeed, mSteer);
	}

	bool IsFinished(float t)
	{
		return (!mIsTesting && mMaxSteer-mMinSteer < mGranularity);
	}

	virtual void Update(float t, float dt) 
	{
		if (mIsTesting)
		{
			mTester.Update(t, dt);

			if (mTester.IsFinished(t))
			{
				mIsTesting = false;
			}
		}

		if (!mIsTesting)
		{
			if (mMaxSteer-mMinSteer > mGranularity)
			{
				printf("instability %f, allowed: %f\n", mTester.GetCenterBoxError(), 0.15f * mTester.mRadius);
				bool was_good_test = mTester.GetCenterBoxError() < 0.15f * mTester.mRadius;

				if (was_good_test)
				{
					mMinSteer = mSteer;
				}
				else
				{
					mMaxSteer = mSteer;
				}

				mSteer = 0.5f * (mMinSteer+mMaxSteer);
				mIsTesting = true;
				printf("testing stable-steer speed: %f, steer: %f\n", mSpeed, mSteer);
				mTester.Init(mpVehicle, mSpeed, mSteer);
			}
			else
			{
				mpVehicle->steer(0.0f, 0.0f, true);
			}
		}
	}

	virtual void Draw(Renderer& renderer, float t) 
	{
		if (mIsTesting)
			mTester.Draw(renderer, t);
	}
};


class VehicleController_StableTurnRadiusLearn : public VehicleController
{
public:

	float mFromSpeed;
	float mToSpeed;
	int mSpeedCount;
	int mCurrSpeed;
	float mSteer;
	float mStableSteer;
	float mMinStability;
	float mStabilityGranularity;

	VehicleController_TurnRadiusTest mTester;
	VehicleController_StableSteerLearn mStableTester;

	VehicleController_StableTurnRadiusLearn()
	{
		mSpeedCount = 0;
		mCurrSpeed = -1;
	}

	void Init(TestVehicle* pVehicle, float fromSpeed, float toSpeed, int speedCount, float steer, float minStability, float stabilityGranularity)
	{
		mCurrSpeed = -1;
		mFromSpeed = fromSpeed;
		mToSpeed = toSpeed;
		mSpeedCount = speedCount;
		mSteer = steer;
		mMinStability= minStability;
		mStabilityGranularity = stabilityGranularity;
	}

	virtual void Update(float t, float dt) 
	{
		if (mCurrSpeed<mSpeedCount)
		{
			if (mCurrSpeed == -1 || mTester.IsFinished(t) )
			{
				if (mCurrSpeed >= 0)
				{
					printf("Turn Radius for speed: %f is %f, error: %f\n", mTester.mThrottle.mSpeed, mTester.mRadius, mTester.GetCenterBoxError());
				}

				++mCurrSpeed;
				if (mCurrSpeed < mSpeedCount)
				{	
					mTester.Init(mpVehicle, mFromSpeed + ((mToSpeed-mFromSpeed)*((float) mCurrSpeed / (float) mSpeedCount)), mSteer);
					mStableTester.Init(mpVehicle, mTester.mThrottle.mSpeed, mStabilityGranularity, mMinStability);
					mStableSteer = -1.0f;
				}
			}
			
			if (mCurrSpeed >= 0 && mCurrSpeed<mSpeedCount)
			{
				if (mStableSteer == -1.0f)
				{
					if (mStableTester.IsFinished(t))
					{
						mStableSteer = mStableTester.mMinSteer;
						printf("stable-steer for speed: %f is %f\n", mTester.mThrottle.mSpeed, mStableSteer);
						mTester.Init(mpVehicle, mTester.mThrottle.mSpeed, mStableSteer);
					}
					else
					{
						mStableTester.Update(t,dt);
					}
				}
				else
				{
					mTester.Update(t, dt);
				}
			}
		}
		else
		{
			mpVehicle->steer(0.0f, 0.0f, true);
		}
	}

	virtual void Draw(Renderer& renderer, float t) 
	{
		if (mStableSteer == -1.0f)
			mStableTester.Draw(renderer, t);
		else
			mTester.Draw(renderer, t);
	}
};

#endif