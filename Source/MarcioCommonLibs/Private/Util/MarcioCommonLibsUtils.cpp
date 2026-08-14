#include "Util/MarcioCommonLibsUtils.h"

#include "Animation/AimOffsetBlendSpace.h"
#include "Animation/AnimSequence.h"
#include "AbstractInstanceManager.h"
#include "FactoryTick.h"
#include "FGCategory.h"
#include "FGCharacterPlayer.h"
#include "FGFactoryConnectionComponent.h"
#include "FGTrainPlatformConnection.h"
#include "FGTrainStationIdentifier.h"
#include "Buildables/FGBuildableRailroadStation.h"
#include "Buildables/FGBuildableTrainPlatform.h"
#include "Buildables/FGBuildableTrainPlatformCargo.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkinnedAssetCommon.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformFileManager.h"
#include "Internationalization/Regex.h"
#include "Misc/FileHelper.h"
#include "Misc/ScopeLock.h"
#include "Misc/StringBuilder.h"
#include "Util/MarcioCommonLibsConfiguration.h"
#include "Util/MCLLogging.h"
#include "Reflection/BlueprintReflectionLibrary.h"
#include "Resources/FGEquipmentDescriptor.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMesh.h"
#include "UObject/UnrealType.h"

#ifndef OPTIMIZE
UE_DISABLE_OPTIMIZATION_SHIP
#endif

