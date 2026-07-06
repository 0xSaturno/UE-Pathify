#include "UEtilitiesModule.h"
#include "ToolMenus.h"
#include "HAL/PlatformApplicationMisc.h"
#include "EditorAssetLibrary.h"
#include "Misc/Paths.h"
#include "Logging/LogMacros.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "NiagaraSystem.h"
#include "Factories/Factory.h"
#include "ContentBrowserMenuContexts.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/PrimaryAssetLabel.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Factories/MaterialFactoryNew.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"

DEFINE_LOG_CATEGORY_STATIC(LogUEtilities, Log, All);

#define LOCTEXT_NAMESPACE "FUEtilitiesModule"

void FUEtilitiesModule::StartupModule()
{
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FUEtilitiesModule::RegisterMenus));
}

void FUEtilitiesModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
}

void FUEtilitiesModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
	{
		FToolMenuSection& Section = Menu->FindOrAddSection("UEtilities");
		
		Section.AddMenuEntry(
			"UEtilities",
			LOCTEXT("UEtilitiesRecreatePathLabel", "UEtilities: Recreate Path (Clipboard)"),
			LOCTEXT("UEtilitiesRecreatePathTooltip", "Reads asset paths from clipboard and recreates folder structure in Content Browser"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &FUEtilitiesModule::PluginButtonClicked))
		);

		Section.AddMenuEntry(
			"UEtilitiesCreateNS",
			LOCTEXT("UEtilitiesCreateNSLabel", "UEtilities: Create Empty NS (Clipboard)"),
			LOCTEXT("UEtilitiesCreateNSTooltip", "Creates empty Niagara Systems from clipboard paths"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &FUEtilitiesModule::CreateNSButtonClicked))
		);

		Section.AddMenuEntry(
			"UEtilitiesCreateMaterial",
			LOCTEXT("UEtilitiesCreateMaterialLabel", "UEtilities: Create Empty Material (Clipboard)"),
			LOCTEXT("UEtilitiesCreateMaterialTooltip", "Creates empty Materials from clipboard paths"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &FUEtilitiesModule::CreateMaterialButtonClicked))
		);

		Section.AddMenuEntry(
			"UEtilitiesCreateMI",
			LOCTEXT("UEtilitiesCreateMILabel", "UEtilities: Create Empty Material Instance (Clipboard)"),
			LOCTEXT("UEtilitiesCreateMITooltip", "Creates empty Material Instances from clipboard paths"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &FUEtilitiesModule::CreateMIButtonClicked))
		);
	}

	UToolMenu* AssetMenu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AssetContextMenu");
	if (AssetMenu)
	{
		FToolMenuSection& Section = AssetMenu->FindOrAddSection("GetAssetActions");
		Section.AddSubMenu(
			"UEtilitiesDataAsset",
			LOCTEXT("AddToDataAssetSubMenuLabel", "Add To PrimaryAssetLabel"),
			LOCTEXT("AddToDataAssetSubMenuTooltip", "Adds selected assets to a PrimaryAssetLabel DataAsset"),
			FNewToolMenuDelegate::CreateRaw(this, &FUEtilitiesModule::PopulateDataAssetSubMenu)
		);
	}
}

