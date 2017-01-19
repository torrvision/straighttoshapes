#! /bin/bash -e

echo "[straighttoshapes] ...Checking dependencies..."
PKGSTOINSTALL=""
for var in "$@"
do
  if [[ ! `dpkg -l | grep -w "ii  $var"` ]]
  then
    echo "[straighttoshapes] ...$var... MISSING"
    PKGSTOINSTALL="$PKGSTOINSTALL $var"
  else
    echo "[straighttoshapes] ...$var... OK"
  fi
done

if [ "$PKGSTOINSTALL" != "" ]
then
  echo "[straighttoshapes] ...Installing missing dependencies:$PKGSTOINSTALL ..."
  sudo apt-get install $PKGSTOINSTALL
fi