void UMarcioCommonLibsUtils::DumpUnknownClass
(
	UObject* obj,
	const FString& linePrefix,
	const FString& filePrefix,
	const FString& fileSuffix,
	bool includeDateTime
)
{
	if (!IsValid(obj))
	{
		MCL_LOG_Warning(TEXT("DumpUnknownClass called with an invalid object"));
		return;
	}

	static FCriticalSection DumpCriticalSection;
	FScopeLock DumpLock(&DumpCriticalSection);

	auto& platformFile = FPlatformFileManager::Get().GetPlatformFile();

	auto dumpFolder = FPaths::Combine(FPaths::ProjectDir(), TEXT("DumpedObjects"));

	platformFile.CreateDirectoryTree(*dumpFolder);

	static bool removePreviousFiles = true;
	if (removePreviousFiles)
	{
		// Delete previous dump files
		removePreviousFiles = false;

		TArray<FString> foundFiles;

		platformFile.FindFiles(foundFiles, *dumpFolder, TEXT(".txt"));

		for (auto file : foundFiles)
		{
			platformFile.DeleteFile(*file);
		}
	}

	auto baseFileName = filePrefix + obj->GetName() + (includeDateTime ? FDateTime::Now().ToString(TEXT("-%Y%m%d_%H%M%S")) : TEXT("")) + fileSuffix;

	auto dumpFile = FPaths::Combine(
		dumpFolder,
		baseFileName + TEXT(".txt")
		);

	if (!fileSuffix.IsEmpty())
	{
		for (auto x = 1; platformFile.FileExists(*dumpFile); x++)
		{
			dumpFile = FPaths::Combine(dumpFolder, baseFileName + TEXT("-") + FString::FromInt(x) + TEXT(".txt"));
		}
	}

	FStringBuilderBase sb;

	auto EOL = TEXT("\n");

	sb << linePrefix << TEXT("Object ") << *obj->GetPathName() << EOL;
	sb << linePrefix << TEXT("Class ") << *obj->GetClass()->GetPathName() << EOL;

	for (auto cls = obj->GetClass()->GetSuperClass(); cls && cls != AActor::StaticClass(); cls = cls->GetSuperClass())
	{
		sb << linePrefix << TEXT("    - Super: ") << *cls->GetPathName() << EOL;
	}

	sb << linePrefix << TEXT("Properties") << EOL;

	for (TFieldIterator<FProperty> property(obj->GetClass()); property; ++property)
	{
		auto floatProperty = CastField<FFloatProperty>(*property);
		auto doubleProperty = CastField<FDoubleProperty>(*property);
		auto intProperty = CastField<FIntProperty>(*property);
		auto boolProperty = CastField<FBoolProperty>(*property);
		auto structProperty = CastField<FStructProperty>(*property);
		auto strProperty = CastField<FStrProperty>(*property);
		auto textProperty = CastField<FTextProperty>(*property);
		auto classProperty = CastField<FClassProperty>(*property);
		auto objectProperty = CastField<FObjectProperty>(*property);
		auto weakObjectProperty = CastField<FWeakObjectProperty>(*property);
		auto arrayProperty = CastField<FArrayProperty>(*property);
		auto byteProperty = CastField<FByteProperty>(*property);
		auto nameProperty = CastField<FNameProperty>(*property);
		auto enumProperty = CastField<FEnumProperty>(*property);

		FString cppType;
		// FString cppTypeForward;
		FString cppExtendedType;

		if (floatProperty ||
			doubleProperty ||
			intProperty ||
			boolProperty ||
			structProperty ||
			strProperty ||
			textProperty ||
			classProperty ||
			(objectProperty && objectProperty->PropertyClass) ||
			(weakObjectProperty && weakObjectProperty->PropertyClass) ||
			arrayProperty ||
			byteProperty ||
			nameProperty ||
			enumProperty
			)
		{
			cppType = property->GetCPPType(&cppExtendedType);
			// cppTypeForward = property->GetCPPTypeForwardDeclaration();
		}
		else
		{
			cppType = TEXT("<<Unknown>>");
		}

		sb << linePrefix <<
			TEXT("    - ") <<
			*property->GetName() <<
			TEXT(" (") <<
			*cppType <<
			*(!cppExtendedType.IsEmpty() ? cppExtendedType : TEXT("")) <<
			// *(!cppTypeForward.IsEmpty() ? TEXT(" / Forward = ") + cppTypeForward : TEXT("")) <<
			TEXT(" / ") <<
			*property->GetClass()->GetName() <<
			TEXT(" / RepIndex = ") <<
			property->RepIndex <<
			TEXT(")") <<
			EOL;

		if (floatProperty)
		{
			sb << linePrefix << TEXT("        = ") << *FText::AsNumber(floatProperty->GetPropertyValue_InContainer(obj)).ToString() << EOL;
		}
		else if (doubleProperty)
		{
			sb << linePrefix << TEXT("        = ") << *FText::AsNumber(doubleProperty->GetPropertyValue_InContainer(obj)).ToString() << EOL;
		}
		else if (intProperty)
		{
			sb << linePrefix << TEXT("        = ") << intProperty->GetPropertyValue_InContainer(obj) << EOL;
		}
		else if (byteProperty)
		{
			sb << linePrefix << TEXT("        = ") << byteProperty->GetPropertyValue_InContainer(obj) << EOL;
		}
		else if (boolProperty)
		{
			sb << linePrefix << TEXT("        = ") << (boolProperty->GetPropertyValue_InContainer(obj) ? TEXT("true") : TEXT("false")) << EOL;
		}
		else if (structProperty)
		{
			if (cppType == TEXT("FFactoryTickFunction"))
			{
				auto factoryTick = structProperty->ContainerPtrToValuePtr<FFactoryTickFunction>(obj);
				if (factoryTick)
				{
					sb << linePrefix << TEXT("        - Tick Interval = ") << FText::AsNumber(factoryTick->TickInterval).ToString() << EOL;
				}
			}
			else if (cppType == TEXT("FVector"))
			{
				auto vector = structProperty->ContainerPtrToValuePtr<FVector>(obj);
				if (vector)
				{
					sb << linePrefix << TEXT("        - = ") << *vector->ToString() << EOL;
				}
			}
			else if (cppType == TEXT("FRotator"))
			{
				auto rotator = structProperty->ContainerPtrToValuePtr<FRotator>(obj);
				if (rotator)
				{
					sb << linePrefix << TEXT("        - = ") << *rotator->ToString() << EOL;
				}
			}
		}
		else if (strProperty)
		{
			sb << linePrefix << TEXT("        = ") << *strProperty->GetPropertyValue_InContainer(obj) << EOL;
		}
		else if (textProperty)
		{
			sb << linePrefix << TEXT("        = ") << *textProperty->GetPropertyValue_InContainer(obj).ToString() << EOL;
		}
		else if (nameProperty)
		{
			sb << linePrefix << TEXT("        = ") << *nameProperty->GetPropertyValue_InContainer(obj).ToString() << EOL;
		}
		else if (classProperty)
		{
			sb << linePrefix << TEXT("        = ") << *GetNameSafe(classProperty->GetPropertyValue_InContainer(obj)) << EOL;
		}
		else if (objectProperty)
		{
			if (cppType == TEXT("TObjectPtr<USkeletalMesh>"))
			{
				auto skeletalMeshPtr = objectProperty->ContainerPtrToValuePtr<TObjectPtr<USkeletalMesh>>(obj);
				if (skeletalMeshPtr)
				{
					auto skeletalMesh = *skeletalMeshPtr;

					sb << linePrefix << TEXT("        = ") << *GetPathNameSafe(skeletalMesh.Get()) << EOL;
				}
			}
			else if (cppType == TEXT("UStaticMesh*"))
			{
				auto staticMeshPtr = objectProperty->ContainerPtrToValuePtr<UStaticMesh*>(obj);
				auto staticMesh = GetValid(*staticMeshPtr);
				if (staticMesh)
				{
					sb << linePrefix << TEXT("        = ") << *GetPathNameSafe(staticMesh) << EOL;
				}
			}
			else if (cppType == TEXT("UFGFactoryConnectionComponent*"))
			{
				auto connectionComponentPtr = objectProperty->ContainerPtrToValuePtr<UFGFactoryConnectionComponent*>(obj);
				auto connectionComponent = GetValid(*connectionComponentPtr);
				if (connectionComponent)
				{
					sb << linePrefix << TEXT("        = ") << *GetPathNameSafe(connectionComponent) << EOL;
					sb << linePrefix << TEXT("            - Relative Transform = ") << *connectionComponent->GetRelativeTransform().ToString() << EOL;
					sb << linePrefix << TEXT("            - Is Connected = ") << (connectionComponent->IsConnected() ? TEXT("true") : TEXT("false")) << EOL;
					sb << linePrefix << TEXT("            - Connection = ") << *GetPathNameSafe(connectionComponent->GetConnection()) << EOL;
					sb << linePrefix << TEXT("            - Connection Owner = ") << *GetPathNameSafe(
						connectionComponent->GetConnection() ? connectionComponent->GetConnection()->GetOwner() : nullptr
						) << EOL;
				}
			}
			else if (cppType == TEXT("UWidgetComponent*"))
			{
				auto widgetComponentPtr = objectProperty->ContainerPtrToValuePtr<UWidgetComponent*>(obj);
				auto widgetComponent = GetValid(*widgetComponentPtr);
				if (widgetComponent)
				{
					sb << linePrefix << TEXT("            - ") << *GetPathNameSafe(widgetComponent->GetClass()) << EOL;
				}
			}
			else
			{
				UObject* objValue = GetValid(objectProperty->GetObjectPropertyValue_InContainer(obj));

				if (objValue)
				{
					sb << linePrefix << TEXT("            - ") << *GetPathNameSafe(objValue) << EOL;
				}
			}
		}
		else if (arrayProperty)
		{
			FScriptArrayHelper arrayHelper(arrayProperty, arrayProperty->ContainerPtrToValuePtr<void>(obj));

			auto innerCppType = arrayProperty->Inner->GetCPPType();

			// sb << linePrefix << TEXT("        - CPPTypeForwardDeclaration = ") << arrayProperty->GetCPPTypeForwardDeclaration() << EOL;
			sb << linePrefix << TEXT("        - Num = ") << arrayHelper.Num() << EOL;
			sb << linePrefix << TEXT("        - Inner Type = ") << *innerCppType << EOL;

			if (innerCppType == TEXT("FSkeletalMaterial"))
			{
				auto array = arrayProperty->ContainerPtrToValuePtr<TArray<FSkeletalMaterial>>(obj);

				if (array)
				{
					for (const auto& x : *array)
					{
						sb << linePrefix <<
							TEXT("            - Material Slot Name = ") <<
							x.MaterialSlotName << TEXT(" / Material Interface = ") <<
							GetFullNameSafe(x.MaterialInterface) <<
							EOL;
					}
				}
			}
			else if (auto arrayObjectProperty = CastField<FObjectProperty>(arrayProperty->Inner))
			{
				for (auto x = 0; x < arrayHelper.Num(); x++)
				{
					void* ObjectContainer = arrayHelper.GetRawPtr(x);
					auto Object = arrayObjectProperty->GetObjectPropertyValue(ObjectContainer);
					sb << linePrefix <<
						TEXT("            - ") <<
						x <<
						TEXT(" = ") <<
						*GetPathNameSafe(Object) <<
						TEXT(" (") <<
						*GetPathNameSafe(Object ? Object->GetClass() : nullptr) <<
						TEXT(" )") <<
						EOL;
				}
			}
			else if (auto arrayWeakObjectProperty = CastField<FWeakObjectProperty>(arrayProperty->Inner))
			{
				for (auto x = 0; x < arrayHelper.Num(); x++)
				{
					void* ObjectContainer = arrayHelper.GetRawPtr(x);
					auto Object = arrayWeakObjectProperty->GetObjectPropertyValue(ObjectContainer);

					sb << linePrefix <<
						TEXT("            - ") <<
						x <<
						TEXT(" = ") <<
						*GetPathNameSafe(Object) <<
						TEXT(" (") <<
						*GetPathNameSafe(Object ? Object->GetClass() : nullptr) <<
						TEXT(" )") <<
						EOL;
				}
			}
		}
		else if (enumProperty)
		{
			const auto objReflection = UBlueprintReflectionLibrary::ReflectObject(obj);

			const auto reflectedValue = objReflection.GetEnumProperty(FName(property->GetName()));

			auto currentValue = reflectedValue.GetCurrentValue();
			auto enumType = enumProperty->GetEnum();

			sb << linePrefix <<
				TEXT("        = ") <<
				currentValue <<
				TEXT(" (") <<
				*(enumType ? enumType->GetNameByValue(currentValue) : NAME_None).ToString() <<
				TEXT(")") <<
				EOL;
		}
	}

	sb << linePrefix << TEXT("====") << EOL;

	FFileHelper::SaveStringToFile(sb.ToString(), *dumpFile, FFileHelper::EEncodingOptions::ForceUTF8);
}

