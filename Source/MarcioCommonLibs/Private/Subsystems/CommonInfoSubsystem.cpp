#include "Subsystems/CommonInfoSubsystem.h"

#include "Buildables/FGBuildableFactory.h"
#include "Buildables/FGBuildableStorage.h"
#include "Components/ActorComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystem/SubsystemActorManager.h"
#include "UObject/UnrealType.h"
#include "Util/MarcioCommonLibsConfiguration.h"
#include "Util/MCLOptimize.h"

#ifndef OPTIMIZE
UE_DISABLE_OPTIMIZATION_SHIP
#endif

TWeakObjectPtr<ACommonInfoSubsystem> ACommonInfoSubsystem::instance;

FCriticalSection ACommonInfoSubsystem::mclCritical;

namespace
{
	bool ClassHierarchyContainsPath(const UClass* cls, const TCHAR* pathFragment)
	{
		for (const UClass* current = cls; current; current = current->GetSuperClass())
		{
			if (current->GetPathName().Contains(pathFragment, ESearchCase::IgnoreCase))
			{
				return true;
			}
		}
		return false;
	}

	UActorComponent* FindFluidTeleportEndpoint(AActor* actor)
	{
		if (!IsValid(actor))
		{
			return nullptr;
		}

		TInlineComponentArray<UActorComponent*> components(actor);
		for (UActorComponent* component : components)
		{
			if (IsValid(component) && component->GetClass()->GetPathName().Equals(
				TEXT("/Script/teleportitem.TeleportFluidEndpointComponent"),
				ESearchCase::IgnoreCase))
			{
				return component;
			}
		}
		return nullptr;
	}
}

ACommonInfoSubsystem::ACommonInfoSubsystem()
{
	ReplicationPolicy = ESubsystemReplicationPolicy::SpawnLocal;
}

void ACommonInfoSubsystem::BeginPlay()
{
	Super::BeginPlay();
}

void ACommonInfoSubsystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(this))
	{
		BuildableSubsystem->mBuildableAddedDelegate.RemoveDynamic(this, &ACommonInfoSubsystem::handleBuildableConstructed);
		BuildableSubsystem->mBuildableRemovedDelegate.RemoveDynamic(this, &ACommonInfoSubsystem::handleBuildableRemoved);
	}

	Super::EndPlay(EndPlayReason);

	noneItemDescriptors.Empty();
	wildCardItemDescriptors.Empty();
	anyUndefinedItemDescriptors.Empty();
	overflowItemDescriptors.Empty();
	nuclearWasteItemDescriptors.Empty();

	baseStorageTeleporterClass = nullptr;
	baseUndergroundSplitterInputClass = nullptr;
	baseUndergroundSplitterOutputClass = nullptr;
	baseModularLoadBalancerClass = nullptr;
	baseBuildableFactorySimpleProducerClass = nullptr;
	baseCounterLimiterClass = nullptr;

	storageContainerClasses.Empty();

	powerPoleClasses.Empty();
	powerPoleWallClasses.Empty();
	powerPoleWallDoubleClasses.Empty();
	powerTowerClasses.Empty();

	allTeleporters.Empty();
	allItemTeleportEmitters.Empty();
	allItemTeleportReceivers.Empty();
	allFluidTeleportEmitters.Empty();
	allFluidTeleportReceivers.Empty();
	allUndergroundInputBelts.Empty();

	initialized = false;

	instance.Reset();
}

ACommonInfoSubsystem* ACommonInfoSubsystem::Get(UWorld* world)
{
	if (!IsValid(world))
	{
		return nullptr;
	}

	if (USubsystemActorManager* SubsystemActorManager = world->GetSubsystem<USubsystemActorManager>())
	{
		return SubsystemActorManager->GetSubsystemActor<ACommonInfoSubsystem>();
	}

	return nullptr;
}

ACommonInfoSubsystem* ACommonInfoSubsystem::Get(UObject* WorldContextObject)
{
	if (!GEngine || !IsValid(WorldContextObject))
	{
		return nullptr;
	}

	return Get(GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull));
}

