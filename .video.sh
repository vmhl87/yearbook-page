rm -f render.mp4
rm -rf .video_frames/
mkdir -p .video_frames/

for ((i=0; i<=$2*$3; i++)); do
	echo "rendering frame $i"
	./run $1 $i $3
	cp image.bmp .video_frames/$i.bmp
done

ffmpeg -framerate $3 -i .video_frames/%d.bmp -c:v libx264 -crf 18 -pix_fmt yuv420p render.mp4
