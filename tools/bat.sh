#!/bin/sh

file_list=`ls *.h *.c -allh | awk '{print $9}'`
for file in $file_list
do
  new_file=`echo $file|sed 's/emdb/etdb/g'`
  cat $file | sed 's/emdb/etdb/g' | sed 's/EMDB/ETDB/g' > $new_file
done
