#!/bin/sh

INTERATIONS=250


cp config.h config.h.bk;

WORKERS=$(seq -s ' ' 1 32);
KERNEL_RADIUSES=$(seq -s ' ' 2 2 100);
FILENAME="kernel_radius-$(date +'%Y-%m-%d_%H-%M-%S').bench";
touch "$FILENAME";

echo '> Benchmarking kernel radius per worker...';

for worker in $WORKERS;
do
  echo "  > # workers: $worker";
  for kernel_radius in $KERNEL_RADIUSES;
  do
    cp config.h.bk config.h;
    sed -i s/'static const unsigned short int NUM_THREADS = 4U;'/"static const unsigned short int NUM_THREADS = ${worker}U;"/ config.h;
    sed -i s/'static const uint8_t KERNEL_RADIUS = 20U;'/"static const uint8_t KERNEL_RADIUS = ${kernel_radius}U;"/ config.h;
    rm -rf build;
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release;
    cmake --build build;
    for i in $(seq 1 $INTERATIONS);
    do
      TIME=$(./build/blurrifier | sed s/total://);
      echo "${worker}:${TIME}:${kernel_radius}" >> "$FILENAME";
      echo -n "$i ($TIME) ";
    done;
  done;
  echo "  > # workers: $worker Done.";
done;

echo '> Done.';

IMAGE_RATIOS=$(seq -s ' ' 1 20);
KERNEL_RADIUS=15
FILENAME="image_ratio-$(date +'%Y-%m-%d_%H-%M-%S').bench";
touch "$FILENAME";

echo '> Benchmarking image ratio per worker...';

for worker in $WORKERS;
do
  echo "  > # workers: $worker";
  cp config.h.bk config.h;
  sed -i s/'static const unsigned short int NUM_THREADS = 8U;'/"static const unsigned short int NUM_THREADS = ${worker}U;"/ config.h;
  sed -i s/'static const uint8_t KERNEL_RADIUS = 20U;'/"static const uint8_t KERNEL_RADIUS = ${KERNEL_RADIUS}U;"/ config.h;
  sed -i s/'static const char *INPUT_IMAGE_FILENAME = "images/01.jpg";'/'static const char *INPUT_IMAGE_FILENAME = "ratio-motanged.jpg";'/ config.h;
  rm -rf build;
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release;
  cmake --build build;
  for image_ratio in $IMAGE_RATIOS;
  do
    DUPLICATE_RATE=$(expr "$image_ratio" - 1);
    montage -mode concatenate -tile "${image_ratio}x1" -duplicate "${DUPLICATE_RATE},0"  ratio-base.jpg ratio-motanged.jpg
    for i in $(seq 1 $INTERATIONS);
    do
      TIME=$(./build/blurrifier | sed s/total://);
      echo "${worker}:${TIME}:${image_ratio}" >> "$FILENAME";
      echo -n "$i ($TIME) ";
    done;
  done;
  echo "  > # workers: $worker Done.";
done;

echo '> Done.';
