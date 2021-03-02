#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <memory.h>
#include <jpeglib.h>

const uint8_t INPUT_IMAGE_COMPONENTS_NUMBER = 3u;
const uint8_t KERNEL_RADIUS = 2u; /* generates 5x5 kernel window */
const uint8_t KERNEL_WIDTH = KERNEL_RADIUS * 2 + 1;
const uint8_t KERNEL_HEIGHT = KERNEL_WIDTH;

void set_decompressor_options(
  struct jpeg_decompress_struct *decompressor,
  struct jpeg_error_mgr *error_manager,
  FILE *input_file
) {
  decompressor->err = jpeg_std_error(error_manager);
  jpeg_create_decompress(decompressor);
  jpeg_stdio_src(decompressor, input_file);
  jpeg_read_header(decompressor, TRUE);
  jpeg_start_decompress(decompressor);
}

void set_compressor_options(
  struct jpeg_compress_struct *compressor,
  const struct jpeg_decompress_struct *decompressor,
  struct jpeg_error_mgr *error_manager,
  FILE *output_file
) {
  compressor->err = jpeg_std_error(error_manager);
  jpeg_create_compress(compressor);
  jpeg_stdio_dest(compressor, output_file);
  compressor->in_color_space = JCS_RGB;
  compressor->jpeg_color_space = JCS_RGB;
  compressor->input_components = decompressor->num_components;
  compressor->num_components = decompressor->num_components;
  jpeg_set_defaults(compressor);
  compressor->image_width = decompressor->output_width;
  compressor->image_height = decompressor->image_height;
  compressor->density_unit = decompressor->density_unit;
  compressor->X_density = decompressor->X_density;
  compressor->Y_density = decompressor->Y_density;
  jpeg_start_compress(compressor, TRUE);
}

struct components {
  float red;
  float green;
  float blue;
};

