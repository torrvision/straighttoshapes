#! /bin/bash -e

/bin/rm -fR build
/bin/rm -fR docs
/bin/rm -fR install
/bin/rm -fR *.tags 

cd libraries
/bin/rm -fR boost_1_59_0
/bin/rm -fR boost-setup
/bin/rm -fR opencv-3.1.0
/bin/rm -fR opencv_contrib-3.1.0
/bin/rm -fR Eigen-3.2.2
/bin/rm -fR *.log
