// ReSharper disable CppUE4CodingStandardNamingViolationWarning
// ReSharper disable CommentTypo

#include "MarcioCommonLibsModule.h"
#include "Util/MCLOptimize.h"
#include "Buildables/FGBuildableFactory.h"
#include "Patching/NativeHookManager.h"
#include "Subsystems/CommonInfoSubsystem.h"

#define LOCTEXT_NAMESPACE "FMarcioCommonLibsModule"

void FMarcioCommonLibsModule::StartupModule()
{
}

void FMarcioCommonLibsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMarcioCommonLibsModule, MarcioCommonLibs)
