# Patches for Dependencies

This directory contains patches that are automatically applied to third-party dependencies during the CMake configuration process.

## Patch Naming Convention

Patches use git's standard format-patch naming convention:
- `NNNN-Description-of-patch.patch` where NNNN is a 4-digit sequence number
- Files are applied in lexicographical order based on filename
- This ensures deterministic patch application order

## TeDDy Memory Leak Fix

**File**: `0001-Fix-memory-leak-in-unique_table-rehash-function.patch`
**Target**: TeDDy v4.1.0 (DecisionDiagrams library)
**Issue**: Memory leak in `unique_table<Data, Degree>::rehash` function

### Problem Description
The TeDDy library v4.1.0 has a memory leak in the hash table rehashing function. When the hash table is resized, the old bucket array (`oldBuckets`) is not freed, causing a memory leak.

### Fix Applied
The patch adds a missing `std::free(oldBuckets);` call after the rehashing operation is complete.

**Location**: `libteddy/details/hash_tables.hpp`, line ~627

**Before**:
```cpp
    };

#ifdef LIBTEDDY_VERBOSE
    debug::out(", load after ", this->get_load_factor(), "\n");
#endif
```

**After**:
```cpp
    };

    // Fix memory leak: free the old buckets array
    std::free(oldBuckets);

#ifdef LIBTEDDY_VERBOSE
    debug::out(", load after ", this->get_load_factor(), "\n");
#endif
```

### How Patches Are Applied
Patches are automatically applied during CMake configuration using:
1. `FetchContent_MakeAvailable(DecisionDiagrams)` downloads TeDDy v4.1.0
2. CMake scans this directory for `*.patch` files
3. Patches are applied in alphabetical order using `git apply`
4. Build proceeds with the patched version

### Creating New Patches
To create additional patches for dependencies:
1. Make changes to the fetched source code in `<build-directory>/_deps/decisiondiagrams-src/`
2. Use `git add` and `git commit` to commit changes
3. Use `git format-patch -1 HEAD` to create a .patch file
4. Rename the patch with appropriate sequence number (e.g., `0002-Description.patch`)
5. Place the patch file in this directory
6. The patch will be automatically applied on next CMake configuration

### Patch Ordering
- Use 4-digit sequence numbers: `0001-`, `0002-`, `0003-`, etc.
- Apply critical fixes with lower numbers (applied first)
- Feature additions or minor fixes can use higher numbers
- CMake applies patches in lexicographical filename order