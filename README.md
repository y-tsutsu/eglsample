# eglsample

## install library

```console
$ sudo apt install libgles2-mesa-dev libegl1-mesa-dev xorg-dev
```

## build

```console
$ cmake -S . -B build  # cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
$ cmake --build ./build
```
