#!/usr/bin/env python3
"""
Create an archive of the project at the state of the last commit.
Excludes untracked files and uses git archive functionality.
"""

import os
import sys
import subprocess
import argparse
from datetime import datetime

def get_project_name():
    """Extract project name from git repository."""
    try:
        result = subprocess.run(
            ['git', 'rev-parse', '--show-toplevel'],
            capture_output=True,
            text=True,
            check=True
        )
        repo_path = result.stdout.strip()
        return os.path.basename(repo_path)
    except subprocess.CalledProcessError:
        return "project"

def create_archive(output_path=None):
    """Create archive of the project at last commit state."""

    # Set default output path
    if output_path is None:
        output_path = ".."

    # Get project name for archive filename
    project_name = get_project_name()
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    archive_name = f"{project_name}_commit_{timestamp}.zip"

    # Construct full output path
    full_output_path = os.path.join(output_path, archive_name)

    try:
        # Create archive using git archive
        subprocess.run(
            ['git', 'archive', '--format=zip', '--output', full_output_path, 'HEAD'],
            check=True
        )

        print(f"Archive created successfully: {full_output_path}")
        return True

    except subprocess.CalledProcessError as e:
        print(f"Error creating archive: {e}", file=sys.stderr)
        return False

def main():
    parser = argparse.ArgumentParser(
        description='Create an archive of the project at the state of the last commit'
    )
    parser.add_argument(
        '--output', '-o',
        dest='output_path',
        help='Path where to put the archive (default: parent directory)'
    )

    args = parser.parse_args()

    success = create_archive(args.output_path)
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()