#ifndef TEST_VEHICLE_H
#define TEST_VEHICLE_H

#include <Physics/Vehicle/hkpVehicleInstance.h>
#include <Physics/Collide/Shape/Convex/ConvexVertices/hkpConvexVerticesShape.h>

#include "VehicleApiUtils.h"
#include "VehicleSetup.h"

class TestVehicle
{
public:

	hkpVehicleInstance* m_vehicle;
	hkpWorld* m_world;

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
		deviceStatus->m_positionX = 0;
		deviceStatus->m_positionY = 0;

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
};

#endif