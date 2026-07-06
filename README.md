![UEtilities Banner](https://i.imgur.com/psTZRll.png)

# UEtilities for Unreal Engine

**UEtilities** is a lightweight Unreal Engine plugin that automates the creation of folder structures and empty assets within your project's Content Browser. It reads asset paths directly from your clipboard and instantly recreates the corresponding directory hierarchy or assets in your project, and provides quick actions for asset management.

## Features

*   **Clipboard Integration**: Simply copy one or more asset paths (e.g., from FModel).
*   **Batch Processing**: Supports multiple paths separated by newlines.
*   **Smart Detection**: Automatically detects the `/Content/` directory in the path to determine the correct relative structure.
*   **Recreate Paths**: Recreates folder structures based on copied paths.
*   **Create Empty Assets**: Automatically create empty Niagara Systems, Materials, or Material Instances from copied paths.
*   **PrimaryAssetLabel Integration**: Quickly add selected assets to any `PrimaryAssetLabel` DataAsset in the project directly from the Content Browser context menu.

## Installation

1.  **Download**: Download the latest compiled release zip from the Releases section.
2.  **Extract**: Extract the `UEtilities_Plugin` folder from the zip file.
3.  **Copy**: Place the `UEtilities_Plugin` folder into your project's `Plugins` directory (create the `Plugins` folder in your project root if it doesn't exist).
    *   *Alternative*: To install globally for all projects, place it in `UE_5.x\Engine\Plugins\Marketplace`.
4.  **Enable Plugin**:
    *   Open your Unreal Project.
    *   Go to **Edit > Plugins**.
    *   Search for "UEtilities" in the Plugins list and make sure it's enabled.
    *   Restart the Editor if needed.
5.  **Verify**: After restart, you should see the **UEtilities** actions under the **Tools** menu.

## Usage

### Recreating Paths & Assets
1.  **Copy Paths**: Copy a list of asset paths to your clipboard.
    *   *Example Input:*
        ```text
        D:\Work\MyGame\Content\Marvel\Characters\Hero\Textures\T_Hero_D.uasset
        Marvel/Content/Marvel/UI/Textures/Ability/1033/New
        Game/Content/Marvel/Characters/9999/9999000/Meshes/SK_HeroZero_9999000.uasset
        ```
2.  **Run Tool**: In the Unreal Editor, go to the top menu bar **Tools** and select one of the following:
    *   **UEtilities: Recreate Path (Clipboard)**
    *   **UEtilities: Create Empty NS (Clipboard)**
    *   **UEtilities: Create Empty Material (Clipboard)**
    *   **UEtilities: Create Empty Material Instance (Clipboard)**
3.  **Result**: The plugin will parse the paths, find the folder structure relative to `Content`, and create any missing directories or empty assets in your Content Browser.

### Adding to PrimaryAssetLabel
1.  **Select Assets**: Select one or more assets in the Content Browser.
2.  **Right-Click**: Open the context menu and select **Add To PrimaryAssetLabel**.
3.  **Select Label**: Choose the target `PrimaryAssetLabel` DataAsset from the list. The selected assets will be automatically assigned.

## Requirements

*   Unreal Engine 5.3 or later.
