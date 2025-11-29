import unreal
import sys
import os
import subprocess

def get_clipboard_text():
    """
    Retrieves text from the Windows clipboard using PowerShell.
    This is safer than ctypes as it runs in a separate process.
    """
    try:
        # Use PowerShell to get clipboard content
        # -NoProfile: Skips loading user profile (much faster)
        # -NonInteractive: Prevents interactive prompts
        cmd = ["powershell", "-NoProfile", "-NonInteractive", "-Command", "Get-Clipboard"]
        
        # Create startup info to hide the console window completely
        startupinfo = subprocess.STARTUPINFO()
        startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
        
        # CREATE_NO_WINDOW = 0x08000000
        result = subprocess.check_output(cmd, universal_newlines=True, startupinfo=startupinfo, creationflags=0x08000000)
        return result.strip()
    except Exception as e:
        unreal.log_error(f"Pathify: Failed to read clipboard via PowerShell: {e}")
        return None

def recreate_path_from_clipboard():
    """
    Reads path(s) from clipboard and recreates structure.
    Supports multiple paths separated by newlines.
    """
    clipboard_text = get_clipboard_text()
    
    if not clipboard_text:
        msg = "Clipboard is empty or does not contain text."
        unreal.log_warning(f"Pathify: {msg}")
        return

    # Split by lines to support batch processing
    lines = clipboard_text.splitlines()
    
    # Filter out empty lines
    paths = [line.strip().strip('"').strip("'") for line in lines if line.strip()]
    
    if not paths:
        msg = "No valid paths found in clipboard."
        unreal.log(f"Pathify: {msg}")
        return

    unreal.log(f"Pathify: Found {len(paths)} path(s) in clipboard.")

    created_count = 0
    existing_count = 0
    failed_count = 0
    details = []

    for asset_path in paths:
        # Logic to recreate path (inline or helper)
        normalized_path = asset_path.replace("\\", "/")
        
        # Find "Content" to determine root
        content_token = "/Content/"
        index = normalized_path.find(content_token)
        
        if index == -1:
            unreal.log_warning(f"Pathify: Skipped invalid path (no /Content/): {asset_path}")
            failed_count += 1
            details.append(f"[FAIL] Invalid path: {asset_path}")
            continue
            
        # Extract relative path after Content
        relative_path = normalized_path[index + len(content_token):]
        
        # Remove filename if present
        if "." in os.path.basename(relative_path):
            directory_path = os.path.dirname(relative_path)
        else:
            directory_path = relative_path
            
        if not directory_path:
            unreal.log_warning(f"Pathify: Skipped (no dir structure): {asset_path}")
            failed_count += 1
            details.append(f"[FAIL] No structure: {asset_path}")
            continue
            
        # Construct full game path
        game_path = f"/Game/{directory_path}"
        
        # Create directory
        if unreal.EditorAssetLibrary.does_directory_exist(game_path):
             existing_count += 1
        elif unreal.EditorAssetLibrary.make_directory(game_path):
            created_count += 1
            unreal.log(f"Pathify: Created {game_path}")
            details.append(f"[OK] Created: {game_path}")
        else:
            failed_count += 1
            unreal.log_error(f"Pathify: Failed to create {game_path}")
            details.append(f"[FAIL] Error creating: {game_path}")

    # Summary
    summary_msg = f"Processed {len(paths)} path(s). Created: {created_count}, Existed: {existing_count}, Failed: {failed_count}"
    
    if failed_count > 0:
        unreal.log_error("Pathify Batch Report:\n" + "\n".join(details))
        unreal.log_error(summary_msg)
    else:
        unreal.log(summary_msg)

def init_menu():
    menus = unreal.ToolMenus.get()
    main_menu = menus.find_menu("LevelEditor.MainMenu.Tools")
    if not main_menu:
        return

    entry = unreal.ToolMenuEntry(
        name="Pathify",
        type=unreal.MultiBlockType.MENU_ENTRY,
        insert_position=unreal.ToolMenuInsert("", unreal.ToolMenuInsertType.DEFAULT)
    )
    
    entry.set_label("Pathify: Recreate Path (Clipboard)")
    entry.set_tool_tip("Reads asset paths from clipboard and recreates folder structure")
    
    entry.set_string_command(
        type=unreal.ToolMenuStringCommandType.PYTHON,
        custom_type="Pathify",
        string="import init_unreal; init_unreal.recreate_path_from_clipboard()" 
    )
    
    main_menu.add_menu_entry("Tools", entry)
    menus.refresh_all_widgets()

if __name__ == "__main__":
    init_menu()
    unreal.log("Pathify: Plugin loaded.")
