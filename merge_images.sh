#!/bin/bash

# Set the output video filename and frame rate (in frames per second)
output_filename="output.mp4"
frame_duration=1  # in seconds

# Calculate the frame rate based on the frame duration
#frame_rate=$(echo "scale=2; 1/$frame_duration" | bc)
frame_rate=25
# Calculate the number of images and the frame rate
num_images=$#
#frame_rate=$(echo "scale=2; $num_images/$frame_duration" | bc)
echo number of images is $num_images
echo frame rate is $frame_rate

# Construct the FFmpeg command to merge the PGM files
ffmpeg_cmd="ffmpeg -y"
for file in "$@"; do
  #ffmpeg_cmd+=" -loop 1 -framerate $frame_rate -i $file"
  ffmpeg_cmd+=" -loop 1 -i $file"
done
#ffmpeg_cmd+=" -c:v libx264 -pix_fmt yuv420p $output_filename"
ffmpeg_cmd+=" -c:v libx264 -pix_fmt yuv420p -r $frame_rate $output_filename"

# Execute the FFmpeg command
eval $ffmpeg_cmd

#./merge_images.sh pattern_ship_00001.pgm pattern_ship_00002.pgm pattern_ship_00003.pgm pattern_ship_00004.pgm pattern_ship_00005.pgm pattern_ship_00006.pgm pattern_ship_00007.pgm pattern_ship_00008.pgm pattern_ship_00009.pgm pattern_ship_00010.pgm pattern_ship_00011.pgm pattern_ship_00012.pgm pattern_ship_00013.pgm pattern_ship_00014.pgm pattern_ship_00015.pgm pattern_ship_00016.pgm pattern_ship_00017.pgm pattern_ship_00018.pgm pattern_ship_00019.pgm pattern_ship_00020.pgm pattern_ship_00021.pgm pattern_ship_00022.pgm pattern_ship_00023.pgm pattern_ship_00024.pgm pattern_ship_00025.pgm pattern_ship_00026.pgm pattern_ship_00027.pgm pattern_ship_00028.pgm pattern_ship_00029.pgm pattern_ship_00030.pgm
#pattern_ship_00001
#ffmpeg -r 1/30 -start_number 1 -i pattern_ship_%05d.pgm -c:v libx264 -r 30 -pix_fmt yuv420p out.mp4
#https://stackoverflow.com/questions/24961127/how-to-create-a-video-from-images-with-ffmpeg
#longest answer
#ffmpeg -framerate 30 -pattern_type glob -i '*.pgm' -c:v libx264 -pix_fmt yuv420p out.mp4
#Slideshow video with one image per second
#ffmpeg -framerate 1 -pattern_type glob -i '*.png' -c:v libx264 -r 30 -pix_fmt yuv420p out.mp4
#ffmpeg -framerate 1 -pattern_type glob -i '*.pgm' -c:v libx264 -r 30 -pix_fmt yuv420p out.mp4