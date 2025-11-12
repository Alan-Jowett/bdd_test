# Example of using fetch_and_patch with multiple projects

# Include the FetchAndPatch module
include(cmake/FetchAndPatch.cmake)

# Example 1: Fetch TeDDy library with patches using a Git tag
fetch_and_patch(DecisionDiagrams
    GIT_REPOSITORY https://github.com/MichalMrena/DecisionDiagrams.git
    GIT_TAG v4.1.0
    PATCHES_DIR ${CMAKE_SOURCE_DIR}/patches/teddy
)

# Example 2: Fetch another project with patches using a Git branch (hypothetical CUDD library)
# Note: This is commented out as it's just an example
# fetch_and_patch(CUDD
#     GIT_REPOSITORY https://github.com/ivmai/cudd.git
#     GIT_BRANCH master
#     PATCHES_DIR ${CMAKE_SOURCE_DIR}/patches/cudd
#     PATCH_STRIP_LEVEL 1
# )

# Example 3: Fetch a project from main branch without patches (uses default patches directory)
# fetch_and_patch(SomeOtherProject
#     GIT_REPOSITORY https://github.com/example/project.git
#     GIT_BRANCH main
# )

# Example 4: Fetch a specific commit hash (treated as GIT_TAG)
# fetch_and_patch(ExampleProject
#     GIT_REPOSITORY https://github.com/example/project.git
#     GIT_TAG abc123def456789
#     PATCHES_DIR ${CMAKE_SOURCE_DIR}/patches/example
# )