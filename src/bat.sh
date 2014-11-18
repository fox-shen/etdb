#!/bin/sh

file_list=`ls *.h *.c -allh | awk '{print $9}'`
for file in $file_list
do
  new_file=`echo $file|`
  cat $file | sed 
done