void ACommonInfoSubsystem::Initialize
(
	const TSet<TSubclassOf<UFGItemDescriptor>>& in_noneItemDescriptors,
	const TSet<TSubclassOf<UFGItemDescriptor>>& in_wildcardItemDescriptors,
	const TSet<TSubclassOf<UFGItemDescriptor>>& in_anyUndefinedItemDescriptors,
	const TSet<TSubclassOf<UFGItemDescriptor>>& in_overflowItemDescriptors,
	const TSet<TSubclassOf<UFGItemDescriptor>>& in_nuclearWasteItemDescriptors,
	const TSet<UClass*>& in_storageContainerClasses,
	const TSet<UClass*>& in_powerPoleClasses,
	const TSet<UClass*>& in_powerPoleWallClasses,
	const TSet<UClass*>& in_powerPoleWallDoubleClasses,
	const TSet<UClass*>& in_powerTowerClasses
)
{
	instance = this;

	noneItemDescriptors = in_noneItemDescriptors;
	wildCardItemDescriptors = in_wildcardItemDescriptors;
	anyUndefinedItemDescriptors = in_anyUndefinedItemDescriptors;
	overflowItemDescriptors = in_overflowItemDescriptors;
	nuclearWasteItemDescriptors = in_nuclearWasteItemDescriptors;

	baseStorageTeleporterClass = UClass::TryFindTypeSlow<UClass>(TEXT("/StorageTeleporter/Buildables/ItemTeleporter/ItemTeleporter_Build.ItemTeleporter_Build_C"));
	baseUndergroundSplitterInputClass = UClass::TryFindTypeSlow<UClass>(TEXT("/UndergroundBelts/Build/Build_UndergroundSplitterInput.Build_UndergroundSplitterInput_C"));
	baseUndergroundSplitterOutputClass = UClass::TryFindTypeSlow<UClass>(TEXT("/UndergroundBelts/Build/Build_UndergroundSplitterOutput.Build_UndergroundSplitterOutput_C"));
	baseModularLoadBalancerClass = UClass::TryFindTypeSlow<UClass>(TEXT("/Script/LoadBalancers.LBBuild_ModularLoadBalancer"));
	baseBuildableFactorySimpleProducerClass = UClass::TryFindTypeSlow<UClass>(TEXT("/Script/FactoryGame.FGBuildableFactorySimpleProducer"));
	baseCounterLimiterClass = UClass::TryFindTypeSlow<UClass>(TEXT("/Script/CounterLimiter.CL_CounterLimiter"));

	storageContainerClasses = in_storageContainerClasses;

	// AddClass(storageContainerClasses, TEXT("/Game/FactoryGame/Buildable/Factory/StorageContainerMk1/Build_StorageContainerMk1.Build_StorageContainerMk1_C"));
	// AddClass(storageContainerClasses, TEXT("/Game/FactoryGame/Buildable/Factory/StorageContainerMk2/Build_StorageContainerMk2.Build_StorageContainerMk2_C"));

	powerPoleClasses = in_powerPoleClasses;

	// AddClass(powerPoleClasses, TEXT("/Game/FactoryGame/Buildable/Factory/PowerPoleMk1/Build_PowerPoleMk1.Build_PowerPoleMk1_C"));
	// AddClass(powerPoleClasses, TEXT("/Game/FactoryGame/Buildable/Factory/PowerPoleMk2/Build_PowerPoleMk2.Build_PowerPoleMk2_C"));
	// AddClass(powerPoleClasses, TEXT("/Game/FactoryGame/Buildable/Factory/PowerPoleMk3/Build_PowerPoleMk3.Build_PowerPoleMk3_C"));

	powerPoleWallClasses = in_powerPoleWallClasses;

	// AddClass(powerPoleWallClasses, TEXT("/Game/FactoryGame/Buildable/Factory/PowerPoleWall/Build_PowerPoleWall.Build_PowerPoleWall_C"));
	// AddClass(powerPoleWallClasses, TEXT("/Game/FactoryGame/Buildable/Factory/PowerPoleWall/Build_PowerPoleWall_Mk2.Build_PowerPoleWall_Mk2_C"));
	// AddClass(powerPoleWallClasses, TEXT("/Game/FactoryGame/Buildable/Factory/PowerPoleWall/Build_PowerPoleWall_Mk3.Build_PowerPoleWall_Mk3_C"));

	powerPoleWallDoubleClasses = in_powerPoleWallDoubleClasses;

	// AddClass(powerPoleWallDoubleClasses, TEXT("/Game/FactoryGame/Buildable/Factory/PowerPoleWallDouble/Build_PowerPoleWallDouble.Build_PowerPoleWallDouble_C"));
	// AddClass(powerPoleWallDoubleClasses, TEXT("/Game/FactoryGame/Buildable/Factory/PowerPoleWallDouble/Build_PowerPoleWallDouble_Mk2.Build_PowerPoleWallDouble_Mk2_C"));
	// AddClass(powerPoleWallDoubleClasses, TEXT("/Game/FactoryGame/Buildable/Factory/PowerPoleWallDouble/Build_PowerPoleWallDouble_Mk3.Build_PowerPoleWallDouble_Mk3_C"));

	powerTowerClasses = in_powerTowerClasses;

	// AddClass(powerTowerClasses, TEXT("/Game/FactoryGame/Buildable/Factory/PowerTower/Build_PowerTower.Build_PowerTower_C"));
	// AddClass(powerTowerClasses, TEXT("/Game/FactoryGame/Buildable/Factory/PowerTower/Build_PowerTowerPlatform.Build_PowerTowerPlatform_C"));

	// removeTeleporterDelegate.BindDynamic(this, &ACommonInfoSubsystem::removeTeleporter);

	auto buildableSubsystem = AFGBuildableSubsystem::Get(this);

	if (buildableSubsystem)
	{
		buildableSubsystem->mBuildableAddedDelegate.AddUniqueDynamic(this, &ACommonInfoSubsystem::handleBuildableConstructed);
		buildableSubsystem->mBuildableRemovedDelegate.AddUniqueDynamic(this, &ACommonInfoSubsystem::handleBuildableRemoved);

		TArray<AActor*> allBuildables;
		UGameplayStatics::GetAllActorsOfClass(buildableSubsystem->GetWorld(), AFGBuildable::StaticClass(), allBuildables);

		for (auto buildableActor : allBuildables)
		{
			IsValidBuildable(Cast<AFGBuildable>(buildableActor));
		}
	}

	// #if UE_BUILD_SHIPPING
	// 	static auto hooked = false;
	//
	// 	if (!hooked)
	// 	{
	// 		hooked = true;
	//
	// 		{
	// 			auto ObjectInstance = GetMutableDefault<AFGBuildableFactory>();
	//
	// 			SUBSCRIBE_METHOD_VIRTUAL_AFTER(
	// 				AFGBuildableFactory::BeginPlay,
	// 				ObjectInstance,
	// 				[](AFGBuildableFactory* self)
	// 				{
	// 				if(instance)
	// 				{
	// 				instance->handleBuildableConstructed(self);
	// 				}
	// 				}
	// 				);
	// 		}
	// 	}
	// #endif

	initialized = true;
}

