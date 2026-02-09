import sys
import json
import os
import argparse

def update_settings(workspace_root, import_paths):
    vscode_dir = os.path.join(workspace_root, ".vscode")
    if not os.path.exists(vscode_dir):
        os.makedirs(vscode_dir)
    
    settings_path = os.path.join(vscode_dir, "settings.json")
    
    data = {}
    if os.path.exists(settings_path):
        try:
            with open(settings_path, 'r', encoding='utf-8') as f:
                content = f.read().strip()
                if content:
                    data = json.loads(content)
        except Exception as e:
            print(f"[Warning] Failed to parse existing settings.json: {e}")
            # Ensure we start fresh if parse fails, or backup? 
            # For now, let's just proceed with empty dict if corrupt
    
    # Update paths
    # We use a set to avoid duplicates, but json needs list
    # Actually, CMake passes the full list we want, so usually we can just overwrite these specific keys
    # to ensure they are insync with the build.
    
    paths_list = import_paths.split(',')
    # Filter empty strings
    paths_list = [p for p in paths_list if p]
    
    data["qt.qml.importPaths"] = paths_list
    data["qml.importPaths"] = paths_list
    
    try:
        with open(settings_path, 'w', encoding='utf-8') as f:
            json.dump(data, f, indent=4)
        print(f"[Info] Updated VSCode settings at {settings_path}")
    except Exception as e:
        print(f"[Error] Failed to write settings.json: {e}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", required=True, help="Workspace root directory")
    parser.add_argument("--paths", required=True, help="Comma separated import paths")
    args = parser.parse_args()
    
    update_settings(args.root, args.paths)
