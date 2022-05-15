# lfuda-c
LFU with Dynamic Aging in-memory cache implementation in C.

# 1. About

This library implements LFU and LFUDA caching algorithms.

# 2. What is LFUDA?

LFUDA tries to combine the best of LFU and LRU, while avoiding the pitfalls of both cache eviction policies. Unlike regular LFU-Aging, 
LFUDA is parameterless, which removes the complexitiy of tuning the parameters.

# 3. Compilation

## 1. Compilation

### Linux
```sh
# Clone the repository
git clone https://github.com/serjzimmerman/lfuda-c

cmake -S ./ -B build/
cd build/

# Compile all targets
make all

# To run all tests
ctest
```