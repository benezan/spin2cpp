#!/bin/sh

if [ "$1" != "" ]; then
  PROG=$1
else
  PROG=../build/spin2cpp
fi
CC=propeller-elf-gcc
ok="ok"
endmsg=$ok

# C mode compilation tests
for i in ctest*.spin
do
  j=`basename $i .spin`
  $PROG --ccode --noheader -DCOUNT=4 $i
  if  diff -ub Expect/$j.h $j.h && diff -ub Expect/$j.c $j.c
  then
      rm -f $j.h $j.c
      echo $j passed
  else
      echo $j failed
      endmsg="TEST FAILURES"
  fi
done

# C++ compilation tests
# These use the -n flag to normalize identifier names because
# I'm too lazy to convert all the identifiers; it also gives -n
# a good workout.

for i in test*.spin
do
  j=`basename $i .spin`
  $PROG -n --noheader -DCOUNT=4 $i
  if  diff -ub Expect/$j.h $j.h && diff -ub Expect/$j.cpp $j.cpp
  then
      rm -f $j.h $j.cpp
      echo $j passed
  else
      echo $j failed
      endmsg="TEST FAILURES"
  fi
done

#
# debug directive tests
#
for i in gtest*.spin
do
  j=`basename $i .spin`
  $PROG -g --noheader -DCOUNT=4 $i
  if  diff -ub Expect/$j.h $j.h && diff -ub Expect/$j.cpp $j.cpp
  then
      rm -f $j.h $j.cpp
      echo $j passed
  else
      echo $j failed
      endmsg="TEST FAILURES"
  fi
done

# clean up
rm -f FullDuplexSerial.cpp FullDuplexSerial.h
if [ "x$endmsg" = "x$ok" ]
then
  rm -f *.h *.cpp
else
  echo $endmsg
  exit 1
fi