void ACommonInfoSubsystem::AddClass(TSet<UClass*>& classes, const FString& classPath)
{
	if (auto foundClass = UClass::TryFindTypeSlow<UClass>(classPath))
	{
		classes.Add(foundClass);
	}
}

bool ACommonInfoSubsystem::IsStorageTeleporter(AActor* actor, TSubclassOf<AActor> cls)
{
	if (actor)
	{
		cls = actor->GetClass();
	}

	return cls && baseStorageTeleporterClass && cls->IsChildOf(baseStorageTeleporterClass);
}

bool ACommonInfoSubsystem::IsItemTeleportEmitter(AActor* actor, TSubclassOf<AActor> cls)
{
	if (actor)
	{
		cls = actor->GetClass();
	}

	// Require both the TeleportItem hierarchy and its public frequency property.
	// A property name alone produced false positives with unrelated modded buildings.
	return cls &&
		ClassHierarchyContainsPath(cls, TEXT("/teleportitem/buildteleport.")) &&
		cls->FindPropertyByName(TEXT("Frequencytransmitter")) != nullptr;
}

bool ACommonInfoSubsystem::IsItemTeleportReceiver(AActor* actor, TSubclassOf<AActor> cls)
{
	if (actor)
	{
		cls = actor->GetClass();
	}

	return cls &&
		ClassHierarchyContainsPath(cls, TEXT("/teleportitem/receiver.")) &&
		cls->FindPropertyByName(TEXT("frequencyreceiver")) != nullptr;
}

