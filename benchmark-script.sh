#!/bin/sh

ITERATIONS=100;

WORKERS=$(seq -s ' ' 1 32);
KERNEL_RADIUSES=$(seq -s ' ' 2 2 20);
FILENAME="kernel_radius-$(date +'%Y-%m-%d_%H-%M-%S').bench";
touch "$FILENAME";

echo '> Benchmarking kernel radius per worker...';

for worker in $WORKERS;
do
  echo "> # workers: $worker";
  for kernel_radius in $KERNEL_RADIUSES;
  do
    echo "> kernel radius: $kernel_radius";
    cp config.h.bk config.h;
    sed -i s/'_THREADS_U;'/"${worker}U;"/ config.h;
    sed -i s/'_RADIUS_U;'/"${kernel_radius}U;"/ config.h;
    rm -rf build;
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release --log-level=ERROR;
    cmake --build build --log-level=ERROR;

    if [ "$worker" -ge 17 ];
    then
        PARALLELISM=1;
    else
        PARALLELISM=$(expr 16 '/' "$worker");
    fi;
    echo -n "> Repating $ITERATIONS times with parallelism $PARALLELISM...";
    ./repeat --iterations "$ITERATIONS" --parallelism "$PARALLELISM" './build/blurrifier' >> "$FILENAME";
    echo '> Done.';
    echo '> Done.';
  done;
  echo "> # workers: $worker Done.";
done;

echo '> Done.';

exit

IMAGE_RATIOS=$(seq -s ' ' 1 25);
KERNEL_RADIUS=10
FILENAME="image_ratio-$(date +'%Y-%m-%d_%H-%M-%S').bench";
touch "$FILENAME";

echo '> Benchmarking image ratio per worker...';

cp images/01.jpg ratio-base.jpg;

for worker in $WORKERS;
do
  echo "  > # workers: $worker";
  cp config.h.bk config.h;
  sed -i s/'_THREADS_U;'/"${worker}U;"/ config.h;
  sed -i s/'_RADIUS_U;'/"${KERNEL_RADIUS}U;"/ config.h;
  rm -rf build;
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release;
  cmake --build build;
  for image_ratio in $IMAGE_RATIOS;
  do
    DUPLICATE_RATE=$(expr "$image_ratio" - 1);
    montage -mode concatenate -tile "${image_ratio}x1" -duplicate "${DUPLICATE_RATE},0"  ratio-base.jpg images/01.jpg
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
