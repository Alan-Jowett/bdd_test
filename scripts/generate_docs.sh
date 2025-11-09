#!/bin/bash

# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: Copyright (c) 2025 Alan Jowett

# Local Documentation Generation Script
# This script mirrors the GitHub Actions workflow for local testing

set -e

echo "🔧 Local Documentation Generation"
echo "================================="

# Check if Doxygen is available
if ! command -v doxygen &> /dev/null; then
    echo "❌ Error: Doxygen is not installed or not in PATH"
    echo "   Please install Doxygen to generate documentation"
    echo "   - Ubuntu/Debian: sudo apt-get install doxygen graphviz"
    echo "   - macOS: brew install doxygen graphviz"
    echo "   - Windows: Download from doxygen.org"
    exit 1
fi

echo "✅ Found Doxygen version: $(doxygen --version)"

# Get version information (similar to GitHub Actions)
if git describe --tags --exact-match >/dev/null 2>&1; then
    VERSION=$(git describe --tags --exact-match)
    echo "📦 Building documentation for tag: $VERSION"
else
    VERSION=$(git branch --show-current 2>/dev/null || echo "unknown")
    echo "📦 Building documentation for branch: $VERSION"
fi

# Create a temporary Doxyfile with updated version
echo "🔄 Updating Doxyfile with version: $VERSION"
sed "s/PROJECT_NUMBER.*=.*/PROJECT_NUMBER = $VERSION/" Doxyfile > Doxyfile.tmp

# Generate documentation
echo "📚 Generating documentation..."
doxygen Doxyfile.tmp

# Cleanup
rm Doxyfile.tmp

# Verify output
if [ ! -d "docs/html" ]; then
    echo "❌ Error: Documentation generation failed - no docs/html directory found"
    exit 1
fi

if [ ! -f "docs/html/index.html" ]; then
    echo "❌ Error: No index.html generated"
    exit 1
fi

# Add .nojekyll for GitHub Pages compatibility
touch docs/html/.nojekyll

echo "✅ Documentation generated successfully!"
echo "📁 Output directory: docs/html/"
echo "🌐 Open docs/html/index.html in your browser"

# Try to open documentation (optional)
if command -v xdg-open &> /dev/null; then
    echo "🚀 Opening documentation..."
    xdg-open docs/html/index.html
elif command -v open &> /dev/null; then
    echo "🚀 Opening documentation..."
    open docs/html/index.html
elif command -v start &> /dev/null; then
    echo "🚀 Opening documentation..."
    start docs/html/index.html
else
    echo "💡 Manually open docs/html/index.html to view the documentation"
fi

echo "✨ Done!"