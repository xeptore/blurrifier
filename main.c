#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <memory.h>
#include <jpeglib.h>
#include <math.h>
#include <omp.h>
#include <pthread.h>
#include <sys/time.h>
#include "config.h"

const uint8_t INPUT_IMAGE_COMPONENTS_NUMBER = 3u;
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

struct pixel_components {
  double red;
  double green;
  double blue;
};

struct kernel_wrapper {
  double kernel[KERNEL_HEIGHT][KERNEL_WIDTH];
};

double gaussian(const double x, const double mu, const double sigma) {
  /**
   * It is ugly, I know!
   * But it's math, and math is ugly :D
   * The formula comes from here: https://en.wikipedia.org/wiki/Gaussian_function.
   */
  return exp(-(pow((x - mu) / sigma, 2) / 2.f));
}

struct kernel_wrapper produce_gaussian_kernel(void) {
  const double sigma = KERNEL_RADIUS / 2.f;
  struct kernel_wrapper output;

  double kernel_row[KERNEL_WIDTH];
  for (size_t i = 0; i < KERNEL_WIDTH; i++) {
    kernel_row[i] = gaussian(i, KERNEL_RADIUS, sigma);
  }

  for (size_t i = 0; i < KERNEL_HEIGHT; i++) {
    for (size_t j = 0; j < KERNEL_WIDTH; j++) {
      const double v = kernel_row[i] * kernel_row[j];
      output.kernel[i][j] = v;
    }
  }

  return output;
}

struct kernel_wrapper produce_mean_kernel(void) {
  struct kernel_wrapper output;

  for (size_t i = 0; i < KERNEL_HEIGHT; i++)
    for (size_t j = 0; j < KERNEL_WIDTH; j++) {
      output.kernel[i][j] = 1.f;
    }

  return output;
}

struct transform_row_params {
  unsigned short IMAGE_WIDTH;
  unsigned short IMAGE_HEIGHT;
  JSAMPARRAY input_image;
  JSAMPARRAY output_image;
  double kernel[KERNEL_HEIGHT][KERNEL_WIDTH];
  unsigned short start_row;
  unsigned short num_rows;
};

void copy_kernel(double destination[KERNEL_HEIGHT][KERNEL_WIDTH], const double source[KERNEL_HEIGHT][KERNEL_WIDTH]) {
  for (size_t i = 0; i < KERNEL_HEIGHT; i++) {
    for (size_t j = 0; j < KERNEL_WIDTH; j++) {
      destination[i][j] = source[i][j];
    }
  };
}

void transform_rows(struct transform_row_params *params) {
  for (size_t i = params->start_row; i < params->start_row + params->num_rows; i++) {
    for (size_t j = 0; j < params->IMAGE_WIDTH; j++) {
      struct pixel_components components_multiplication_sum = {
        .red = 0.f,
        .green = 0.f,
        .blue = 0.f,
      };
      double kernel_cells_sum = 0.f;
      const size_t ki_start = i > KERNEL_RADIUS ? 0 : KERNEL_RADIUS - i;
      const size_t ki_end = i > params->IMAGE_HEIGHT - KERNEL_RADIUS ? params->IMAGE_HEIGHT - i + KERNEL_RADIUS : KERNEL_HEIGHT;
      const size_t kj_start = j > KERNEL_RADIUS ? 0 : KERNEL_RADIUS - j;
      const size_t kj_end = j > params->IMAGE_WIDTH - KERNEL_RADIUS ? params->IMAGE_WIDTH - j + KERNEL_RADIUS : KERNEL_WIDTH;
      for (size_t ki = ki_start; ki < ki_end; ki++) {
        for (size_t kj = kj_start; kj < kj_end; kj++) {
          kernel_cells_sum += params->kernel[ki][kj];
          const uint16_t red = params->input_image[i + ki - KERNEL_RADIUS][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 0];
          const uint16_t green = params->input_image[i + ki - KERNEL_RADIUS][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 1];
          const uint16_t blue = params->input_image[i + ki - KERNEL_RADIUS][(j + kj - KERNEL_RADIUS) * INPUT_IMAGE_COMPONENTS_NUMBER + 2];
          components_multiplication_sum.red += red * params->kernel[ki][kj];
          components_multiplication_sum.green += green * params->kernel[ki][kj];
          components_multiplication_sum.blue += blue * params->kernel[ki][kj];
        }
      }
      params->output_image[i][INPUT_IMAGE_COMPONENTS_NUMBER * j + 0] = round(components_multiplication_sum.red / kernel_cells_sum);
      params->output_image[i][INPUT_IMAGE_COMPONENTS_NUMBER * j + 1] = round(components_multiplication_sum.green / kernel_cells_sum);
      params->output_image[i][INPUT_IMAGE_COMPONENTS_NUMBER * j + 2] = round(components_multiplication_sum.blue / kernel_cells_sum);
    }
  }
}

