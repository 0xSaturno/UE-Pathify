#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FUEtilitiesModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterMenus();
	void PluginButtonClicked();
	void CreateNSButtonClicked();
	void CreateMaterialButtonClicked();
	void CreateMIButtonClicked();

	void CreateAssetsFromClipboard(UClass* AssetClass, UFactory* Factory, const FString& LogPrefix);

	void PopulateDataAssetSubMenu(UToolMenu* Menu);
	void ExecuteAddToDataAsset(TArray<FAssetData> SelectedAssets, FString TargetDataAssetPath);
};
