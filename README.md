# Let

## Building

If you have a [GitHub account setup with SSH](https://docs.github.com/en/authentication/connecting-to-github-with-ssh), just do this:
```sh
git clone --recurse-submodules git@github.com:leissa/let.git
```
Otherwise, clone via HTTPS:
```sh
git clone --recurse-submodules https://github.com/leissa/let.git
```
Then, build with:
```sh
cd let
mkdir build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j $(nproc)
```
For a `Release` build simply use `-DCMAKE_BUILD_TYPE=Release`.

Test:
```sh
 ./build/bin/let -d test/test.sql
 ```