int transform(
  const char *input_filename,
  const char *output_filename,
  const double kernel[KERNEL_HEIGHT][KERNEL_WIDTH]
) {
  FILE *input_file = fopen(input_filename, "rb");
  if (!input_file) {
    (void)fprintf(
      stderr,
      "🛑🙁 error opening input jpeg file '%s': %s 🙁🛑\n",
      input_filename,
      strerror(errno)
    );
    return errno;
  }

  FILE *output_file = fopen(output_filename, "wb");
  if (!output_file) {
    (void)fprintf(
      stderr,
      "🛑🙁 error opening output jpeg file '%s': %s 🙁🛑\n",
      output_filename,
      strerror(errno)
    );
    return errno;
  }

  struct jpeg_error_mgr error_manager;

  struct jpeg_decompress_struct decompressor;
  set_decompressor_options(&decompressor, &error_manager, input_file);

  const unsigned short IMAGE_WIDTH = decompressor.image_width;
  const unsigned short IMAGE_HEIGHT = decompressor.image_height;

  struct jpeg_compress_struct compressor;
  set_compressor_options(&compressor, &decompressor, &error_manager, output_file);

  const unsigned long IMAGE_SIZE_IN_BYTES = IMAGE_HEIGHT * IMAGE_WIDTH * INPUT_IMAGE_COMPONENTS_NUMBER;

  unsigned char *buffer = malloc(2 * IMAGE_SIZE_IN_BYTES + NUM_THREADS * sizeof(struct transform_row_params));
  JSAMPROW write_buffer[IMAGE_HEIGHT];
  for (size_t i = 0; i < IMAGE_HEIGHT; i++) {
    write_buffer[i] = &buffer[i * IMAGE_WIDTH * INPUT_IMAGE_COMPONENTS_NUMBER];
  }

  JSAMPROW read_buffer[IMAGE_HEIGHT];
  for (size_t i = 0; i < IMAGE_HEIGHT; i++) {
    read_buffer[i] = &buffer[i * IMAGE_WIDTH * INPUT_IMAGE_COMPONENTS_NUMBER + IMAGE_SIZE_IN_BYTES];
  }

  while (decompressor.output_scanline < decompressor.output_height) {
    (void)jpeg_read_scanlines(
      &decompressor,
      &read_buffer[decompressor.output_scanline],
      decompressor.output_height - decompressor.output_scanline
    );
  }

  pthread_t thread_ids[NUM_THREADS];
  struct transform_row_params *thread_params_refs[NUM_THREADS];

  const unsigned int quotient = decompressor.image_height / NUM_THREADS;
  const unsigned int remainder = decompressor.image_height % NUM_THREADS;

  unsigned long total_assigned_rows = 0U;

  struct timespec start_time, end;

  timespec_get(&start_time, TIME_UTC);

  #pragma omp parallel for
  for (size_t i = 0; i < NUM_THREADS; i++) {
    const unsigned long int worker_quotient = (i < remainder) ? (quotient + 1) : (quotient);
    struct transform_row_params *params = (struct transform_row_params *)&buffer[2 * IMAGE_SIZE_IN_BYTES + i * sizeof(struct transform_row_params)];
    params->input_image = read_buffer;
    params->output_image = write_buffer;
    copy_kernel(params->kernel, kernel);
    params->IMAGE_HEIGHT = IMAGE_HEIGHT;
    params->IMAGE_WIDTH = IMAGE_WIDTH;
    params->num_rows = worker_quotient;
    params->start_row = total_assigned_rows;
    total_assigned_rows += worker_quotient;
    transform_rows(params);
  }

  timespec_get(&end, TIME_UTC);

  unsigned long int time_in_nano_seconds = (end.tv_sec - start_time.tv_sec) * 1e9 + (end.tv_nsec - start_time.tv_nsec);
  printf("total:%lu", time_in_nano_seconds);

  while (compressor.next_scanline < compressor.image_height) {
    (void)jpeg_write_scanlines(
      &compressor,
      &write_buffer[compressor.next_scanline],
      compressor.image_height - compressor.next_scanline
    );
  }

  (void)jpeg_finish_decompress(&decompressor);
  jpeg_finish_compress(&compressor);
  jpeg_destroy_decompress(&decompressor);
  jpeg_destroy_compress(&compressor);
  free(buffer);
  fclose(input_file);
  fclose(output_file);

  return 0;
}

int main() {
  omp_set_num_threads(NUM_THREADS);

  return transform(
    INPUT_IMAGE_FILENAME,
    OUTPUT_IMAGE_FILENAME,
    produce_gaussian_kernel().kernel
  );
}
