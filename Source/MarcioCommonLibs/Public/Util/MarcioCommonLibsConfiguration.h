#pragma once

#include "GameFramework/Actor.h"
#include "Engine/GameInstance.h"
#include "MarcioCommonLibs_ConfigStruct.h"

#include "MarcioCommonLibsConfiguration.generated.h"

UCLASS(BlueprintType)
class MARCIOCOMMONLIBS_API UMarcioCommonLibsConfiguration : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="MarcioCommonLibsConfiguration")
	static void SetMarcioCommonLibsConfiguration
	(
		UPARAM(DisplayName = "Configuration") const struct FMarcioCommonLibs_ConfigStruct& in_configuration
	);

	UFUNCTION(BlueprintCallable, Category="MarcioCommonLibsConfiguration")
	static void GetMarcioCommonLibsConfiguration
	(
		UPARAM(DisplayName = "Log Level") int32& out_logLevel
	);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MarcioCommonLibsConfiguration")
	static int32 GetLogLevelMCL();

public:
	static FMarcioCommonLibs_ConfigStruct configuration;
};