class AActor* UMarcioCommonLibsUtils::GetHitActor(const FHitResult& hit)
{
	auto actor = hit.GetActor();

	auto abstractInstanceManger = Cast<AAbstractInstanceManager>(hit.GetActor());
	if (abstractInstanceManger)
	{
		FInstanceHandle handle;
		if (abstractInstanceManger->ResolveHit(hit, handle))
		{
			actor = AAbstractInstanceManager::GetOwnerByHandle(handle);
		}
	}

	return actor;
}

void UMarcioCommonLibsUtils::DumpInformation(AActor* worldContext, TSubclassOf<UFGEquipmentDescriptor> itemDescriptorClass)
{
	if (!IsValid(worldContext) || !itemDescriptorClass)
	{
		return;
	}

	UWorld* World = worldContext->GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	// if (configuration.dumpConnections)
	{
		auto className = GetFullNameSafe(itemDescriptorClass);

		MCL_LOG_Display(TEXT("Dumping "), *className);

		MCL_LOG_Display(TEXT("    Equipment small icon = "), *GetPathNameSafe(UFGEquipmentDescriptor::GetSmallIcon(itemDescriptorClass)));
		MCL_LOG_Display(TEXT("    Equipment big icon = "), *GetPathNameSafe(UFGEquipmentDescriptor::GetBigIcon(itemDescriptorClass)));
		MCL_LOG_Display(TEXT("    Equipment conveyor mesh = "), *GetPathNameSafe(UFGEquipmentDescriptor::GetItemMesh(itemDescriptorClass)));
		MCL_LOG_Display(TEXT("    Equipment category = "), *GetPathNameSafe(UFGEquipmentDescriptor::GetCategory(itemDescriptorClass)));

		auto equipmentDescriptorClass = TSubclassOf<UFGEquipmentDescriptor>(itemDescriptorClass);
		if (!equipmentDescriptorClass)
		{
			MCL_LOG_Display(TEXT("    Class "), *className, TEXT(" is not an Equipment Descriptor"));

			return;
		}

		MCL_LOG_Display(TEXT("    Equipment stack size = "), *getEnumItemName(StaticEnum<EStackSize>(), static_cast<int32>(UFGEquipmentDescriptor::GetStackSize(equipmentDescriptorClass))));

		const TSubclassOf<AFGEquipment> EquipmentClass = UFGEquipmentDescriptor::GetEquipmentClass(equipmentDescriptorClass);
		MCL_LOG_Display(TEXT("    Equipment class = "), *GetPathNameSafe(EquipmentClass.Get()));

		if (!EquipmentClass)
		{
			return;
		}

		AFGEquipment* equipment = World->SpawnActor<AFGEquipment>(EquipmentClass.Get());
		if (!IsValid(equipment))
		{
			MCL_LOG_Warning(TEXT("Unable to spawn equipment for "), *className);
			return;
		}

		MCL_LOG_Display(TEXT("    Equipment slot = "), *getEnumItemName(StaticEnum<EEquipmentSlot>(), static_cast<int32>(equipment->mEquipmentSlot)));
		//MCL_LOG_Display(TEXT("    Equipment attachment socket = "), *equipment->mAttachSocket.ToString());
		MCL_LOG_Display(TEXT("    Equipment arm animation = "), *getEnumItemName(StaticEnum<EArmEquipment>(), static_cast<int32>(equipment->GetArmsAnimation())));
		MCL_LOG_Display(TEXT("    Equipment back animation = "), *getEnumItemName(StaticEnum<EBackEquipment>(), static_cast<int32>(equipment->GetBackAnimation())));
		MCL_LOG_Display(TEXT("    Equipment idle pose animation = "), *GetPathNameSafe(equipment->GetIdlePoseAnimation()));
		MCL_LOG_Display(TEXT("    Equipment idle pose animation 3p = "), *GetPathNameSafe(equipment->GetIdlePoseAnimation3p()));
		MCL_LOG_Display(TEXT("    Equipment crouch pose animation 3p = "), *GetPathNameSafe(equipment->GetCrouchPoseAnimation3p()));
		MCL_LOG_Display(TEXT("    Equipment slide pose animation 3p = "), *GetPathNameSafe(equipment->GetSlidePoseAnimation3p()));
		MCL_LOG_Display(TEXT("    Equipment attachmend idle AO = "), *GetPathNameSafe(equipment->GetAttachmentIdleAO()));

		auto& components = equipment->GetComponents();

		// DUMP IT ALL!
		DumpUnknownClass(
			equipment,
			TEXT(""),
			TEXT(""),
			TEXT(""),
			false
			);

		for (auto component : components)
		{
			if (!IsValid(component))
			{
				continue;
			}

			MCL_LOG_Display(TEXT("    Component Class = "), *GetFullNameSafe(component->GetClass()));

			if (auto scene = Cast<USceneComponent>(component))
			{
				MCL_LOG_Display(TEXT("        Location = "), *scene->GetComponentLocation().ToString());
				MCL_LOG_Display(TEXT("        Rotation = "), *scene->GetComponentRotation().ToString());
				MCL_LOG_Display(TEXT("        Scale = "), *scene->GetComponentScale().ToString());
			}

			if (auto skeletalMesh = Cast<USkeletalMeshComponent>(component))
			{
				MCL_LOG_Display(TEXT("        Animation Mode = "), *getEnumItemName(StaticEnum<EAnimationMode::Type>(), skeletalMesh->GetAnimationMode()));
				auto animClass = skeletalMesh->GetAnimClass();
				MCL_LOG_Display(TEXT("        Anim Class = "), *GetPathNameSafe(animClass));
				if (animClass)
				{
					MCL_LOG_Display(TEXT("        Anim Base Class = "), *GetPathNameSafe(animClass->GetSuperClass()));
				}
				MCL_LOG_Display(TEXT("        Disable Post Process Blueprint = "), skeletalMesh->GetDisablePostProcessBlueprint() ? TEXT("true") : TEXT("false"));
				MCL_LOG_Display(TEXT("        Global Anim Rate Scale = "), skeletalMesh->GlobalAnimRateScale);
				MCL_LOG_Display(TEXT("        Pause Anims = "), skeletalMesh->bPauseAnims ? TEXT("true") : TEXT("false"));
				MCL_LOG_Display(TEXT("        Use Ref Pose On Init Anim = "), skeletalMesh->bUseRefPoseOnInitAnim ? TEXT("true") : TEXT("false"));
				MCL_LOG_Display(TEXT("        Skeletal Mesh = "), *GetPathNameSafe(skeletalMesh->GetSkinnedAsset()));
				if (USkinnedAsset* SkinnedAsset = skeletalMesh->GetSkinnedAsset())
				{
					DumpUnknownClass(
						SkinnedAsset,
						TEXT("        "),
						GetNameSafe(itemDescriptorClass) + TEXT("-"),
						TEXT("-") + component->GetName(),
						false
						);
				}
				MCL_LOG_Display(TEXT("        Position = "), skeletalMesh->GetPosition());
			}

			DumpUnknownClass(
				component,
				TEXT("        "),
				GetNameSafe(itemDescriptorClass) + TEXT("-"),
				TEXT("-") + component->GetName(),
				false
				);
		}

		equipment->Destroy();

		MCL_LOG_Display(TEXT(" Dump done"));
		MCL_LOG_Display(TEXT("===="));
	}
}