bool ACommonInfoSubsystem::TryGetItemTeleportFrequency(AActor* actor, int64& outFrequency) const
{
	outFrequency = 0;

	if (!IsValid(actor))
	{
		return false;
	}

	const FName propertyName = actor->GetClass()->FindPropertyByName(TEXT("Frequencytransmitter"))
		? FName(TEXT("Frequencytransmitter"))
		: FName(TEXT("frequencyreceiver"));
	FProperty* property = actor->GetClass()->FindPropertyByName(propertyName);

	if (const FIntProperty* intProperty = CastField<FIntProperty>(property))
	{
		outFrequency = intProperty->GetPropertyValue_InContainer(actor);
		return true;
	}

	if (const FInt64Property* int64Property = CastField<FInt64Property>(property))
	{
		outFrequency = int64Property->GetPropertyValue_InContainer(actor);
		return true;
	}

	if (const FByteProperty* byteProperty = CastField<FByteProperty>(property))
	{
		outFrequency = byteProperty->GetPropertyValue_InContainer(actor);
		return true;
	}

	return false;
}

bool ACommonInfoSubsystem::IsFluidTeleportEmitter(AActor* actor, TSubclassOf<AActor> cls)
{
	if (actor)
	{
		cls = actor->GetClass();
	}

	return cls && ClassHierarchyContainsPath(
		cls,
		TEXT("/teleportitem/fluid/teleportfluid."));
}

bool ACommonInfoSubsystem::IsFluidTeleportReceiver(AActor* actor, TSubclassOf<AActor> cls)
{
	if (actor)
	{
		cls = actor->GetClass();
	}

	return cls && ClassHierarchyContainsPath(
		cls,
		TEXT("/teleportitem/fluid/receiverfluid."));
}

bool ACommonInfoSubsystem::TryGetFluidTeleportFrequency(AActor* actor, int64& outFrequency) const
{
	outFrequency = 0;
	UActorComponent* endpoint = FindFluidTeleportEndpoint(actor);
	if (!endpoint)
	{
		return false;
	}

	if (const FIntProperty* frequencyProperty =
		FindFProperty<FIntProperty>(endpoint->GetClass(), TEXT("Frequency")))
	{
		outFrequency = frequencyProperty->GetPropertyValue_InContainer(endpoint);
		return true;
	}

	return false;
}

bool ACommonInfoSubsystem::IsPowerPole(AActor* actor, TSubclassOf<AActor> cls)
{
	if (actor)
	{
		cls = actor->GetClass();
	}

	return cls && powerPoleClasses.Contains(cls);
}

bool ACommonInfoSubsystem::IsPowerPoleWall(AActor* actor, TSubclassOf<AActor> cls)
{
	if (actor)
	{
		cls = actor->GetClass();
	}

	return cls && powerPoleWallClasses.Contains(cls);
}

bool ACommonInfoSubsystem::IsPowerPoleWallDouble(AActor* actor, TSubclassOf<AActor> cls)
{
	if (actor)
	{
		cls = actor->GetClass();
	}

	return cls && powerPoleWallDoubleClasses.Contains(cls);
}

bool ACommonInfoSubsystem::IsPowerTower(AActor* actor, TSubclassOf<AActor> cls)
{
	if (actor)
	{
		cls = actor->GetClass();
	}

	return cls && powerTowerClasses.Contains(cls);
}

