# blurifier

Educational-purpose image blurifier.

## Usage

1. Provider a jpeg file named _image.jpg_ in your current working directory.
2. Execute the binary (blurifier-jpeg). It will generate a _image-blurred.jpg_ file.
3. You're done!

## Limitations

Currently only jpeg images are supported using `libjpeg`. So, it inherits the limitations of `libjpeg`.

## Development

You'll need CMake and a C compiler. I used CMake version `3.19.6` and clang version `11.1.0`. If you want to use another compiler, set its path in `CMakeLists.txt` file (`CMAKE_C_COMPILER` configuration variable).
