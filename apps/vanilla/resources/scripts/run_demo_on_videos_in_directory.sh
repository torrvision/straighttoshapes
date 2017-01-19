#!/bin/bash -e

VIDEO_DIR=$1

echo ${VIDEO_DIR}

for f in ${VIDEO_DIR}/*.mp4
do
  echo "Processing ${f}..";
  ./../../../../build/bin/apps/vanilla/vanilla -m demo -d /media/mikesapi/DATASETDISK/ms-workspace/detection/yolo --dataset sbd --encoding embedding -w /media/mikesapi/DATASETDISK/ms-workspace/detection/yolo/results/cvpr2017/demoModels/sbdae50/train-sbd-yolo-embedding-c20-sp50-20161110T124531/train-sbd-yolo-embedding-c20-sp50-20161110T124531-final.weights   --task instancesegmentation --videoFile ${f} --saveDir /tmp/ --shapeparams 50
done
