# Usage: ./run-macos.sh solution.cpp
set -xe
./compile.sh "$1" executable
DYLD_LIBRARY_PATH=. ./executable
