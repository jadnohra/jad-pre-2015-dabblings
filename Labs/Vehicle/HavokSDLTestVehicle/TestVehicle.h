#ifndef TEST_VEHICLE_H
#define TEST_VEHICLE_H

#include <Physics/Vehicle/hkpVehicleInstance.h>
#include <Physics/Collide/Shape/Convex/ConvexVertices/hkpConvexVerticesShape.h>

#include "VehicleApiUtils.h"
#include "VehicleSetup.h"
#include "SDL.h"

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
		deviceStatus->m_positionX = m_steer.x;
		deviceStatus->m_positionY = -m_steer.y;

		if (deviceStatus->m_positionX != 0.0f || deviceStatus->m_positionY != 0.0f)
		{
			m_vehicle->getChassis()->activate();
			int x = 0;
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

	Vector2D to2D(const hkVector4& vect)
	{
		return Vector2D(vect(0), vect(2));
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
			Vector2D right = to2D(rot.getColumn(0));
			Vector2D fwd = to2D(rot.getColumn(2));

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
		}
	}
};

#endif