bool ACommonInfoSubsystem::IsStorageContainer(AActor* actor, TSubclassOf<AActor> cls)
{
	if (actor)
	{
		cls = actor->GetClass();
	}

	return cls && storageContainerClasses.Contains(cls);
}

bool ACommonInfoSubsystem::IsModularLoadBalancer(AActor* actor, TSubclassOf<AActor> cls)
{
	if (actor)
	{
		cls = actor->GetClass();
	}

	return cls && baseModularLoadBalancerClass && cls->IsChildOf(baseModularLoadBalancerClass);
}

bool ACommonInfoSubsystem::IsCounterLimiter(AActor* actor, TSubclassOf<AActor> cls)
{
	if (actor)
	{
		cls = actor->GetClass();
	}

	return cls && baseCounterLimiterClass && cls->IsChildOf(baseCounterLimiterClass);
}

bool ACommonInfoSubsystem::IsUndergroundSplitter(AActor* actor, TSubclassOf<AActor> cls)
{
	return IsUndergroundSplitterInput(actor, cls) || IsUndergroundSplitterOutput(actor, cls);
}

bool ACommonInfoSubsystem::IsUndergroundSplitterInput(AActor* actor, TSubclassOf<AActor> cls)
{
	if (actor)
	{
		cls = actor->GetClass();
	}

	return cls && baseUndergroundSplitterInputClass && cls->IsChildOf(baseUndergroundSplitterInputClass);
}

bool ACommonInfoSubsystem::IsUndergroundSplitterOutput(AActor* actor, TSubclassOf<AActor> cls)
{
	if (actor)
	{
		cls = actor->GetClass();
	}

	return cls && baseUndergroundSplitterOutputClass && cls->IsChildOf(baseUndergroundSplitterOutputClass);
}

void ACommonInfoSubsystem::handleBuildableConstructed(AFGBuildable* buildable)
{
	IsValidBuildable(buildable);
}

bool ACommonInfoSubsystem::IsValidBuildable(AFGBuildable* newBuildable)
{
	if (!newBuildable)
	{
		return false;
	}

	if (IsItemTeleportEmitter(newBuildable))
	{
		if (auto emitter = Cast<AFGBuildableFactory>(newBuildable))
		{
			addItemTeleportEmitter(emitter);
		}

		return true;
	}
	else if (IsItemTeleportReceiver(newBuildable))
	{
		if (auto receiver = Cast<AFGBuildableFactory>(newBuildable))
		{
			addItemTeleportReceiver(receiver);
		}

		return true;
	}
	else if (IsFluidTeleportEmitter(newBuildable))
	{
		if (auto emitter = Cast<AFGBuildableFactory>(newBuildable))
		{
			addFluidTeleportEmitter(emitter);
		}

		return true;
	}
	else if (IsFluidTeleportReceiver(newBuildable))
	{
		if (auto receiver = Cast<AFGBuildableFactory>(newBuildable))
		{
			addFluidTeleportReceiver(receiver);
		}

		return true;
	}
	else if (IsUndergroundSplitterInput(newBuildable))
	{
		if (auto underGroundBelt = Cast<AFGBuildableStorage>(newBuildable))
		{
			addUndergroundInputBelt(underGroundBelt);

			return true;
		}
	}
	else if (IsStorageTeleporter(newBuildable))
	{
		if (auto storageTeleporter = Cast<AFGBuildableFactory>(newBuildable))
		{
			addTeleporter(storageTeleporter);
		}

		return true;
	}

	return false;
}

