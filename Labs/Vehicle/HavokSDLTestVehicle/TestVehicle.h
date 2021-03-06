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

inline Vector2D to2D(const hkVector4& vect)
{
	return Vector2D(vect(HAVOC_2D_X), vect(HAVOC_2D_Y));
}

inline Vector2D to2DRight(const hkRotation& rot)
{
	return to2D(rot.getColumn(HAVOC_2D_X));
}

inline Vector2D to2DFwd(const hkRotation& rot)
{
	return to2D(rot.getColumn(HAVOC_2D_Y));
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
	

	void HandleEvent(const SDL_Event& evt)
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
				m_steer.x = 1.0f;
			}
			else if (evt.key.keysym.sym == SDLK_LEFT)
			{
				m_steer.x = -1.0f;
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


			if (evt.key.keysym.sym == SDLK_RIGHT && m_steer.x == 1.0f)
			{
				m_steer.x = 0.0f;
			}
			else if (evt.key.keysym.sym == SDLK_LEFT && m_steer.x == -1.0f)
			{
				m_steer.x = 0.0f;
			}
		}
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

	void HK_CALL steer(hkReal timestep)
	{
		m_world->lock();

		hkpVehicleDriverInputAnalogStatus*	deviceStatus = (hkpVehicleDriverInputAnalogStatus*)m_vehicle->m_deviceStatus;
		deviceStatus->m_positionX = -m_steer.x;
		deviceStatus->m_positionY = -m_steer.y;

		//if (deviceStatus->m_positionX != 0.0f || deviceStatus->m_positionY != 0.0f)
		{
			m_vehicle->getChassis()->activate();
		}

		/*
		//HKG_PAD_BUTTON padButtonState = pad->getButtonState();

		//
		// Update controller "position" within range [-1, 1] for both X, Y directions.
		//
		hkpVehicleDriverInputAnalogStatus*	deviceStatus = (hkpVehicleDriverInputAnalogStatus*)vehicle.m_deviceStatus;

		const float steerSpeed     =  3.0f * timestep;
		const float backSteerSpeed = 10.0f * timestep;
		hkReal deltaY = -inputYPosition  * 0.2f;
		hkReal deltaX = -inputXPosition  * backSteerSpeed;

		if ( (padButtonState & (HKG_PAD_BUTTON_0|HKG_PAD_DPAD_UP)) != 0 )
		{
			// Accelerate.
			deltaY = -0.1f;
		}
		else if ( (padButtonState & (HKG_PAD_BUTTON_2|HKG_PAD_DPAD_DOWN)) != 0 )
		{
			// Brake/reverse.
			deltaY = 0.1f;
		}

		if ( (padButtonState & HKG_PAD_DPAD_LEFT) != 0 )
		{
			// Turn left.
			if ( inputXPosition <= 0.0f){	deltaX = 0.0f;	}
			deltaX -= steerSpeed;
		}
		else if ( (padButtonState & HKG_PAD_DPAD_RIGHT) != 0 )
		{
			// Turn right.
			if ( inputXPosition >= 0.0f){	deltaX = 0.0f;	}
			deltaX += steerSpeed;
		}

		// add analog controls
		{
			deltaY -= .12f * pad->getStickPosY(1);

			hkReal x = pad->getStickPosX(0);
			//if ( x < 0 && inputXPosition <= 0.0f){	deltaX = 0.0f;	}
			//if ( x > 0 && inputXPosition >= 0.0f){	deltaX = 0.0f;	}
			deltaX += x * backSteerSpeed;
		}


		inputXPosition = hkMath::clamp( inputXPosition+deltaX, -1.0f, 1.0f);
		inputYPosition = hkMath::clamp( inputYPosition+deltaY, -1.0f, 1.0f);

		// Now -1 <= m_inputXPosition <= 1 and
		// -1 <= m_inputYPosition <= 1
		deviceStatus->m_positionY = inputYPosition;
		deviceStatus->m_positionX = inputXPosition;

		// Handbrake.
		//hkprintf( "%f %f\n", *inputXPosition, *inputYPosition);
		deviceStatus->m_handbrakeButtonPressed = (padButtonState & HKG_PAD_BUTTON_1) != 0;

		
		*/
		m_world->unlock();
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

			renderer.DrawQuad(renderer.WorldToScreen(quad[0]), 
								renderer.WorldToScreen(quad[1]), 
								renderer.WorldToScreen((quad[2]+quad[3]) * 0.5f), 
								renderer.WorldToScreen((quad[2]+quad[3]) * 0.5f), Color::kWhite);

			for (int i = 0; i < m_vehicle->m_wheelsInfo.getSize(); ++i)
			{
				const hkpVehicleInstance::WheelInfo& info = m_vehicle->m_wheelsInfo[i];
				Vector2D pos = to2D(info.m_hardPointWs);
				Vector2D dir = rotate90(to2D(info.m_spinAxisWs));
				//Vector2D dir = to2D(info.m_spinAxisWs);
				float radius = m_vehicle->m_data->m_wheelParams[i].m_radius;
				
				renderer.DrawLine(renderer.WorldToScreen(pos - (dir * radius)), 
								  renderer.WorldToScreen(pos + (dir * radius)),
								  Color::kWhite);
			}
		}
	}
};

#endif