void FUEtilitiesModule::PluginButtonClicked()
{
	FString ClipboardText;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardText);

	if (ClipboardText.IsEmpty())
	{
		UE_LOG(LogUEtilities, Warning, TEXT("Clipboard is empty or does not contain text."));
		return;
	}

	TArray<FString> Lines;
	ClipboardText.ParseIntoArrayLines(Lines);

	TArray<FString> Paths;
	for (FString& Line : Lines)
	{
		Line.TrimStartAndEndInline();
		Line = Line.TrimQuotes();
		if (!Line.IsEmpty())
		{
			Paths.Add(Line);
		}
	}

	if (Paths.Num() == 0)
	{
		UE_LOG(LogUEtilities, Log, TEXT("No valid paths found in clipboard."));
		return;
	}

	UE_LOG(LogUEtilities, Log, TEXT("Found %d path(s) in clipboard."), Paths.Num());

	int32 CreatedCount = 0;
	int32 ExistingCount = 0;
	int32 FailedCount = 0;
	TArray<FString> Details;

	for (const FString& AssetPath : Paths)
	{
		FString NormalizedPath = AssetPath.Replace(TEXT("\\"), TEXT("/"));
		
		FString ContentToken = TEXT("/Content/");
		int32 Index = NormalizedPath.Find(ContentToken);
		
		if (Index == INDEX_NONE)
		{
			UE_LOG(LogUEtilities, Warning, TEXT("Skipped invalid path (no /Content/): %s"), *AssetPath);
			FailedCount++;
			Details.Add(FString::Printf(TEXT("[FAIL] Invalid path: %s"), *AssetPath));
			continue;
		}

		FString RelativePath = NormalizedPath.Mid(Index + ContentToken.Len());
		
		FString DirectoryPath;
		if (RelativePath.Contains(TEXT(".")))
		{
			DirectoryPath = FPaths::GetPath(RelativePath);
		}
		else
		{
			DirectoryPath = RelativePath;
		}

		if (DirectoryPath.IsEmpty())
		{
			UE_LOG(LogUEtilities, Warning, TEXT("Skipped (no dir structure): %s"), *AssetPath);
			FailedCount++;
			Details.Add(FString::Printf(TEXT("[FAIL] No structure: %s"), *AssetPath));
			continue;
		}

		FString GamePath = FString::Printf(TEXT("/Game/%s"), *DirectoryPath);

		if (UEditorAssetLibrary::DoesDirectoryExist(GamePath))
		{
			ExistingCount++;
		}
		else if (UEditorAssetLibrary::MakeDirectory(GamePath))
		{
			CreatedCount++;
			UE_LOG(LogUEtilities, Log, TEXT("Created %s"), *GamePath);
			Details.Add(FString::Printf(TEXT("[OK] Created: %s"), *GamePath));
		}
		else
		{
			FailedCount++;
			UE_LOG(LogUEtilities, Error, TEXT("Failed to create %s"), *GamePath);
			Details.Add(FString::Printf(TEXT("[FAIL] Error creating: %s"), *GamePath));
		}
	}

	FString SummaryMsg = FString::Printf(TEXT("Processed %d path(s). Created: %d, Existed: %d, Failed: %d"), Paths.Num(), CreatedCount, ExistingCount, FailedCount);
	
	if (FailedCount > 0)
	{
		UE_LOG(LogUEtilities, Error, TEXT("UEtilities Batch Report:"));
		for (const FString& Detail : Details)
		{
			UE_LOG(LogUEtilities, Error, TEXT("%s"), *Detail);
		}
		UE_LOG(LogUEtilities, Error, TEXT("%s"), *SummaryMsg);
	}
	else
	{
		UE_LOG(LogUEtilities, Log, TEXT("%s"), *SummaryMsg);
	}
}

