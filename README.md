# Sensor Board Protocol

The Sensor Board Protocol is a small library that implements the message format
and related encoders/decoders for the sensor board.

## Building for Zephyr

To build with Zephyr, simply include the repository in the west manifest:

```yaml
projects:
  - name: lyng-senbrd-protocol
    remote: url/to/remote
    revision: main
    path: path/to/local/checkout
```

And the Zephyr build system will automatically pick up the module.

## Building for other applications

To build for non-Zephyr applications, add to the top level CMakeLists.txt or
build as a static library for manual inclusion into other projects.

To build with CMake and GCC:

```bash
mkdir build && cd build
cmake ..
make
```

For debug builds, add the `-DCMAKE_BUILD_TYPE=Debug` flag to the cmake call.
For release builds, add the `-DCMAKE_BUILD_TYPE=Release` flag to the cmake call.
