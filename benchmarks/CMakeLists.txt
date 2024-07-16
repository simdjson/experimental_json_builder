CPMAddPackage("gh:nlohmann/json@3.10.5")

# Include reflect-cpp
CPMAddPackage(
  NAME reflect-cpp
  GITHUB_REPOSITORY getml/reflect-cpp
  PATCHES reflect-cpp.patch
  GIT_TAG v0.13.0
)
add_subdirectory(simpleparser)
add_subdirectory(src)