AFGCharacterPlayer* UMarcioCommonLibsUtils::GetFGPlayer(UWidget* widget)
{
	if (!IsValid(widget))
	{
		return nullptr;
	}

	APlayerController* owningPlayer = widget->GetOwningPlayer();
	if (!IsValid(owningPlayer))
	{
		return nullptr;
	}

	return Cast<AFGCharacterPlayer>(owningPlayer->K2_GetPawn());
}

int32 UMarcioCommonLibsUtils::getIndexFromName(const FString& name)
{
	static const FRegexPattern IndexPattern(TEXT("(\\d+)$"));
	FRegexMatcher m(IndexPattern, name);
	if (m.FindNext())
	{
		return FCString::Atoi(*m.GetCaptureGroup(1));
	}

	return INDEX_NONE;
}

FString UMarcioCommonLibsUtils::getEnumItemName(UEnum* MyEnum, int32 value)
{
	FString valueStr;

	if (MyEnum)
	{
		valueStr = MyEnum->GetDisplayNameTextByValue(value).ToString();
	}
	else
	{
		valueStr = TEXT("(Unknown)");
	}

	return FString::Printf(TEXT("%s (%d)"), *valueStr, value);
}

AFGBuildableTrainPlatform* UMarcioCommonLibsUtils::getNthTrainPlatform(AFGBuildableRailroadStation* station, int32 index)
{
	if (!IsValid(station) || index == 0)
	{
		return nullptr;
	}

	for (int32 DirectionIndex = 0; DirectionIndex <= 1; ++DirectionIndex)
	{
		int32 offsetDistance = 1;

		TSet<AFGBuildableTrainPlatform*> seenPlatforms;

		UFGTrainPlatformConnection* platformConnection = station->GetStationOutputConnection();
		if (!IsValid(platformConnection))
		{
			continue;
		}

		if (DirectionIndex)
		{
			platformConnection = station->GetConnectionInOppositeDirection(platformConnection);
		}

		if (!IsValid(platformConnection))
		{
			continue;
		}

		platformConnection = platformConnection->GetConnectedTo();
		while (IsValid(platformConnection))
		{
			AFGBuildableTrainPlatform* connectedPlatform = platformConnection->GetPlatformOwner();

			if (!IsValid(connectedPlatform) || seenPlatforms.Contains(connectedPlatform))
			{
				break;
			}

			seenPlatforms.Add(connectedPlatform);

			const int32 adjustedOffsetDistance = DirectionIndex ? -offsetDistance : offsetDistance;

			if (index == adjustedOffsetDistance)
			{
				return connectedPlatform;
			}

			UFGTrainPlatformConnection* OppositeConnection = connectedPlatform->GetConnectionInOppositeDirection(platformConnection);
			platformConnection = IsValid(OppositeConnection) ? OppositeConnection->GetConnectedTo() : nullptr;
			++offsetDistance;
		}
	}

	return nullptr;
}

