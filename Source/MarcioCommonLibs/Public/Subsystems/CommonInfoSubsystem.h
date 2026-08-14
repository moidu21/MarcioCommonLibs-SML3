#pragma once
#include "Subsystem/ModSubsystem.h"

#include "CommonInfoSubsystem.generated.h"

UCLASS(BlueprintType)
class MARCIOCOMMONLIBS_API ACommonInfoSubsystem : public AModSubsystem
{
	GENERATED_BODY()
public:
	ACommonInfoSubsystem();

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	static ACommonInfoSubsystem* Get(UWorld* world);

	UFUNCTION(
		BlueprintCallable,
		Category="MarcioCommonLibs",
		DisplayName= "Get Common Info Subsystem",
		Meta = (WorldContext= "WorldContextObject", HidePin= "WorldContextObject", DefaultToSelf = "WorldContextObject")
	)
	static ACommonInfoSubsystem* Get(class UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category="MarcioCommonLibs")
	virtual void Initialize
	(
		UPARAM(DisplayName = "None Item Descriptor") const TSet<TSubclassOf<class UFGItemDescriptor>>& in_noneItemDescriptors,
		UPARAM(DisplayName = "Wildcard Item Descriptor") const TSet<TSubclassOf<class UFGItemDescriptor>>& in_wildcardItemDescriptors,
		UPARAM(DisplayName = "Any Undefined Item Descriptor") const TSet<TSubclassOf<class UFGItemDescriptor>>& in_anyUndefinedItemDescriptors,
		UPARAM(DisplayName = "Overflow Item Descriptor") const TSet<TSubclassOf<class UFGItemDescriptor>>& in_overflowItemDescriptors,
		UPARAM(DisplayName = "Nuclear Waste Item Descriptor") const TSet<TSubclassOf<class UFGItemDescriptor>>& in_nuclearWasteItemDescriptors,
		UPARAM(DisplayName = "Storage Container Class") const TSet<UClass*>& in_storageContainerClasses,
		UPARAM(DisplayName = "Power Pole Class") const TSet<UClass*>& in_powerPoleClasses,
		UPARAM(DisplayName = "Power Pole Wall Class") const TSet<UClass*>& in_powerPoleWallClasses,
		UPARAM(DisplayName = "Power Pole Wall Double Class") const TSet<UClass*>& in_powerPoleWallDoubleClasses,
		UPARAM(DisplayName = "Power Tower Class") const TSet<UClass*>& in_powerTowerClasses
	);

	virtual void AddClass(TSet<UClass*>& classes, const FString& classPath);

	UFUNCTION(BlueprintCallable, Category="MarcioCommonLibs")
	virtual bool IsStorageTeleporter(AActor* actor, TSubclassOf<AActor> cls = nullptr);

	UFUNCTION(BlueprintCallable, Category="MarcioCommonLibs")
	virtual bool IsItemTeleportEmitter(AActor* actor, TSubclassOf<AActor> cls = nullptr);

	UFUNCTION(BlueprintCallable, Category="MarcioCommonLibs")
	virtual bool IsItemTeleportReceiver(AActor* actor, TSubclassOf<AActor> cls = nullptr);

	virtual bool TryGetItemTeleportFrequency(AActor* actor, int64& outFrequency) const;

	UFUNCTION(BlueprintCallable, Category="MarcioCommonLibs")
	virtual bool IsFluidTeleportEmitter(AActor* actor, TSubclassOf<AActor> cls = nullptr);

	UFUNCTION(BlueprintCallable, Category="MarcioCommonLibs")
	virtual bool IsFluidTeleportReceiver(AActor* actor, TSubclassOf<AActor> cls = nullptr);

	virtual bool TryGetFluidTeleportFrequency(AActor* actor, int64& outFrequency) const;

	UFUNCTION(BlueprintCallable, Category="MarcioCommonLibs")
	virtual bool IsPowerPole(AActor* actor, TSubclassOf<AActor> cls = nullptr);

	UFUNCTION(BlueprintCallable, Category="MarcioCommonLibs")
	virtual bool IsPowerPoleWall(AActor* actor, TSubclassOf<AActor> cls = nullptr);

	UFUNCTION(BlueprintCallable, Category="MarcioCommonLibs")
	virtual bool IsPowerPoleWallDouble(AActor* actor, TSubclassOf<AActor> cls = nullptr);

	UFUNCTION(BlueprintCallable, Category="MarcioCommonLibs")
	virtual bool IsPowerTower(AActor* actor, TSubclassOf<AActor> cls = nullptr);

	UFUNCTION(BlueprintCallable, Category="MarcioCommonLibs")
	virtual bool IsStorageContainer(AActor* actor, TSubclassOf<AActor> cls = nullptr);

	UFUNCTION(BlueprintCallable, Category="MarcioCommonLibs")
	virtual bool IsModularLoadBalancer(AActor* actor, TSubclassOf<AActor> cls = nullptr);

	UFUNCTION(BlueprintCallable, Category="EfficiencyCheckerLogic")
	virtual bool IsCounterLimiter(AActor* actor, TSubclassOf<AActor> cls = nullptr);

	UFUNCTION(BlueprintCallable, Category="EfficiencyCheckerLogic")
	virtual bool IsUndergroundSplitter(AActor* actor, TSubclassOf<AActor> cls = nullptr);

	UFUNCTION(BlueprintCallable, Category="EfficiencyCheckerLogic")
	virtual bool IsUndergroundSplitterInput(AActor* actor, TSubclassOf<AActor> cls = nullptr);

