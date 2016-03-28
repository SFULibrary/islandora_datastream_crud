#!/bin/sh

###############################################################
# Simple shell script that uses ImageMagick's montage command #
# to add a watermark/label to a group of image files, like    #
# Islandora thumbnails.                                       #
#                                                             #
# Usage: ./add_label_to_image.sh                              #
###############################################################

# You will want to modify these variables before running the command.
INPUTDIR=/tmp/imagetest
OUTPUTDIR=/tmp/imagetest_out
LABEL="I am a label"
# See list of color names at http://www.imagemagick.org/script/color.php
BGCOLOR=grey88

for i in $INPUTDIR/*
do
    if [ ! -d "$OUTPUTDIR" ]; then mkdir "$OUTPUTDIR"; fi
    filename=$(basename "$i")
    montage \
    -label "$LABEL" \
    "$i" \
    -geometry +0+0 \
    -background $BGCOLOR \
    "$OUTPUTDIR/$filename"
done

echo "Finished adding '$LABEL' to files. Ouput is in $OUTPUTDIR."
