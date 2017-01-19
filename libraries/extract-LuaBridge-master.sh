#! /bin/bash -e

echo "[straighttoshapes] Extracting LuaBridge-master"

if [ -d LuaBridge-master ]
then
  echo "[straighttoshapes] ...Skipping archive extraction (already extracted)"
  exit
else
  echo "[straighttoshapes] ...Extracting archive..."
  /bin/rm -fR tmp
  mkdir tmp
  cd tmp
  tar xzf ../setup/LuaBridge-master/LuaBridge-master.tar.gz
  cd ..
  mv tmp/LuaBridge-master .
  rmdir tmp
fi
