#! /bin/bash -e

VOC_DEVKIT_PATH=$1

cp matlab/evaluate_detection.m $VOC_DEVKIT_PATH/
cp matlab/VOCevaldet.m $VOC_DEVKIT_PATH/VOCcode/

exit 0
