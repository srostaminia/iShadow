#!/bin/bash

if [ 2 != $# ]; then
   echo "ERROR: too few arguments"
   echo "usage: $0 <oldname> <newname>"
   exit 1
fi

oldname=$1
newname=$2


for ii in `ls -1`; do
   oldfilename=$ii
   newfilename=${oldfilename//$oldname/$newname}
   echo mv $oldfilename $newfilename
   mv $oldfilename $newfilename
done