void UMarcioCommonLibsUtils::getTrainPlatformIndexes(AFGBuildableTrainPlatform* trainPlatformCargo, TSet<int32>& indexes, TSet<AFGBuildableRailroadStation*>& destinationStations)
{
	if (!IsValid(trainPlatformCargo))
	{
		return;
	}

	TInlineComponentArray<UFGTrainPlatformConnection*> railComponents;
	trainPlatformCargo->GetComponents(railComponents);
	if (railComponents.Num() < 2)
	{
		return;
	}

	for (int32 DirectionIndex = 0; DirectionIndex <= 1; ++DirectionIndex)
	{
		int32 offsetDistance = 1;

		TSet<AFGBuildableTrainPlatform*> seenPlatforms;

		UFGTrainPlatformConnection* InitialConnection = railComponents[DirectionIndex];
		if (!IsValid(InitialConnection))
		{
			continue;
		}

		UFGTrainPlatformConnection* platformConnection = InitialConnection->GetConnectedTo();
		while (IsValid(platformConnection))
		{
			AFGBuildableTrainPlatform* connectedPlatform = platformConnection->GetPlatformOwner();

			if (!IsValid(connectedPlatform) || seenPlatforms.Contains(connectedPlatform))
			{
				break;
			}

			seenPlatforms.Add(connectedPlatform);

			MCL_LOG_Display_Condition(
				*connectedPlatform->GetName(),
				TEXT(" direction = "),
				DirectionIndex,
				TEXT(" / orientation reversed = "),
				connectedPlatform->IsOrientationReversed() ? TEXT("true") : TEXT("false")
				);

			if (AFGBuildableRailroadStation* station = Cast<AFGBuildableRailroadStation>(connectedPlatform))
			{
				destinationStations.Add(station);

				if (AFGTrainStationIdentifier* StationIdentifier = station->GetStationIdentifier())
				{
					MCL_LOG_Display_Condition(
						TEXT("    Station = "),
						*StationIdentifier->GetStationName().ToString()
						);
				}

				if (platformConnection == station->GetStationOutputConnection())
				{
					indexes.Add(offsetDistance);
					MCL_LOG_Display_Condition(TEXT("        offset distance = "), offsetDistance);
				}
				else
				{
					indexes.Add(-offsetDistance);
					MCL_LOG_Display_Condition(TEXT("        offset distance = "), -offsetDistance);
				}
			}

			UFGTrainPlatformConnection* OppositeConnection = connectedPlatform->GetConnectionInOppositeDirection(platformConnection);
			platformConnection = IsValid(OppositeConnection) ? OppositeConnection->GetConnectedTo() : nullptr;
			++offsetDistance;
		}
	}
}


#ifndef OPTIMIZE
UE_ENABLE_OPTIMIZATION_SHIP
#endif
