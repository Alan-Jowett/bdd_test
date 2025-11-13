#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

"""
Setup script for Python BDD Demo dependencies

This script checks for and installs the required Python dependencies
for the Python BDD comparison functionality.
"""

import subprocess
import sys
from pathlib import Path

def check_python_version():
    """Check if Python version meets minimum requirements.

    Verifies that the current Python installation is version 3.8 or newer,
    which is required for the 'dd' library and modern Python features
    used in the BDD demonstration scripts.

    Returns:
        bool: True if Python version is compatible, False otherwise

    Side Effects:
        Prints version information or error messages to stdout
    """
    if sys.version_info < (3, 8):
        print("Error: Python 3.8 or newer is required")
        return False
    print(f"✓ Python {sys.version.split()[0]} detected")
    return True

def install_requirements():
    """Install Python dependencies from requirements.txt.

    Reads the requirements.txt file in the same directory and installs
    all specified packages using pip. This ensures the 'dd' library
    and other dependencies are available for BDD operations.

    Returns:
        bool: True if installation succeeded, False if any errors occurred

    Side Effects:
        - Installs packages to the current Python environment
        - Prints installation progress and results to stdout
        - May modify the Python package environment

    Raises:
        subprocess.CalledProcessError: If pip installation fails
    """
    requirements_file = Path(__file__).parent / "requirements.txt"

    if not requirements_file.exists():
        print(f"Error: requirements.txt not found at {requirements_file}")
        return False

    print("Installing Python BDD dependencies...")
    try:
        result = subprocess.run([
            sys.executable, "-m", "pip", "install", "-r", str(requirements_file)
        ], check=True, capture_output=True, text=True)

        print("✓ Dependencies installed successfully")
        return True

    except subprocess.CalledProcessError as e:
        print(f"Error installing dependencies: {e}")
        print("Output:", e.stdout)
        print("Error:", e.stderr)
        return False

def verify_installation():
    """Verify that the 'dd' library is properly installed and functional.

    Attempts to import the 'dd' library and create a basic BDD instance
    to ensure the installation is working correctly. This validation
    catches common installation issues before running BDD demonstrations.

    Returns:
        bool: True if 'dd' library is working correctly, False otherwise

    Side Effects:
        Prints verification status and version information to stdout

    Note:
        This function performs a minimal functionality test by creating
        a simple BDD to verify the library is fully operational.
    """
    try:
        import dd
        print(f"✓ 'dd' library version {dd.__version__} installed and working")

        # Test basic BDD functionality
        from dd.autoref import BDD
        bdd = BDD()
        bdd.declare('x')
        expr = bdd.add_expr('x')
        assert expr is not None
        print("✓ BDD functionality test passed")

        return True

    except ImportError as e:
        print(f"Error: Failed to import 'dd' library: {e}")
        return False
    except Exception as e:
        print(f"Error: BDD functionality test failed: {e}")
        return False

def main():
    """Main setup function that orchestrates the complete Python BDD environment setup.

    Coordinates the entire setup process including Python version checking,
    dependency installation, and verification. Provides user feedback at each
    step and returns appropriate exit codes for automation.

    Returns:
        int: Exit code (0 for success, 1 for failure) suitable for shell scripts
            and automated build processes.

    Side Effects:
        - Prints progress messages to stdout for user feedback
        - May install Python packages to the current environment
        - May prompt user for confirmation depending on pip behavior
    """
    print("Python BDD Demo Setup")
    print("===================")
    print()

    if not check_python_version():
        return 1

    print()
    if not install_requirements():
        return 1

    print()
    if not verify_installation():
        print()
        print("Installation verification failed. Try running:")
        print("  pip install --upgrade dd")
        return 1

    print()
    print("🎉 Setup completed successfully!")
    print()
    print("You can now run Python BDD comparisons:")
    print("  python python_bdd/bdd_demo_python.py test_expressions/simple_expression.txt")
    print()
    print("Or enable Python tests in CMake:")
    print("  cmake -DENABLE_PYTHON_BDD=ON ..")
    print()

    return 0

if __name__ == "__main__":
    sys.exit(main())
