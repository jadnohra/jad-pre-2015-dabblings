/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <Common/Base/hkBase.h>

#if defined(HK_PLATFORM_PS3)
#	include <StandAloneDemos/ConsoleExample/ps3setup.h>
#endif

#if defined(HK_PLATFORM_PS2)
#  if defined(HK_COMPILER_MWERKS)
#	  include <mwutils.h> // Include for mwInit call (see comment below)
#  endif
#  include <StandAloneDemos/ConsoleExample/ps2setup.h>   // Required to load modules for PS2
#endif

#if defined(HK_PLATFORM_GC)
#  include <StandAloneDemos/ConsoleExample/gcnsetup.h>   // Required to init GC heap and define new and delete
#endif

#if defined(HK_PLATFORM_XBOX)
#if defined(HK_ARCH_IA32)
#  include <StandAloneDemos/ConsoleExample/xboxsetup.h>   // Clears the display buffers to dark green so that
// you know the app is running at least. Purely cosmetic
#else
#  include <StandAloneDemos/ConsoleExampleMt/xothersetup.h> // setup another type of xbox..
#endif
#endif

#if defined( HK_PLATFORM_PSP )

#include <StandAloneDemos/ConsoleExample/R3000setup.h>
#include <moduleexport.h>
#include <kernel.h>

// for .prx generation
SCE_MODULE_INFO( Havok_SimpleConsole, 0, 1, 1 );

// 24MB heap space
#if defined(HK_PLATFORM_PSP)
#	if defined(HK_COMPILER_MWERKS)
	// stack size used for main thread; cannot use mw_psp_support.c and
	// modify size after.
	int sce_newlib_heap_kb_size = 1024 * 24;
	char sce_user_main_thread_name[] = "user_main";
	int sce_user_main_thread_stack_kb_size = 256;
	int sce_user_main_thread_priority = SCE_KERNEL_MODULE_INIT_PRIORITY;
	unsigned int sce_user_main_thread_attribute = SCE_KERNEL_TH_UMODE;
	extern "C"
	{
		void _init(void){}
		void _fini(void){}
	}
#	else
	int sce_newlib_heap_kb_size = 1024 * 24;
#	endif
#endif

#if defined(HK_COMPILER_MWERKS)
#include <mwutils.h>
#endif

// division by zero workaround
void SetC1( int value )
{
	__asm__ volatile("ctc1 $4, $31");
}

#endif

void PlatformInit()
{
#if defined( HK_PLATFORM_PSP )

	SetC1(0);

	R3000Initialisation();

	// enable VFPU
	int status = sceKernelChangeCurrentThreadAttr( HK_NULL, SCE_KERNEL_TH_USE_VFPU );

	if( status != SCE_KERNEL_ERROR_OK )
	{
		HK_ASSERT2( 0x00, false, "VFPU not available!" );
	}

#endif

	// Xbox initialization : this simply sets the screen colour to dark green, not required (just astetic).
#if defined(HK_PLATFORM_XBOX)
#if defined(HK_ARCH_IA32)
	InitXBox();
#else
	InitXOther();
#endif
#endif

	// Gamecube initialization : this simply sets up the heap
#if defined(HK_PLATFORM_GC)
	InitGCN();
#endif

	// PSP MW initialization
#if defined(HK_COMPILER_MWERKS) && defined(HK_PLATFORM_PSP)
	mwInit();
#endif

	// PS2 initialization : If you are using this simple app as a reference
	// examine ps2setup.h to find out which modules you must load and initialize
#if defined(HK_PLATFORM_PS2)

#if defined(HK_COMPILER_MWERKS)
	// On PS2 (Metrowerks only) it is necessary to call mwInit to call global
	// constructors or initialise global objects
	mwInit();
#endif

	// This structure holds our ps2 initialization settings
	PS2SetupOptions ps2Options;

	// Initialze defaults for IRX modules, reboot IOP settings
	ps2SetupDefaults(ps2Options);

	// Initialize and reboot IOP, load IRX modules for file streaming and visual debugger
	ps2LoadModules(ps2Options);

#endif

#if defined(HK_PLATFORM_PS3)
	InitPS3();
#endif

}

/*
* Havok SDK - DEMO RELEASE, BUILD(#20090320)
*
* Confidential Information of Havok.  (C) Copyright 1999-2009 
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
*
* Use of this software for evaluation purposes is subject to and indicates 
* acceptance of the End User licence Agreement for this product. A copy of 
* the license is included with this software and is also available from salesteam@havok.com.
*
*/
