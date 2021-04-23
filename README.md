# blurifier

Educational-purpose image blurifier.

## Usage

1. Provider a jpeg file named _image.jpg_ in your current working directory.
2. Execute the binary (blurifier). It will generate a _image-blurred.jpg_ file.
3. You're done!

## Configuration

You can change the default configuration parameters in `config.h`.

Each one is defined as following:

```c
// Results in a ((2 * KERNEL_RADIUS + 1) * (2 * KERNEL_RADIUS + 1)) kernel window
const uint8_t KERNEL_RADIUS = 5u;

// Input image file name. You can set file path, too.
// Remember the basics!
const char *INPUT_FILENAME = "image.jpg";

// Output image file name. You can set file path, too.
// Remember the basics!
const char *OUTPUT_FILENAME = "image-blurred-mean-5.jpg";
```

## Limitations

Currently only RGB-colored jpeg input/output images are supported using `libjpeg`. So, it inherits the limitations of `libjpeg`.

## Development

You'll need CMake and a C compiler. I used CMake version `3.20.1` and clang version `11.1.0`. If you want to use another compiler, set its path in `CMakeLists.txt` file (`CMAKE_C_COMPILER` configuration variable).