int transform(const char *input_filename, const char *output_filename) {
  FILE *input_file = fopen(input_filename, "rb");
  if (!input_file) {
    fprintf(stderr, "🛑🙁 error opening input jpeg file '%s': %s 🙁🛑\n", input_filename, strerror(errno));
    return errno;
  }

  FILE *output_file = fopen(output_filename, "wb");
  if (!output_file) {
    fprintf(stderr, "🛑🙁 error opening output jpeg file '%s': %s 🙁🛑\n", output_filename, strerror(errno));
    return errno;
  }

  struct jpeg_error_mgr error_manager;

  struct jpeg_decompress_struct decompressor;
  set_decompressor_options(&decompressor, &error_manager, input_file);

  const uint16_t IMAGE_WIDTH = decompressor.image_width;
  const uint16_t IMAGE_HEIGHT = decompressor.image_height;

  struct jpeg_compress_struct compressor;
  set_compressor_options(&compressor, &decompressor, &error_manager, output_file);

  unsigned char *frame_free_space = malloc(KERNEL_HEIGHT * IMAGE_WIDTH * INPUT_IMAGE_COMPONENTS_NUMBER);
  const unsigned char kernel[5][5] = {
    { 1u, 2u, 4u, 2u, 1u },
    { 2u, 3u, 5u, 3u, 2u },
    { 4u, 5u, 6u, 5u, 4u },
    { 2u, 3u, 5u, 3u, 2u },
    { 1u, 2u, 4u, 2u, 1u },
  };

  JSAMPROW output_image_row_samples[] = { malloc(IMAGE_WIDTH * INPUT_IMAGE_COMPONENTS_NUMBER) };

  unsigned char *image_buffer[KERNEL_HEIGHT];
  for (size_t i = 0; i < KERNEL_HEIGHT; i++) {
    image_buffer[i] = &frame_free_space[i * IMAGE_WIDTH * INPUT_IMAGE_COMPONENTS_NUMBER];
  }

  const uint16_t IN_FRAME_HEIGHT = IMAGE_HEIGHT - KERNEL_RADIUS;
  const uint16_t IN_FRAME_WIDTH = IMAGE_WIDTH - KERNEL_RADIUS;

  for (size_t i = 0; i < KERNEL_RADIUS; i++) {
    jpeg_read_scanlines(&decompressor, &image_buffer[i], 1);
  }

  for (size_t i = 0; i < KERNEL_RADIUS; i++) {
    jpeg_read_scanlines(&decompressor, &image_buffer[i + KERNEL_RADIUS], 1);

    for (size_t j = 0; j < KERNEL_RADIUS; j++) {
      struct components multiplication_sum = {
        .red = 0.f,
        .green = 0.f,
        .blue = 0.f,
      };
      unsigned short int kernel_sum = 0u;
      for (size_t ki = KERNEL_RADIUS - i; ki < KERNEL_HEIGHT; ki++) {
        for (size_t kj = KERNEL_RADIUS - j; kj < KERNEL_WIDTH; kj++) {
          kernel_sum += kernel[ki][kj];
          const uint16_t red = image_buffer[i + ki - KERNEL_RADIUS][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 0];
          const uint16_t green = image_buffer[i + ki - KERNEL_RADIUS][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 1];
          const uint16_t blue = image_buffer[i + ki - KERNEL_RADIUS][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 2];
          multiplication_sum.red += red * kernel[ki][kj];
          multiplication_sum.green += green * kernel[ki][kj];
          multiplication_sum.blue += blue * kernel[ki][kj];
        }
      }
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 0] = multiplication_sum.red / kernel_sum;
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 1] = multiplication_sum.green / kernel_sum;
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 2] = multiplication_sum.blue / kernel_sum;
    }

    for (size_t j = KERNEL_RADIUS; j < IN_FRAME_WIDTH; j++) {
      struct components multiplication_sum = {
        .red = 0.f,
        .green = 0.f,
        .blue = 0.f,
      };
      unsigned short int kernel_sum = 0u;
      for (size_t ki = KERNEL_RADIUS - i; ki < KERNEL_HEIGHT; ki++) {
        for (size_t kj = 0; kj < KERNEL_WIDTH; kj++) {
          kernel_sum += kernel[ki][kj];
          const uint16_t red = image_buffer[i + ki - KERNEL_RADIUS][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 0];
          const uint16_t green = image_buffer[i + ki - KERNEL_RADIUS][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 1];
          const uint16_t blue = image_buffer[i + ki - KERNEL_RADIUS][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 2];
          multiplication_sum.red += red * kernel[ki][kj];
          multiplication_sum.green += green * kernel[ki][kj];
          multiplication_sum.blue += blue * kernel[ki][kj];
        }
      }
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 0] = multiplication_sum.red / kernel_sum;
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 1] = multiplication_sum.green / kernel_sum;
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 2] = multiplication_sum.blue / kernel_sum;
    }

    for (size_t j = IN_FRAME_WIDTH; j < IMAGE_WIDTH; j++) {
      struct components multiplication_sum = {
        .red = 0.f,
        .green = 0.f,
        .blue = 0.f,
      };
      unsigned short int kernel_sum = 0u;
      for (size_t ki = KERNEL_RADIUS - i; ki < KERNEL_HEIGHT; ki++) {
        for (size_t kj = 0; kj < KERNEL_RADIUS + IMAGE_WIDTH - j; kj++) {
          kernel_sum += kernel[ki][kj];
          const uint16_t red = image_buffer[i + ki - KERNEL_RADIUS][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 0];
          const uint16_t green = image_buffer[i + ki - KERNEL_RADIUS][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 1];
          const uint16_t blue = image_buffer[i + ki - KERNEL_RADIUS][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 2];
          multiplication_sum.red += red * kernel[ki][kj];
          multiplication_sum.green += green * kernel[ki][kj];
          multiplication_sum.blue += blue * kernel[ki][kj];
        }
      }
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 0] = multiplication_sum.red / kernel_sum;
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 1] = multiplication_sum.green / kernel_sum;
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 2] = multiplication_sum.blue / kernel_sum;
    }

    jpeg_write_scanlines(&compressor, output_image_row_samples, 1);
  }

  for (size_t i = KERNEL_RADIUS; i < IN_FRAME_HEIGHT; i++) {
    jpeg_read_scanlines(&decompressor, &image_buffer[(i + KERNEL_RADIUS) % KERNEL_HEIGHT], 1);

    for (size_t j = 0; j < KERNEL_RADIUS; j++) {
      struct components multiplication_sum = {
        .red = 0.f,
        .green = 0.f,
        .blue = 0.f,
      };
      unsigned short int kernel_sum = 0u;
      for (size_t ki = 0; ki < KERNEL_HEIGHT; ki++) {
        for (size_t kj = KERNEL_RADIUS - j; kj < KERNEL_WIDTH; kj++) {
          kernel_sum += kernel[ki][kj];
          const uint16_t red = image_buffer[(i - KERNEL_RADIUS + ki) % KERNEL_HEIGHT][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 0];
          const uint16_t green = image_buffer[(i - KERNEL_RADIUS + ki) % KERNEL_HEIGHT][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 1];
          const uint16_t blue = image_buffer[(i - KERNEL_RADIUS + ki) % KERNEL_HEIGHT][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 2];
          multiplication_sum.red += red * kernel[ki][kj];
          multiplication_sum.green += green * kernel[ki][kj];
          multiplication_sum.blue += blue * kernel[ki][kj];
        }
      }
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 0] = multiplication_sum.red / kernel_sum;
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 1] = multiplication_sum.green / kernel_sum;
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 2] = multiplication_sum.blue / kernel_sum;
    }

    for (size_t j = KERNEL_RADIUS; j < IN_FRAME_WIDTH; j++) {
      struct components multiplication_sum = {
        .red = 0.f,
        .green = 0.f,
        .blue = 0.f,
      };
      unsigned short int kernel_sum = 0u;
      for (size_t ki = 0; ki < KERNEL_HEIGHT; ki++) {
        for (size_t kj = 0; kj < KERNEL_WIDTH; kj++) {
          kernel_sum += kernel[ki][kj];
          const uint16_t red = image_buffer[(i - KERNEL_RADIUS + ki) % KERNEL_HEIGHT][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 0];
          const uint16_t green = image_buffer[(i - KERNEL_RADIUS + ki) % KERNEL_HEIGHT][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 1];
          const uint16_t blue = image_buffer[(i - KERNEL_RADIUS + ki) % KERNEL_HEIGHT][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 2];
          multiplication_sum.red += red * kernel[ki][kj];
          multiplication_sum.green += green * kernel[ki][kj];
          multiplication_sum.blue += blue * kernel[ki][kj];
        }
      }
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 0] = multiplication_sum.red / kernel_sum;
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 1] = multiplication_sum.green / kernel_sum;
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 2] = multiplication_sum.blue / kernel_sum;
    }

    for (size_t j = IN_FRAME_WIDTH; j < IMAGE_WIDTH; j++) {
      struct components multiplication_sum = {
        .red = 0.f,
        .green = 0.f,
        .blue = 0.f,
      };
      unsigned short int kernel_sum = 0u;
      for (size_t ki = 0; ki < KERNEL_HEIGHT; ki++) {
        for (size_t kj = 0; kj < KERNEL_RADIUS + IMAGE_WIDTH - j; kj++) {
          kernel_sum += kernel[ki][kj];
          const uint16_t red = image_buffer[(i - KERNEL_RADIUS + ki) % KERNEL_HEIGHT][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 0];
          const uint16_t green = image_buffer[(i - KERNEL_RADIUS + ki) % KERNEL_HEIGHT][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 1];
          const uint16_t blue = image_buffer[(i - KERNEL_RADIUS + ki) % KERNEL_HEIGHT][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 2];
          multiplication_sum.red += red * kernel[ki][kj];
          multiplication_sum.green += green * kernel[ki][kj];
          multiplication_sum.blue += blue * kernel[ki][kj];
        }
      }
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 0] = multiplication_sum.red / kernel_sum;
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 1] = multiplication_sum.green / kernel_sum;
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 2] = multiplication_sum.blue / kernel_sum;
    }

    jpeg_write_scanlines(&compressor, output_image_row_samples, 1);
  }

  for (size_t i = IN_FRAME_HEIGHT; i < IMAGE_HEIGHT; i++) {
    for (size_t j = 0; j < KERNEL_RADIUS; j++) {
      struct components multiplication_sum = {
        .red = 0.f,
        .green = 0.f,
        .blue = 0.f,
      };
      unsigned short int kernel_sum = 0u;
      for (size_t ki = 0; ki < KERNEL_RADIUS + IMAGE_HEIGHT - i; ki++) {
        for (size_t kj = KERNEL_RADIUS - j; kj < KERNEL_WIDTH; kj++) {
          kernel_sum += kernel[ki][kj];
          const uint16_t red = image_buffer[(i - KERNEL_RADIUS + ki) % KERNEL_HEIGHT][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 0];
          const uint16_t green = image_buffer[(i - KERNEL_RADIUS + ki) % KERNEL_HEIGHT][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 1];
          const uint16_t blue = image_buffer[(i - KERNEL_RADIUS + ki) % KERNEL_HEIGHT][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 2];
          multiplication_sum.red += red * kernel[ki][kj];
          multiplication_sum.green += green * kernel[ki][kj];
          multiplication_sum.blue += blue * kernel[ki][kj];
        }
      }
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 0] = multiplication_sum.red / kernel_sum;
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 1] = multiplication_sum.green / kernel_sum;
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 2] = multiplication_sum.blue / kernel_sum;
    }

    for (size_t j = KERNEL_RADIUS; j < IN_FRAME_WIDTH; j++) {
      struct components multiplication_sum = {
        .red = 0.f,
        .green = 0.f,
        .blue = 0.f,
      };
      unsigned short int kernel_sum = 0u;
      for (size_t ki = 0; ki < KERNEL_RADIUS + IMAGE_HEIGHT - i; ki++) {
        for (size_t kj = 0; kj < KERNEL_WIDTH; kj++) {
          kernel_sum += kernel[ki][kj];
          const uint16_t red = image_buffer[(i - KERNEL_RADIUS + ki) % KERNEL_HEIGHT][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 0];
          const uint16_t green = image_buffer[(i - KERNEL_RADIUS + ki) % KERNEL_HEIGHT][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 1];
          const uint16_t blue = image_buffer[(i - KERNEL_RADIUS + ki) % KERNEL_HEIGHT][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 2];
          multiplication_sum.red += red * kernel[ki][kj];
          multiplication_sum.green += green * kernel[ki][kj];
          multiplication_sum.blue += blue * kernel[ki][kj];
        }
      }
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 0] = multiplication_sum.red / kernel_sum;
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 1] = multiplication_sum.green / kernel_sum;
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 2] = multiplication_sum.blue / kernel_sum;
    }

    for (size_t j = IN_FRAME_WIDTH; j < IMAGE_WIDTH; j++) {
      struct components multiplication_sum = {
        .red = 0.f,
        .green = 0.f,
        .blue = 0.f,
      };
      unsigned short int kernel_sum = 0u;
      for (size_t ki = 0; ki < KERNEL_RADIUS + IMAGE_HEIGHT - i; ki++) {
        for (size_t kj = 0; kj < KERNEL_RADIUS + IMAGE_WIDTH - j; kj++) {
          kernel_sum += kernel[ki][kj];
          const uint16_t red = image_buffer[(i - KERNEL_RADIUS + ki) % KERNEL_HEIGHT][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 0];
          const uint16_t green = image_buffer[(i - KERNEL_RADIUS + ki) % KERNEL_HEIGHT][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 1];
          const uint16_t blue = image_buffer[(i - KERNEL_RADIUS + ki) % KERNEL_HEIGHT][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 2];
          multiplication_sum.red += red * kernel[ki][kj];
          multiplication_sum.green += green * kernel[ki][kj];
          multiplication_sum.blue += blue * kernel[ki][kj];
        }
      }
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 0] = multiplication_sum.red / kernel_sum;
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 1] = multiplication_sum.green / kernel_sum;
      output_image_row_samples[0][INPUT_IMAGE_COMPONENTS_NUMBER * j + 2] = multiplication_sum.blue / kernel_sum;
    }

    jpeg_write_scanlines(&compressor, output_image_row_samples, 1);
  }

  jpeg_finish_decompress(&decompressor);
  jpeg_finish_compress(&compressor);
  jpeg_destroy_decompress(&decompressor);
  jpeg_destroy_compress(&compressor);
  free(output_image_row_samples[0]);
  free(frame_free_space);
  fclose(input_file);
  fclose(output_file);

  return 0;
}

int main() {
  const char *infilename = "image.jpg";
  const char *outfilename = "image-blurred.jpg";
  return transform(infilename, outfilename);
}