void ACommonInfoSubsystem::handleBuildableRemoved(AFGBuildable* buildable)
{
	if (!buildable)
	{
		return;
	}

	if (IsItemTeleportEmitter(buildable) || IsItemTeleportReceiver(buildable))
	{
		removeItemTeleportNode(buildable);
	}
	else if (IsFluidTeleportEmitter(buildable) || IsFluidTeleportReceiver(buildable))
	{
		removeFluidTeleportNode(buildable);
	}
	else if (IsUndergroundSplitterInput(buildable))
	{
		if (auto underGroundBelt = Cast<AFGBuildableStorage>(buildable))
		{
			removeUndergroundInputBelt(underGroundBelt);
		}
	}
	else if (IsStorageTeleporter(buildable))
	{
		if (auto storageTeleporter = Cast<AFGBuildableFactory>(buildable))
		{
			removeTeleporter(storageTeleporter);
		}
	}
}

void ACommonInfoSubsystem::addTeleporter(AFGBuildableFactory* teleporter)
{
	FScopeLock ScopeLock(&mclCritical);

	if (!allTeleporters.Contains(teleporter))
	{
		allTeleporters.Add(teleporter);

		// teleporter->OnEndPlay.AddDynamic(this, &ACommonInfoSubsystem::removeTeleporter);
	}
}

void ACommonInfoSubsystem::removeTeleporter(AActor* teleporter/*, EEndPlayReason::Type reason*/)
{
	FScopeLock ScopeLock(&mclCritical);
	allTeleporters.Remove(Cast<AFGBuildableFactory>(teleporter));

	// teleporter->OnEndPlay.RemoveDynamic(this, &ACommonInfoSubsystem::removeTeleporter);
}

void ACommonInfoSubsystem::addItemTeleportEmitter(AFGBuildableFactory* emitter)
{
	FScopeLock ScopeLock(&mclCritical);
	allItemTeleportEmitters.Add(emitter);
}

void ACommonInfoSubsystem::addItemTeleportReceiver(AFGBuildableFactory* receiver)
{
	FScopeLock ScopeLock(&mclCritical);
	allItemTeleportReceivers.Add(receiver);
}

void ACommonInfoSubsystem::removeItemTeleportNode(AActor* node)
{
	FScopeLock ScopeLock(&mclCritical);
	AFGBuildableFactory* buildable = Cast<AFGBuildableFactory>(node);
	allItemTeleportEmitters.Remove(buildable);
	allItemTeleportReceivers.Remove(buildable);
}

void ACommonInfoSubsystem::addFluidTeleportEmitter(AFGBuildableFactory* emitter)
{
	FScopeLock ScopeLock(&mclCritical);
	allFluidTeleportEmitters.Add(emitter);
}

void ACommonInfoSubsystem::addFluidTeleportReceiver(AFGBuildableFactory* receiver)
{
	FScopeLock ScopeLock(&mclCritical);
	allFluidTeleportReceivers.Add(receiver);
}

void ACommonInfoSubsystem::removeFluidTeleportNode(AActor* node)
{
	FScopeLock ScopeLock(&mclCritical);
	AFGBuildableFactory* buildable = Cast<AFGBuildableFactory>(node);
	allFluidTeleportEmitters.Remove(buildable);
	allFluidTeleportReceivers.Remove(buildable);
}

void ACommonInfoSubsystem::addUndergroundInputBelt(AFGBuildableStorage* undergroundInputBelt)
{
	FScopeLock ScopeLock(&ACommonInfoSubsystem::mclCritical);

	if (!allUndergroundInputBelts.Contains(undergroundInputBelt))
	{
		allUndergroundInputBelts.Add(undergroundInputBelt);

		// undergroundInputBelt->OnEndPlay.AddDynamic(this, &ACommonInfoSubsystem::removeUndergroundInputBelt);
	}
}

void ACommonInfoSubsystem::removeUndergroundInputBelt(AActor* undergroundInputBelt/*, EEndPlayReason::Type reason*/)
{
	FScopeLock ScopeLock(&ACommonInfoSubsystem::mclCritical);
	allUndergroundInputBelts.Remove(Cast<AFGBuildableStorage>(undergroundInputBelt));

	// undergroundInputBelt->OnEndPlay.RemoveDynamic(this, &ACommonInfoSubsystem::removeUndergroundInputBelt);
}

#ifndef OPTIMIZE
UE_ENABLE_OPTIMIZATION_SHIP
#endif