	UFUNCTION(BlueprintCallable, Category="EfficiencyCheckerLogic")
	virtual bool IsUndergroundSplitterOutput(AActor* actor, TSubclassOf<AActor> cls = nullptr);

	UFUNCTION()
	virtual void handleBuildableConstructed(class AFGBuildable* buildable);

	UFUNCTION()
	virtual void handleBuildableRemoved(class AFGBuildable* buildable);

	UFUNCTION(BlueprintCallable, Category="EfficiencyCheckerLogic")
	virtual bool IsValidBuildable(class AFGBuildable* newBuildable);

	virtual void addTeleporter(class AFGBuildableFactory* teleporter);
	virtual void addItemTeleportEmitter(class AFGBuildableFactory* emitter);
	virtual void addItemTeleportReceiver(class AFGBuildableFactory* receiver);
	virtual void addFluidTeleportEmitter(class AFGBuildableFactory* emitter);
	virtual void addFluidTeleportReceiver(class AFGBuildableFactory* receiver);
	virtual void addUndergroundInputBelt(class AFGBuildableStorage* actor);

	UFUNCTION()
	virtual void removeTeleporter(AActor* teleporter/*, EEndPlayReason::Type reason*/);
	virtual void removeItemTeleportNode(AActor* node);
	virtual void removeFluidTeleportNode(AActor* node);
	UFUNCTION()
	virtual void removeUndergroundInputBelt(AActor* undergroundInputBelt/*, EEndPlayReason::Type reason*/);

	static FCriticalSection mclCritical;

	UPROPERTY(BlueprintReadWrite, Category="MarcioCommonLibs")
	bool initialized = false;

	UPROPERTY(BlueprintReadWrite, Category="MarcioCommonLibs")
	TObjectPtr<UClass> baseStorageTeleporterClass = nullptr;
	UPROPERTY(BlueprintReadWrite, Category="MarcioCommonLibs")
	TObjectPtr<UClass> baseUndergroundSplitterInputClass = nullptr;
	UPROPERTY(BlueprintReadWrite, Category="MarcioCommonLibs")
	TObjectPtr<UClass> baseUndergroundSplitterOutputClass = nullptr;
	UPROPERTY(BlueprintReadWrite, Category="MarcioCommonLibs")
	TObjectPtr<UClass> baseModularLoadBalancerClass = nullptr;
	UPROPERTY(BlueprintReadWrite, Category="MarcioCommonLibs")
	TObjectPtr<UClass> baseBuildableFactorySimpleProducerClass = nullptr;
	UPROPERTY(BlueprintReadWrite, Category="MarcioCommonLibs")
	TObjectPtr<UClass> baseCounterLimiterClass = nullptr;

	UPROPERTY(BlueprintReadWrite, Category="MarcioCommonLibs")
	TSet<TObjectPtr<UClass>> storageContainerClasses;

	UPROPERTY(BlueprintReadWrite, Category="MarcioCommonLibs")
	TSet<TObjectPtr<UClass>> powerPoleClasses;
	UPROPERTY(BlueprintReadWrite, Category="MarcioCommonLibs")
	TSet<TObjectPtr<UClass>> powerPoleWallClasses;
	UPROPERTY(BlueprintReadWrite, Category="MarcioCommonLibs")
	TSet<TObjectPtr<UClass>> powerPoleWallDoubleClasses;
	UPROPERTY(BlueprintReadWrite, Category="MarcioCommonLibs")
	TSet<TObjectPtr<UClass>> powerTowerClasses;

	UPROPERTY(BlueprintReadWrite, Category="MarcioCommonLibs")
	TSet<TSubclassOf<class UFGItemDescriptor>> nuclearWasteItemDescriptors;
	UPROPERTY(BlueprintReadWrite, Category="MarcioCommonLibs")
	TSet<TSubclassOf<class UFGItemDescriptor>> noneItemDescriptors;
	UPROPERTY(BlueprintReadWrite, Category="MarcioCommonLibs")
	TSet<TSubclassOf<class UFGItemDescriptor>> wildCardItemDescriptors;
	UPROPERTY(BlueprintReadWrite, Category="MarcioCommonLibs")
	TSet<TSubclassOf<class UFGItemDescriptor>> anyUndefinedItemDescriptors;
	UPROPERTY(BlueprintReadWrite, Category="MarcioCommonLibs")
	TSet<TSubclassOf<class UFGItemDescriptor>> overflowItemDescriptors;

	UPROPERTY(Transient)
	TSet<TObjectPtr<class AFGBuildableFactory>> allTeleporters;

	/** TeleportItem nodes are kept separate from Storage Teleporter nodes because
	 *  they use an integer frequency instead of the StorageID string. */
	UPROPERTY(Transient)
	TSet<TObjectPtr<class AFGBuildableFactory>> allItemTeleportEmitters;

	UPROPERTY(Transient)
	TSet<TObjectPtr<class AFGBuildableFactory>> allItemTeleportReceivers;

	/** Fluid endpoints use the same integer frequency concept but expose it on
	 *  their native endpoint component instead of as a Blueprint actor field. */
	UPROPERTY(Transient)
	TSet<TObjectPtr<class AFGBuildableFactory>> allFluidTeleportEmitters;

	UPROPERTY(Transient)
	TSet<TObjectPtr<class AFGBuildableFactory>> allFluidTeleportReceivers;

	UPROPERTY(Transient)
	TSet<TObjectPtr<class AFGBuildableStorage>> allUndergroundInputBelts;

	// FActorEndPlaySignature::FDelegate removeTeleporterDelegate;
protected:
	static TWeakObjectPtr<ACommonInfoSubsystem> instance;
};