void FUEtilitiesModule::CreateAssetsFromClipboard(UClass* AssetClass, UFactory* Factory, const FString& LogPrefix)
{
	FString ClipboardText;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardText);

	if (ClipboardText.IsEmpty())
	{
		UE_LOG(LogUEtilities, Warning, TEXT("%s: Clipboard is empty or does not contain text."), *LogPrefix);
		return;
	}

	TArray<FString> Lines;
	ClipboardText.ParseIntoArrayLines(Lines);

	TArray<FString> Paths;
	for (FString& Line : Lines)
	{
		Line.TrimStartAndEndInline();
		Line = Line.TrimQuotes();
		if (!Line.IsEmpty())
		{
			Paths.Add(Line);
		}
	}

	if (Paths.Num() == 0)
	{
		UE_LOG(LogUEtilities, Log, TEXT("%s: No valid paths found in clipboard."), *LogPrefix);
		return;
	}

	UE_LOG(LogUEtilities, Log, TEXT("%s: Processing %d path(s) for asset creation."), *LogPrefix, Paths.Num());

	int32 CreatedCount = 0;
	int32 ExistingCount = 0;
	int32 FailedCount = 0;
	TArray<FString> Details;

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

	for (const FString& AssetPath : Paths)
	{
		FString NormalizedPath = AssetPath.Replace(TEXT("\\"), TEXT("/"));
		
		FString ContentToken = TEXT("/Content/");
		int32 Index = NormalizedPath.Find(ContentToken);
		
		if (Index == INDEX_NONE)
		{
			UE_LOG(LogUEtilities, Warning, TEXT("%s: Skipped invalid path (no /Content/): %s"), *LogPrefix, *AssetPath);
			FailedCount++;
			Details.Add(FString::Printf(TEXT("[FAIL] Invalid path: %s"), *AssetPath));
			continue;
		}

		FString RelativePath = NormalizedPath.Mid(Index + ContentToken.Len());
		
		if (!RelativePath.Contains(TEXT(".")))
		{
			UE_LOG(LogUEtilities, Warning, TEXT("%s: Skipped (appears to be a folder): %s"), *LogPrefix, *AssetPath);
			FailedCount++;
			Details.Add(FString::Printf(TEXT("[FAIL] Not a file path: %s"), *AssetPath));
			continue;
		}

		FString DirectoryPath = FPaths::GetPath(RelativePath);
		FString Filename = FPaths::GetBaseFilename(RelativePath);
		
		FString GamePath = FString::Printf(TEXT("/Game/%s"), *DirectoryPath);

		if (!UEditorAssetLibrary::DoesDirectoryExist(GamePath))
		{
			if (UEditorAssetLibrary::MakeDirectory(GamePath))
			{
				UE_LOG(LogUEtilities, Log, TEXT("%s: Created folder %s"), *LogPrefix, *GamePath);
			}
			else
			{
				UE_LOG(LogUEtilities, Error, TEXT("%s: Failed to create folder %s"), *LogPrefix, *GamePath);
				FailedCount++;
				Details.Add(FString::Printf(TEXT("[FAIL] Folder creation error: %s"), *GamePath));
				continue;
			}
		}

		FString FinalAssetPath = FString::Printf(TEXT("%s/%s.%s"), *GamePath, *Filename, *Filename);
		
		if (UEditorAssetLibrary::DoesAssetExist(FinalAssetPath))
		{
			ExistingCount++;
			Details.Add(FString::Printf(TEXT("[SKIP] Exists: %s"), *FinalAssetPath));
		}
		else
		{
			UObject* NewAsset = AssetToolsModule.Get().CreateAsset(Filename, GamePath, AssetClass, Factory);
			if (NewAsset)
			{
				CreatedCount++;
				FString UniqueName = NewAsset->GetPathName();
				UE_LOG(LogUEtilities, Log, TEXT("%s: Created Asset %s"), *LogPrefix, *UniqueName);
				Details.Add(FString::Printf(TEXT("[OK] Created: %s"), *UniqueName));
			}
			else
			{
				FailedCount++;
				UE_LOG(LogUEtilities, Error, TEXT("%s: Failed to create asset %s"), *LogPrefix, *FinalAssetPath);
				Details.Add(FString::Printf(TEXT("[FAIL] Asset creation error: %s"), *FinalAssetPath));
			}
		}
	}

	FString SummaryMsg = FString::Printf(TEXT("%s Batch: Processed %d. Created: %d, Existed: %d, Failed: %d"), *LogPrefix, Paths.Num(), CreatedCount, ExistingCount, FailedCount);
	
	if (FailedCount > 0)
	{
		UE_LOG(LogUEtilities, Error, TEXT("%s Batch Report:"), *LogPrefix);
		for (const FString& Detail : Details)
		{
			UE_LOG(LogUEtilities, Error, TEXT("%s"), *Detail);
		}
		UE_LOG(LogUEtilities, Error, TEXT("%s"), *SummaryMsg);
	}
	else
	{
		UE_LOG(LogUEtilities, Log, TEXT("%s"), *SummaryMsg);
	}
}

void FUEtilitiesModule::CreateNSButtonClicked()
{
	UClass* NSClass = LoadClass<UObject>(nullptr, TEXT("/Script/Niagara.NiagaraSystem"));
	if (!NSClass)
	{
		UE_LOG(LogUEtilities, Error, TEXT("Could not find UNiagaraSystem class!"));
		return;
	}

	UClass* FactoryClass = LoadClass<UFactory>(nullptr, TEXT("/Script/NiagaraEditor.NiagaraSystemFactoryNew"));
	if (!FactoryClass)
	{
		UE_LOG(LogUEtilities, Error, TEXT("Could not find UNiagaraSystemFactoryNew class!"));
		return;
	}

	UFactory* NSFactory = NewObject<UFactory>(GetTransientPackage(), FactoryClass);
	CreateAssetsFromClipboard(NSClass, NSFactory, TEXT("UEtilities(NS)"));
}

