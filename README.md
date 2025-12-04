
![Pathify Banner](https://i.imgur.com/jNddfU4.png)

# Pathify for Unreal Engine

**Pathify** is a lightweight Unreal Engine plugin that automates the creation of folder structures within your project's Content Browser. It reads asset paths directly from your clipboard and instantly recreates the corresponding directory hierarchy in your project.

## Features

*   **Clipboard Integration**: Simply copy one or more asset paths (e.g., from Fmodel).
*   **Batch Processing**: Supports multiple paths separated by newlines.
*   **Smart Detection**: Automatically detects the `/Content/` directory in the path to determine the correct relative structure.
*   **Pure Python**: No C++ compilation required. Runs using Unreal's built-in Python scripting environment.

## Installation

1.  **Download**: Download this repository from the green Code button.
2.  **Copy**: Place the `Pathify` folder into your project's `Plugins` directory (create the `Plugins` folder in your project root if it doesn't exist).
    *   *Alternative*: To install globally for all projects, place it in `UE_5.x\Engine\Plugins\Marketplace`.
3.  **Enable Python**:
    *   Open your Unreal Project.
    *   Go to **Edit > Plugins**.
    *   Search for "Python Editor Script Plugin" and enable it.
    *   Restart the Editor if prompted.
    *   Search for "Pathify" in the Plugins list and make sure it's enabled.
    *   Restart the Editor if needed.
4.  **Verify**: After restart, look for the **Pathify** entry in the **Tools** menu.

## Usage

1.  **Copy Paths**: Copy a list of asset paths to your clipboard.
    *   *Example Input:*
        ```text
        D:\Work\MyGame\Content\Marvel\Characters\Hero\Textures\T_Hero_D.uasset
        Marvel/Content/Marvel/UI/Textures/Ability/1033/New
        Game/Content/Marvel/Characters/9999/9999000/Meshes/SK_HeroZero_9999000.uasset
        ```
2.  **Run Tool**: In the Unreal Editor, go to the top menu bar:
    *   **Tools** > **Pathify: Recreate Path (Clipboard)**
3.  **Result**: The plugin will parse the paths, find the folder structure relative to `Content`, and create any missing directories in your Content Browser.
4.  **Report**: A log message will appear in Editor's console summarizing how many folders were created, how many already existed, and if any paths failed.

## Requirements

*   Unreal Engine 5.0 or later (should work on 4.27+ with Python enabled).
*   **Python Editor Script Plugin** must be enabled in the project.

## License

MIT License. Free to use in personal and commercial projects.
