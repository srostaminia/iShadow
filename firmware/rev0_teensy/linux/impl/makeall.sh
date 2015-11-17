#!/bin/bash

# Russ Bielawski
# 2012-11-15

# pretty quick and dirty

prognames=( 'cvEyeTracker'
            'frontdisplayandsave'
            'frontsave'
            'frontgazecoordsoverlay'
            'fronteyedetect'
            'gazesetgroundtruth'
            'glassescapsnapshots'
            'glassescapstream'
            'glassesreplaystream'
            'glassesreplaystream_color_hack'
            'ml_rawfeatures_knearest' 
            'randomizeondisk' )


if [ ! -d "./bin" ]; then
   mkdir bin
fi

for ii in ${prognames[*]}; do
   if [ -d $ii ]; then
      cd $ii
      make
      if [ 0 == $? ]; then
         cp -pf $ii ../bin
      fi
      cd ..
   fi
done