void FUEtilitiesModule::CreateMaterialButtonClicked()
{
	UMaterialFactoryNew* MatFactory = NewObject<UMaterialFactoryNew>();
	CreateAssetsFromClipboard(UMaterial::StaticClass(), MatFactory, TEXT("UEtilities(Mat)"));
}

void FUEtilitiesModule::CreateMIButtonClicked()
{
	UMaterialInstanceConstantFactoryNew* MIFactory = NewObject<UMaterialInstanceConstantFactoryNew>();
	CreateAssetsFromClipboard(UMaterialInstanceConstant::StaticClass(), MIFactory, TEXT("UEtilities(MI)"));
}

void FUEtilitiesModule::PopulateDataAssetSubMenu(UToolMenu* Menu)
{
	UContentBrowserAssetContextMenuContext* Context = Menu->FindContext<UContentBrowserAssetContextMenuContext>();
	if (!Context || Context->SelectedAssets.Num() == 0)
	{
		return;
	}

	TArray<FAssetData> SelectedAssets = Context->SelectedAssets;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetDataList;
	
	FARFilter Filter;
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	Filter.ClassPaths.Add(FTopLevelAssetPath(TEXT("/Script/Engine"), TEXT("PrimaryAssetLabel")));
#else
	Filter.ClassNames.Add(TEXT("PrimaryAssetLabel"));
#endif
	AssetRegistryModule.Get().GetAssets(Filter, AssetDataList);

	FToolMenuSection& Section = Menu->AddSection("PrimaryAssetLabels", LOCTEXT("PrimaryAssetLabelsHeading", "PrimaryAssetLabels"));

	for (const FAssetData& AssetData : AssetDataList)
	{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
		FString ObjectPath = AssetData.GetObjectPathString();
#else
		FString ObjectPath = AssetData.ObjectPath.ToString();
#endif
		FString AssetName = AssetData.AssetName.ToString();

		Section.AddMenuEntry(
			FName(*AssetName),
			FText::FromString(AssetName),
			FText::FromString(ObjectPath),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this, SelectedAssets, ObjectPath]()
			{
				ExecuteAddToDataAsset(SelectedAssets, ObjectPath);
			}))
		);
	}
}

void FUEtilitiesModule::ExecuteAddToDataAsset(TArray<FAssetData> SelectedAssets, FString TargetDataAssetPath)
{
	UPrimaryAssetLabel* LabelAsset = Cast<UPrimaryAssetLabel>(StaticLoadObject(UPrimaryAssetLabel::StaticClass(), nullptr, *TargetDataAssetPath));
	if (!LabelAsset)
	{
		UE_LOG(LogUEtilities, Error, TEXT("Failed to load DataAsset: %s"), *TargetDataAssetPath);
		return;
	}

	bool bModified = false;
	for (const FAssetData& SelectedAsset : SelectedAssets)
	{
		TSoftObjectPtr<UObject> SoftPtr(SelectedAsset.ToSoftObjectPath());
		if (!LabelAsset->ExplicitAssets.Contains(SoftPtr))
		{
			int32 EmptyIndex = LabelAsset->ExplicitAssets.IndexOfByPredicate([](const TSoftObjectPtr<UObject>& Ptr) { return Ptr.IsNull(); });
			if (EmptyIndex != INDEX_NONE)
			{
				LabelAsset->ExplicitAssets[EmptyIndex] = SoftPtr;
			}
			else
			{
				LabelAsset->ExplicitAssets.Add(SoftPtr);
			}
			bModified = true;
		}
	}

	if (bModified)
	{
		LabelAsset->Modify();
		UE_LOG(LogUEtilities, Log, TEXT("Added %d asset(s) to %s"), SelectedAssets.Num(), *TargetDataAssetPath);
	}
	else
	{
		UE_LOG(LogUEtilities, Log, TEXT("No new assets added to %s (they may already be in the array)."), *TargetDataAssetPath);
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUEtilitiesModule, UEtilities)
