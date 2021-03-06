# Network-Learn

![license](https://img.shields.io/badge/license-MIT-blue.svg?style=flat-square)
![lines](https://tokei.rs/b1/github/ho-229/Network-Learn)
![windows](https://github.com/ho229v3666/Network-Learn/workflows/Windows/badge.svg?style=flat-square)
![linux](https://github.com/ho229v3666/Network-Learn/workflows/Linux/badge.svg?style=flat-square)  
A cross-platform network learning ~~demos~~(toys). And I try ~~not~~ to use 3rd-party libraries.  
Welcome to try it out and leave your comments.

| Name | Description |
| ---- | ----------- |
| [TinyWebServer](./TinyWebServer) | A tiny `Http/Https` web server |

## Build

Use `cmake` to build your project.  
Require `C++17` support and `OpenSSL`.

* Install OpenSSL 1.1.

  * Windows(run in `PowerShell`)

    ```shell
    choco install openssl
    ```

  * MacOS

    ```shell
    brew install openssl@1.1
    ```

* Configure and build

```shell
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

Of course, you can also use IDE to config and build.
