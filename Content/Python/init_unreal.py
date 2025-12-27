"""
Pathify - Bootstrap loader
This file is auto-loaded by Unreal Engine on startup.
The actual logic lives in pathify.py to avoid module naming conflicts.
"""
import unreal

# Import and initialize the main module
try:
    import pathify
    pathify.init_menu()
    unreal.log("Pathify: Plugin loaded.")
except Exception as e:
    unreal.log_error(f"Pathify: Failed to initialize - {e}")
