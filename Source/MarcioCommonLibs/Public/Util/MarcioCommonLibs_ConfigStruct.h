#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "MarcioCommonLibs_ConfigStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/MarcioCommonLibs/Configuration/MarcioCommonLibs_Config' */
USTRUCT(BlueprintType)
struct FMarcioCommonLibs_ConfigStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    int32 logLevel{};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FMarcioCommonLibs_ConfigStruct GetActiveConfig(UObject* WorldContext) {
        FMarcioCommonLibs_ConfigStruct ConfigStruct{};

        if (!GEngine || !IsValid(WorldContext)) {
            return ConfigStruct;
        }

        FConfigId ConfigId{"MarcioCommonLibs", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            if (UGameInstance* GameInstance = World->GetGameInstance()) {
                if (UConfigManager* ConfigManager = GameInstance->GetSubsystem<UConfigManager>()) {
                    ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FMarcioCommonLibs_ConfigStruct::StaticStruct(), &ConfigStruct});
                }
            }
        }
        return ConfigStruct;
    }
};
