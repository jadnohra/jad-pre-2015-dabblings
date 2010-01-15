#ifndef PHYSICS_H
#define PHYSICS_H


// Math and base include
#include <Common/Base/hkBase.h>
#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>
#include <Common/Base/Memory/hkThreadMemory.h>
#include <Common/Base/Memory/Memory/Pool/hkPoolMemory.h>
#include <Common/Base/System/Error/hkDefaultError.h>
#include <Common/Base/Monitor/hkMonitorStream.h>

// Dynamics includes
#include <Physics/Collide/hkpCollide.h>
#include <Physics/Collide/Agent/ConvexAgent/SphereBox/hkpSphereBoxAgent.h>
#include <Physics/Collide/Shape/Convex/Box/hkpBoxShape.h>
#include <Physics/Collide/Shape/Convex/Sphere/hkpSphereShape.h>
#include <Physics/Collide/Dispatch/hkpAgentRegisterUtil.h>


#include <Physics/Dynamics/World/hkpWorld.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include <Physics/Utilities/Dynamics/Inertia/hkpInertiaTensorComputer.h>

// Visual Debugger includes
#include <Common/Visualize/hkVisualDebugger.h>
#include <Physics/Utilities/VisualDebugger/hkpPhysicsContext.h>

// Keycode
#include <Common/Base/keycode.cxx>

#define HK_CLASSES_FILE <Common/Serialize/ClassList/hkPhysicsClasses.h>
#include <Common/Serialize/Util/hkBuiltinTypeRegistry.cxx>

// Generate a custom list to trim memory requirements
#define HK_COMPAT_FILE <Common/Compat/hkCompatVersions.h>
#include <Common/Compat/hkCompat_None.cxx>

#include "TestVehicle.h"

// Required for time routines below
#ifdef HK_PLATFORM_PS2
#include <libpc.h>
#endif

#include <stdio.h>


static void HK_CALL errorReport(const char* msg, void*)
{
	printf("%s", msg);
}

// Metrowerks does not deadstrip properly.
#if defined( HK_COMPILER_MWERKS )
hkTestEntry* hkUnitTestDatabase;
hkBool HK_CALL hkTestReport(hkBool32 cond, const char* desc, const char* file, int line)
{
	errorReport(desc, 0);
	return false;
}
#endif

void PlatformInit();

class Physics
{
public:

	hkpWorld* physicsWorld;
	hkpPhysicsContext* context;
	hkVisualDebugger* vdb;
	hkPoolMemory* memoryManager;
	hkThreadMemory* threadMemory;
	char* stackBuffer;


	Physics()
	{
		physicsWorld = NULL;
		context = NULL;
		vdb = NULL;
		memoryManager = NULL;
		threadMemory = NULL;
		stackBuffer = NULL;
	}

	~Physics()
	{
		Destroy();
	}

	void Destroy()
	{
		{
			//
			// Clean up physics and graphics
			//

			physicsWorld->removeReference();
			vdb->removeReference();

			// Contexts are not reference counted at the base class level by the VDB as 
			// they are just interfaces really. So only delete the context after you have 
			// finished using the VDB.
			context->removeReference(); 
		}
		// Deallocate stack area
		threadMemory->setStackArea(0, 0);
		hkDeallocate(stackBuffer);

		threadMemory->removeReference();

		// Quit base system
		hkBaseSystem::quit(); 
	}

	void Init()
	{
		// Do platform specific initialization
		PlatformInit();

		// Initialize the base system including our memory system
		memoryManager = new hkPoolMemory();
		threadMemory = new hkThreadMemory(memoryManager/*, 16*/);
		hkBaseSystem::init( memoryManager, threadMemory, errorReport );	
		memoryManager->removeReference();

		// We now initialize the stack area to 100k (fast temporary memory to be used by the engine).
		stackBuffer;
		{ 
			int stackSize = 0x100000;
			stackBuffer = hkAllocate<char>( stackSize, HK_MEMORY_CLASS_BASE);
			hkThreadMemory::getInstance().setStackArea( stackBuffer, stackSize);
		}

		{
			// Create the physics world
			physicsWorld;
			{
				hkpWorldCinfo worldInfo;
				worldInfo.setupSolverInfo(hkpWorldCinfo::SOLVER_TYPE_4ITERS_MEDIUM); 
				worldInfo.m_gravity = hkVector4(0.0f, -9.8f, 0.0f);	
				worldInfo.m_broadPhaseBorderBehaviour = hkpWorldCinfo::BROADPHASE_BORDER_FIX_ENTITY; // just fix the entity if the object falls off too far 

				// You must specify the size of the broad phase - objects should not be simulated outside this region
				worldInfo.setBroadPhaseWorldSize(2000.0f);
				physicsWorld = new hkpWorld(worldInfo);
			}

			// Register all collision agents, even though only box - box will be used in this particular example.
			// It's important to register collision agents before adding any entities to the world.
			{
				hkpAgentRegisterUtil::registerAllAgents( physicsWorld->getCollisionDispatcher() );
			}

			// Create all the physics rigid bodies
			setupPhysics( physicsWorld );

			//
			// Initialize the visual debugger so we can connect remotely to the simulation
			// The context must exist beyond the use of the VDB instance, and you can make
			// whatever contexts you like for your own viewer types.
			//

			context = new hkpPhysicsContext;
			hkpPhysicsContext::registerAllPhysicsProcesses(); // all the physics viewers
			context->addWorld(physicsWorld); // add the physics world so the viewers can see it
			vdb = setupVisualDebugger(context);


			//
			// Simulate the world for 1 minute.
			// Take fixed time steps of 1/60th of a second. 
			// This works well if your game runs solidly at 60Hz. If your game runs at 30Hz
			// you can take either 2 60Hz steps or 1 30Hz step. Note that at lower frequencies (i.e. 30 Hz)
			// more bullet through paper issues appear, and constraints will not be as stiff.
			// If you run at variable frame rate, or are likely to drop frames, you can consider
			// running your physics for a variable number of steps based on the system clock (i.e. last frame time).
			// Please refer to the user guide section on time stepping for a full treatment of this issue.
			//
		}
	}

