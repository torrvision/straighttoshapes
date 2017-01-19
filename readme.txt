
# In order to run yolo demo on live webcam:
./build/bin/apps/vanilla/vanilla -d /media/mikesapi/DATASETDISK/ms-workspace/detection/yolo/ -m demo --encoding bbox

# Testing the system on an image:
./build/bin/apps/vanilla/vanilla -d /media/mikesapi/DATASETDISK/ms-workspace/detection/yolo/ -m test --task detection -i ./apps/vanilla/resources/2008_001122.jpg -t 0.2

./build/bin/apps/vanilla/vanilla -d /media/mikesapi/DATASETDISK/ms-workspace/detection/yolo/ -m test --task instancesegmentation -i ./apps/vanilla/resources/2008_001122.jpg -t 0.2 --encoding embedding --shapeparams 20

./build/bin/apps/vanilla/vanilla -d /media/mikesapi/DATASETDISK/ms-workspace/detection/yolo/ -m test --task instancesegmentation -i ./apps/vanilla/resources/2008_001122.jpg -t 0.2 --encoding embedding --shapeparams 50
