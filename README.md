## Build

```bash
cmake -S . -B build
cmake --build build
```

Use the following commands to build with dynamically linked libraries.
```bash
cmake -S . -B build -DUNICODE_NORM_LIBRARY_STATIC_LINK=OFF
cmake --build build
```

## Install

```bash
sudo mv build/unicode_norm /usr/local/bin/
```


## Uninstall

```bash
sudo rm /usr/local/bin/unicode_norm
```

