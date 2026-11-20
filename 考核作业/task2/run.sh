#!/usr/bin/env bash
set -Eeuo pipefail

# Resolve the project directory from this script, not from the caller's cwd.
PROJECT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${BUILD_DIR:-$PROJECT_DIR/build}"
TARGET="task2"
EXECUTABLE="$BUILD_DIR/$TARGET"

show_help() {
    cat <<'EOF'
Usage:
  ./run.sh build
  ./run.sh run
  ./run.sh run -- arg1 arg2
  ./run.sh clean
  ./run.sh help

Commands:
  build    Configure and build the C++ project
  run      Build if needed, then run the program
  clean    Remove the build directory
  help     Show this help message

Examples:
  ./run.sh build
  ./run.sh run
  ./run.sh run -- hello 123
EOF
}

error() {
    echo "Error: $*" >&2
}

check_cmake() {
    if ! command -v cmake >/dev/null 2>&1; then
        error "CMake is not installed. Install it with: sudo apt install cmake"
        return 1
    fi
}

configure_project() {
    check_cmake || return 1

    if [[ ! -f "$PROJECT_DIR/CMakeLists.txt" ]]; then
        error "CMakeLists.txt not found in: $PROJECT_DIR"
        return 1
    fi

    cmake -S "$PROJECT_DIR" -B "$BUILD_DIR"
}

sources_are_newer_than_executable() {
    [[ ! -x "$EXECUTABLE" ]] && return 0

    # Check source/header/CMake files, excluding the generated build directory.
    local newer
    newer="$(find "$PROJECT_DIR" \
        -path "$BUILD_DIR" -prune -o \
        -type f \( -name '*.c' -o -name '*.cc' -o -name '*.cpp' \
                    -o -name '*.h' -o -name '*.hh' -o -name '*.hpp' \
                    -o -name 'CMakeLists.txt' -o -name '*.cmake' \) \
        -newer "$EXECUTABLE" -print -quit)"

    [[ -n "$newer" ]]
}

build_project() {
    configure_project || {
        error "CMake configuration failed."
        return 1
    }

    if ! sources_are_newer_than_executable; then
        echo "Build is up to date. Skipping compilation."
        return 0
    fi

    echo "Building target: $TARGET"
    cmake --build "$BUILD_DIR" --target "$TARGET" || {
        error "Build failed."
        return 1
    }

    if [[ ! -x "$EXECUTABLE" ]]; then
        error "Build finished, but executable was not found: $EXECUTABLE"
        return 1
    fi

    echo "Build completed successfully."
}

run_project() {
    shift  # Remove the 'run' subcommand.

    # Optional '--' separates run.sh options from program arguments.
    if [[ "${1:-}" == "--" ]]; then
        shift
    fi

    build_project || return 1

    if [[ ! -x "$EXECUTABLE" ]]; then
        error "Executable is missing or not executable: $EXECUTABLE"
        return 1
    fi

    echo "Running $TARGET..."
    "$EXECUTABLE" "$@"
}

clean_project() {
    if [[ -d "$BUILD_DIR" ]]; then
        rm -rf -- "$BUILD_DIR"
        echo "Removed build directory: $BUILD_DIR"
    else
        echo "Nothing to clean."
    fi
}

case "${1:-help}" in
    build)
        [[ $# -eq 1 ]] || { error "Usage: ./run.sh build"; exit 2; }
        build_project
        ;;
    run)
        run_project "$@"
        ;;
    clean)
        [[ $# -eq 1 ]] || { error "Usage: ./run.sh clean"; exit 2; }
        clean_project
        ;;
    help|-h|--help)
        show_help
        ;;
    *)
        error "Unknown command: $1"
        show_help >&2
        exit 2
        ;;
esac
