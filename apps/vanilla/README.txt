#Description of experiments

--------------------
Experiment 1-sbd: 
-- Encoding: Binary Mask (256 parameters, 16x16)
-- Task: instancesegmentation

command:
-m train -d /media/mikesapi/DATASETDISK/ms-workspace/detection/yolo --dataset sbd --encoding mask -w extraction.conv.weights --task instancesegmentation -g 0

nohup nice -15 ./bin/apps/vanilla/vanilla -m train -d /media/nfshdd/ms-workspace/detection/yolo --dataset sbd --encoding mask -w extraction.conv.weights --task instancesegmentation --seed 54321 -g 2 > exp1-sbd.out 2>&1 &

machine: dm-tvg-linux, spaint

uniqueStamp: train-sbd-yolo-mask-c20-sp256-20161102T121505
--------------------

--------------------
Experiment 1-sbd-demo (trained on trainval): SUCCESS, SAVED
-- Encoding: Binary Mask (256 parameters, 16x16)
-- Task: instancesegmentation

command:
-m train -d /media/mikesapi/DATASETDISK/ms-workspace/detection/yolo --dataset sbd --encoding mask -w extraction.conv.weights --task instancesegmentation -g 0

machine: dm-tvg-linux, spaint

uniqueStamp: train-sbd-yolo-mask-c20-sp256-20161031T163015
train-sbd-yolo-mask-c20-sp256-20161104T110755 (part2)
--------------------

--------------------
Experiment 1-coco:
-- Encoding: Binary Mask (256 parameters, 16x16)
-- Task: instancesegmentation

command:
./bin/apps/vanilla/vanilla -m train -d ~/desktop/yolo --dataset coco --encoding mask -w extraction.conv.weights --task instancesegmentation -g 0

./bin/apps/vanilla/vanilla -m train -d ~/desktop/yolo --dataset coco --encoding mask -w /home/mikesapi/desktop/yolo/results/detection/train-coco-yolo-coco-mask-c80-sp256-20161029T175242/train-coco-yolo-coco-mask-c80-sp256-20161029T175242-final.weights --task instancesegmentation -g 0

machine: ms-tvg-workstation, spaint-runner1

uniqueStamp: train-coco-yolo-coco-mask-c80-sp256-20161029T175242
             train-coco-yolo-coco-mask-c80-sp256-20161101T120444 (part2)
--------------------

--------------------
Experiment 2-sbd: SUCCESS, SAVED
-- Encoding: Radial (256 parameters), gridSize = 5.
-- Task: instancesegmentation

command:
./vanilla -m train -d ~/desktop/yolo --dataset sbd --encoding radial -w extraction.conv.weights --task instancesegmentation -g 0

./build/bin/apps/vanilla/vanilla -m train -d /media/mikesapi/DATASETDISK/ms-workspace/detection/yolo --dataset sbd --encoding radial -w /media/mikesapi/DATASETDISK/ms-workspace/detection/yolo/results/detection/train-sbd-yolo-radial-c20-sp256-20161031T101649/train-sbd-yolo-radial-c20-sp256-20161031T101649-final.weights --task instancesegmentation

machine: mikesapi-tvg-laptop

uniqueStamp: train-sbd-yolo-radial-c20-sp256-20161031T101649
train-sbd-yolo-radial-c20-sp256-20161101T125952 (part2)

--------------------

--------------------
Experiment 2b-coco:
-- Dataset: COCO
-- Encoding: Radial (256 parameters)
-- Task: instancesegmentation

command:
./build/bin/apps/vanilla/vanilla -m train -d ~/desktop/yolo --dataset coco --encoding radial -w extraction.conv.weights --task instancesegmentation -g 1

machine: ms-tvg-workstation, spaint-runner2

uniqueStamp: train-coco-yolo-coco-radial-c80-sp256-20161031T100539
train-coco-yolo-coco-radial-c80-sp256-20161103T093620 (part2)
--------------------

Encoding: embedding 50!
ms-tvg-beast:

CUDA_VISIBLE_DEVICES=2 ./bin/apps/vanilla/vanilla -m train -d /mnt/DATADISK/ms-workspace/detection/yolo --dataset coco --encoding embedding -w extraction.conv.weights --task instancesegmentation -g 0

CUDA_VISIBLE_DEVICES=3 ./bin/apps/vanilla/vanilla -m train -d /mnt/DATADISK/ms-workspace/detection/yolo --dataset sbd --encoding embedding -w extraction.conv.weights --task instancesegmentation -g 0

ms-tvg-workstation:
runner 2: sbd trainval (demo)

Encoding: embedding 20!
runner 1: sbd train





To Test/investigate:
/home/mikesapi/desktop/yolo/results/detection/train-sbd-yolo-embedding-c20-sp20-20161117T153051/train-sbd-yolo-embedding-c20-sp20-20161117T153051-final.weights


CUDA_VISIBLE_DEVICES=3 ./build/bin/apps/vanilla/vanilla -m train -d /mnt/DATADISK/ms-workspace/detection/yolo --dataset sbd --encoding embedding -w extraction.conv.weights --task instancesegmentation -g 0 --shapeparams 20

./bin/apps/vanilla/vanilla -m train -d ~/desktop/yolo -w extraction.conv.weights --dataset sbd --encoding embedding --task instancesegmentation -g 0 --shapeparams 20

