#pragma once
#include "GameFramework/Actor.h"
#include "Resources/FGItemDescriptor.h"

#include "MarcioCommonLibsUtils.generated.h"

UCLASS(BlueprintType)
class MARCIOCOMMONLIBS_API UMarcioCommonLibsUtils : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="MarcioCommonLibsUtils")
	static void DumpUnknownClass
	(
		UObject* obj,
		const FString& linePrefix = TEXT(""),
		const FString& filePrefix = TEXT(""),
		const FString& fileSuffix = TEXT(""),
		bool includeDateTime = true
	);

	UFUNCTION(BlueprintCallable, Category="MarcioCommonLibsUtils")
	static class AActor* GetHitActor(const FHitResult& hit);

	UFUNCTION(BlueprintCallable, Category="MarcioCommonLibsUtils")
	static void DumpInformation(AActor* worldContext, TSubclassOf<class UFGEquipmentDescriptor> itemDescriptorClass);

	UFUNCTION(BlueprintCallable, Category="MarcioCommonLibsUtils", BlueprintPure)
	static class AFGCharacterPlayer* GetFGPlayer(class UWidget* widget);

	UFUNCTION(BlueprintCallable, Category="MarcioCommonLibsUtils")
	static int32 getIndexFromName(const FString& name);

	static FString getEnumItemName(UEnum* MyEnum, int32 value);

	UFUNCTION(BlueprintCallable, Category="MarcioCommonLibsUtils")
	static class AFGBuildableTrainPlatform* getNthTrainPlatform(class AFGBuildableRailroadStation* station, int32 index);

	UFUNCTION(BlueprintCallable, Category="MarcioCommonLibsUtils")
	static void getTrainPlatformIndexes(class AFGBuildableTrainPlatform* trainPlatformCargo, TSet<int32>& indexes, TSet<AFGBuildableRailroadStation*>& destinationStations);
};
