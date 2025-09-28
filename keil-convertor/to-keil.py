#!/usr/bin/env python3
import os
import xml.etree.ElementTree as ET
from xml.dom import minidom
import shutil

# --- Configuration ---
# Paths to your template files
UVPROJX_TEMPLATE = "template.uvprojx"
UVOPTX_TEMPLATE = "template.uvoptx"

# Your project's root directory
PROJECT_ROOT = ".."

# Directory to store the generated Keil project files
KEIL_PROJECT_DIR = "../keil-ver"

# Output Keil project name
OUTPUT_PROJECT_NAME = "keil_stm32f407ighx"

# Target Device (Must match Keil's internal naming exactly)
TARGET_DEVICE = "STM32F407IGHx"

# Preprocessor definitions to add
DEFINES = []

# Files to exclude from the project
EXCLUDE_FILES = [
    "stm32f4xx_fmc.c",
    "stm32f4xx_fmc.h",
]

# Define the mapping from your filesystem structure to Keil Groups
GROUP_CONFIG = [
    ("CORE", ["./core/CMSIS", "./startup-mdkarm"], [".c", ".h", ".s"], True),
    ("APP", ["./src/app"], [".c", ".h"], False),
    ("BSP", ["./src/bsp"], [".c", ".h"], False),
    ("UTILS", ["./src/utils"], [".c", ".h"], False),
    ("MOD", ["./src/mod"], [".c", ".h"], False),
    ("ERR", ["./src/err"], [".c", ".h"], False),
    ("USER", ["./src"], [".c", ".h"], False),  # Main files like main.c, it.c
]

INCLUDE_CONFIG = [
    "./src",
    "./lib",
    "./core/CMSIS/Device/ST/STM32F4xx/Include",
    "./core/CMSIS/Include",
]


# --- Helper Functions ---
def prettify_xml(elem):
    """Return a pretty-printed XML string for the Element."""
    rough_string = ET.tostring(elem, "utf-8")
    reparsed = minidom.parseString(rough_string)
    return reparsed.toprettyxml(indent="  ")


def find_files(base_dirs, extensions, recursive=True):
    """Find all files with given extensions in base_dirs, excluding specified files."""
    found_files = set()
    for base_dir in base_dirs:
        full_base_dir = os.path.join(PROJECT_ROOT, base_dir)
        if not os.path.exists(full_base_dir):
            print(f"Warning: Directory {full_base_dir} does not exist.")
            continue

        if recursive:
            for root, _, files in os.walk(full_base_dir):
                for file in files:
                    if os.path.basename(file) in EXCLUDE_FILES:
                        continue
                    if any(file.endswith(ext) for ext in extensions):
                        full_path = os.path.join(root, file)
                        # Make path relative to the Keil project's location
                        rel_path = os.path.relpath(full_path, KEIL_PROJECT_DIR).replace(
                            os.sep, "\\"
                        )
                        found_files.add(rel_path)
        else:
            for file in os.listdir(full_base_dir):
                if os.path.basename(file) in EXCLUDE_FILES:
                    continue
                full_path = os.path.join(full_base_dir, file)
                if os.path.isfile(full_path) and any(
                    file.endswith(ext) for ext in extensions
                ):
                    rel_path = os.path.relpath(full_path, KEIL_PROJECT_DIR).replace(
                        os.sep, "\\"
                    )
                    found_files.add(rel_path)
    return list(found_files)


def get_file_type(extension):
    """Map file extension to Keil's FileType."""
    if extension in [".c", ".cpp"]:
        return "1"
    if extension in [".s", ".asm"]:
        return "2"
    return "5"  # Default to Text/Document for headers, etc.


