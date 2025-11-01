while true; do
	inotifywait -e modify scene.cpp
	make threads=10
done
