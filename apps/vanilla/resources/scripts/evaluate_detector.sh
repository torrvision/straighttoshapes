#! /bin/bash -e

RESULTS_PATH=$1
RESULTS_FILENAME=$2
COMPETITION_CODE=$3
TEST_SET=$4

doStuff () {
  echo ${RESULTS_PATH} | sed 's/\(yolo\/\).*/\1/g'
}

YOLO_PATH=$(doStuff)

SCRIPT_PATH=${YOLO_PATH}data/voc/VOCdevkit/

echo "YOLO_PATH = $YOLO_PATH"
echo "SCRIPT_PATH = $SCRIPT_PATH"
echo "RESULTS_PATH = $RESULTS_PATH"
echo "RESULTS_FILENAME = $RESULTS_FILENAME"
echo "COMPETITION_CODE = $COMPETITION_CODE"
echo "TEST_SET = $TEST_SET"

/usr/local/bin/matlab -nodesktop -nosplash -nodisplay -r "cd('${SCRIPT_PATH}'); try, evaluate_detection('$RESULTS_PATH','$RESULTS_FILENAME','$COMPETITION_CODE','$TEST_SET'); catch err; disp(err.message); end; quit" -logfile /tmp/evaluate_detection_$RESULTS_PATH.log


exit 0