# --- Main Script ---
def main():
    # Create the output directory if it doesn't exist
    if not os.path.exists(KEIL_PROJECT_DIR):
        os.makedirs(KEIL_PROJECT_DIR)

    # Load the template .uvprojx
    tree_proj = ET.parse(UVPROJX_TEMPLATE)
    root_proj = tree_proj.getroot()

    # Load the template .uvoptx
    tree_opt = ET.parse(UVOPTX_TEMPLATE)
    root_opt = tree_opt.getroot()

    # --- Modify .uvprojx ---
    target_option = root_proj.find(".//Targets/Target/TargetOption")
    if target_option is None:
        print("Error: <TargetOption> not found in .uvprojx template.")
        return

    common_option = target_option.find("TargetCommonOption")
    if common_option is None:
        print("Error: <TargetCommonOption> not found in .uvprojx template.")
        return

    # 1. Update Device Name
    device_elem = common_option.find("Device")
    if device_elem is not None:
        device_elem.text = TARGET_DEVICE

    # 2. Update Project Name
    target_name_elem = root_proj.find(".//Targets/Target/TargetName")
    if target_name_elem is not None:
        target_name_elem.text = OUTPUT_PROJECT_NAME
    output_name_elem = common_option.find("OutputName")
    if output_name_elem is not None:
        output_name_elem.text = OUTPUT_PROJECT_NAME

    # 3. Set Build Output Path
    output_dir_elem = target_option.find(
        "TargetArmAds/ArmAdsMisc/AdsCpuFile/TargetCommonOption/OutputDirectory"
    )
    if output_dir_elem is not None:
        output_dir_elem.text = ".\\build\\"
    else:  # Fallback for simpler structures
        output_dir_elem = common_option.find("OutputDirectory")
        if output_dir_elem is not None:
            output_dir_elem.text = ".\\build\\"

    # 4. Add Defines (HSE_VALUE)
    cads = target_option.find("TargetArmAds/Cads")
    if cads is not None:
        defines_elem = cads.find("VariousControls/Define")
        if defines_elem is not None:
            existing_defines = defines_elem.text.split(",") if defines_elem.text else []
            existing_defines.extend(DEFINES)
            defines_elem.text = ", ".join(sorted(list(set(existing_defines))))

        # 6. Update Include Paths
        include_path_elem = cads.find("VariousControls/IncludePath")
        if include_path_elem is not None:
            include_dirs = set()
            # Add directories from GROUP_CONFIG
            for base_dir in INCLUDE_CONFIG:
                # Normalize the path and add the directory
                full_path = os.path.join(PROJECT_ROOT, base_dir)
                rel_path = os.path.relpath(full_path, KEIL_PROJECT_DIR)
                include_dirs.add(rel_path.replace(os.sep, "\\"))

            # Sort for consistent output
            sorted_paths = sorted(list(include_dirs))
            include_path_elem.text = ";".join(sorted_paths)

    # 5. Clear and rebuild file groups
    groups_elem = root_proj.find(".//Targets/Target/Groups")
    if groups_elem is not None:
        groups_elem.clear()
    else:
        print("Error: <Groups> element not found in .uvprojx template.")
        return

    for group_name, base_dirs, extensions, recursive in GROUP_CONFIG:
        files_in_group = find_files(base_dirs, extensions, recursive)
        if not files_in_group:
            continue

        new_group = ET.SubElement(groups_elem, "Group")
        ET.SubElement(new_group, "GroupName").text = group_name
        files_elem = ET.SubElement(new_group, "Files")

        for file_path in sorted(files_in_group):
            _, ext = os.path.splitext(file_path)
            file_elem = ET.SubElement(files_elem, "File")
            # Extract just the filename part for FileName element
            # Handle both Windows and Linux path separators
            if "\\" in file_path:
                basename = file_path.split("\\")[-1]
            else:
                basename = os.path.basename(file_path)
            # Debug: Print the file_path and basename to see what's happening
            # print(f"DEBUG: file_path: '{file_path}', basename: '{basename}'")
            ET.SubElement(file_elem, "FileName").text = basename
            ET.SubElement(file_elem, "FileType").text = get_file_type(ext)
            ET.SubElement(file_elem, "FilePath").text = file_path

    # --- Modify .uvoptx ---
    target_name_opt_elem = root_opt.find(".//Target/TargetName")
    if target_name_opt_elem is not None:
        target_name_opt_elem.text = OUTPUT_PROJECT_NAME

    # Clear existing groups in .uvoptx
    for group_opt in root_opt.findall("./Group"):
        root_opt.remove(group_opt)

    # --- Write new files ---
    output_uvprojx = os.path.join(KEIL_PROJECT_DIR, f"{OUTPUT_PROJECT_NAME}.uvprojx")
    output_uvoptx = os.path.join(KEIL_PROJECT_DIR, f"{OUTPUT_PROJECT_NAME}.uvoptx")

    # Use minidom for pretty printing the final XML
    proj_xml_str = ET.tostring(root_proj, "utf-8")
    proj_reparsed = minidom.parseString(proj_xml_str)
    # Fix for cross-platform compatibility
    with open(output_uvprojx, "w", encoding="utf-8", newline="\r\n") as f:
        # Remove extra newlines that minidom adds
        content = proj_reparsed.toprettyxml(indent="  ")
        content = "\n".join([line for line in content.split("\n") if line.strip()])
        f.write(content)
        # Ensure file has proper Windows line endings
        if f.tell() > 0 and content[-1] != "\n":
            f.write("\r\n")

    opt_xml_str = ET.tostring(root_opt, "utf-8")
    opt_reparsed = minidom.parseString(opt_xml_str)
    # Fix for cross-platform compatibility
    with open(output_uvoptx, "w", encoding="utf-8", newline="\r\n") as f:
        # Remove extra newlines that minidom adds
        content = opt_reparsed.toprettyxml(indent="  ")
        content = "\n".join([line for line in content.split("\n") if line.strip()])
        f.write(content)
        # Ensure file has proper Windows line endings
        if f.tell() > 0 and content[-1] != "\n":
            f.write("\r\n")

    # Set file permissions for cross-platform compatibility
    try:
        os.chmod(output_uvprojx, 0o666)  # rw-rw-rw- for Windows compatibility
        os.chmod(output_uvoptx, 0o666)  # rw-rw-rw- for Windows compatibility
    except OSError:
        # If chmod fails (e.g., on Windows), just continue
        pass

    print(f"Keil project files generated in: '{KEIL_PROJECT_DIR}'")


if __name__ == "__main__":
    main()
