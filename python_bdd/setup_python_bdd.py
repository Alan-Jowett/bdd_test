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
    """Check if Python version is compatible"""
    if sys.version_info < (3, 8):
        print("Error: Python 3.8 or newer is required")
        return False
    print(f"✓ Python {sys.version.split()[0]} detected")
    return True

def install_requirements():
    """Install requirements from requirements.txt"""
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
    """Verify that the dd library is properly installed"""
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
    """Main setup function"""
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