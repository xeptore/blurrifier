// Results in a ((2 * KERNEL_RADIUS + 1) * (2 * KERNEL_RADIUS + 1)) kernel window
static const uint8_t KERNEL_RADIUS = 20U;

static const unsigned short int NUM_THREADS = 4U;

static const char *INPUT_IMAGE_FILENAME = "images/01.jpg";

static const char *OUTPUT_IMAGE_FILENAME = "images/01-blurred-gaussian-20.jpg";
