# Usage: ./compile.sh solution.cpp
set -xe
g++ -O2 -std=c++17 -Iinclude -Iinclude/crypto -Iinclude/tdutils -Iinclude/ton -Iinclude/common "$1" -L. -lton_crypto_lib -o $2
# g++ -g -pedantic -Wall -Wextra -Weffc++ -std=c++17 -Iinclude -Iinclude/crypto -Iinclude/tdutils -Iinclude/ton -Iinclude/common "$1" -L. -lton_crypto_lib -o $2
echo "#!/usr/bin/env bash\nDYLD_LIBRARY_PATH=. ./$2" > ./solution
chmod +x ./solution