	void Update(float dt)
	{
		//g_vehicle->steer(dt);
		physicsWorld->stepDeltaTime(dt);

		// Step the visual debugger
		stepVisualDebugger(vdb);
		// Step the graphics display (none in this demo).
	}

	void setupPhysics(hkpWorld* physicsWorld)
	{
		// Create the floor as a fixed box
		{
			hkpRigidBodyCinfo boxInfo;
			hkVector4 boxSize(1000.0f, 0.5f , 1000.0f);
			hkpBoxShape* boxShape = new hkpBoxShape(boxSize);
			boxInfo.m_shape = boxShape;
			boxInfo.m_motionType = hkpMotion::MOTION_FIXED;
			boxInfo.m_position.set(0.0f, 0.0f, 0.0f);
			boxInfo.m_restitution = 0.9f;

			hkpRigidBody* floor = new hkpRigidBody(boxInfo);
			boxShape->removeReference();

			physicsWorld->addEntity(floor);
			floor->removeReference();
		}

		// Create a moving sphere
		/*
		{
			hkReal sphereRadius = 0.5f;
			hkpConvexShape* sphereShape = new hkpSphereShape(sphereRadius);

			hkpRigidBodyCinfo sphereInfo;
			sphereInfo.m_shape = sphereShape;
			sphereInfo.m_position.set(0.0f, 5.0f, 0.0f);
			sphereInfo.m_motionType = hkpMotion::MOTION_SPHERE_INERTIA;

			// Compute mass properties
			hkReal sphereMass = 10.0f;
			hkpMassProperties sphereMassProperties;
			hkpInertiaTensorComputer::computeSphereVolumeMassProperties(sphereRadius, sphereMass, sphereMassProperties);
			sphereInfo.m_inertiaTensor = sphereMassProperties.m_inertiaTensor;
			sphereInfo.m_centerOfMass = sphereMassProperties.m_centerOfMass;
			sphereInfo.m_mass = sphereMassProperties.m_mass;					

			// Create sphere RigidBody
			hkpRigidBody* sphereRigidBody = new hkpRigidBody(sphereInfo);
			sphereShape->removeReference();

			physicsWorld->addEntity(sphereRigidBody);
			g_ball = sphereRigidBody;
			sphereRigidBody->removeReference();
		}

		{
			g_vehicle = new TestVehicle();
			g_vehicle->Create(physicsWorld);
		}
		*/
	}

	hkVisualDebugger* setupVisualDebugger(hkpPhysicsContext* physicsWorlds)
	{
		// Setup the visual debugger                 
		hkArray<hkProcessContext*> contexts;
		contexts.pushBack(physicsWorlds);

		hkVisualDebugger* vdb = new hkVisualDebugger(contexts); 
		vdb->serve(); 

		// Allocate memory for internal profiling information
		// You can discard this if you do not want Havok profiling information
		hkMonitorStream& stream = hkMonitorStream::getInstance();
		stream.resize( 500 * 1024 );	// 500K for timer info
		stream.reset();

		// Initialize PS2 performance counters
		// We could also initialize these to profile I$ or D$ performance
#ifdef HK_PLATFORM_PS2
		scePcStart( SCE_PC_CTE | SCE_PC_U0 | SCE_PC_U1 | SCE_PC0_ICACHE_MISS | SCE_PC1_CPU_CYCLE, 0 ,0 );
#endif

		return vdb;
	}

	void stepVisualDebugger(hkVisualDebugger* vdb)
	{
		// As the lock step to server or a large blocking send may cause a >7 sec wait (causing an untrappable interupt),
		// we must stop the perf counter to stop potential overflow. The counter is rest anyway after we have stepped anyway.
		// The vdb step could do this stop and start for you but then the type of perf timers
		// on scePcStart would be a default set and hidden from you. This is more explicit.
#ifdef HK_PLATFORM_PS2
		scePcStop(); 
#endif

		// Step the debugger
		vdb->step();

		// Reset internal profiling info for next frame
		hkMonitorStream::getInstance().reset();

		// On PS2 we must restart the performance counters every 7 seconds
		// otherwise we will get an exception which cannot be caught by the user
#ifdef HK_PLATFORM_PS2
		scePcStart( SCE_PC_CTE | SCE_PC_U0 | SCE_PC_U1 | SCE_PC0_ICACHE_MISS | SCE_PC1_CPU_CYCLE, 0 ,0 );
#endif
	}
};

#endif