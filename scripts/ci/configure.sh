#!/usr/bin/env bash

((${BASH_VERSION%%.*} >= 4)) || { echo >&2 "$0: Error: Please upgrade Bash."; exit 1; }

set -euxo pipefail

conan --version # check it works

cmake_args=(
    -S .
    -B build
    -G "${AUDACITY_CMAKE_GENERATOR}"
    -D use_pch=no
    -D CMAKE_BUILD_TYPE="${AUDACITY_BUILD_TYPE}"
    -D CMAKE_INSTALL_PREFIX="${AUDACITY_INSTALL_PREFIX}"
)

if [[ "${AUDACITY_CMAKE_GENERATOR}" == "Visual Studio"* ]]; then
    cmake_args+=(
        # skip unneeded configurations
        -D CMAKE_CONFIGURATION_TYPES="${AUDACITY_BUILD_TYPE}"
    )
    case "${AUDACITY_ARCH_LABEL}" in
    32bit)  cmake_args+=( -A Win32 ) ;;
    64bit)  cmake_args+=( -A x64 ) ;;
    *)      echo >&2 "$0: Unrecognised arch label '${AUDACITY_ARCH_LABEL}'" ; exit 1 ;;
    esac
elif [[ "${AUDACITY_CMAKE_GENERATOR}" == Xcode* ]]; then
    cmake_args+=(
        # skip unneeded configurations
        -D CMAKE_CONFIGURATION_TYPES="${AUDACITY_BUILD_TYPE}"
        -T buildsystem=1
    )
fi

if [[ -n "${APPLE_CODESIGN_IDENTITY}" && "${OSTYPE}" == darwin* ]]; then
    cmake_args+=(
        -D APPLE_CODESIGN_IDENTITY="${APPLE_CODESIGN_IDENTITY}"
        -D perform_codesign=yes
    )

    if [[ ${GIT_BRANCH} == release* ]]; then
        cmake_args+=(
            -D APPLE_NOTARIZATION_USER_NAME="${APPLE_NOTARIZATION_USER_NAME}"
            -D APPLE_NOTARIZATION_PASSWORD="${APPLE_NOTARIZATION_PASSWORD}"
            -D perform_notarization=yes
        )
    fi
elif [[ -n "${WINDOWS_CERTIFICATE}" && "${OSTYPE}" == msys* ]]; then
    # Windows certificate will be used from the environment
    cmake_args+=(
        -D perform_codesign=yes
    )
fi

# Use Homebrew's libmad instead because our submodule fails to build on macOS.
# Tenacity's libmad
if [[ "${OSTYPE}" == darwin* ]]; then
    # We'll be using the system's libmad by default
    cmake_args+=(
        -D saucedacity_use_libmad=off
    )
fi

if [[ ${GIT_BRANCH} == release* ]]; then
    cmake_args+=(
        -D package_manual=yes
    )
fi

# Configure Audacity
cmake "${cmake_args[@]}"

# Remove build directories and sources to reduce the cache size.
conan remove "*" --src --builds --force
