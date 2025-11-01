#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

#define MAX_THREADS 12

#include "vec.cpp"
#include "rng.cpp"
#include "ray.cpp"
#include "camera.cpp"

#include "light.cpp"
#include "surface.cpp"

#include "scene.cpp"

void render(int id);
buffer _buffers[MAX_THREADS-1];

int main(int argc, char *argv[]){
	if(argc > 3){
		try{
			camera.fps = std::stoi(argv[3]);

		}catch(const std::invalid_argument &e){
			std::cerr << "invalid arg " << e.what() << '\n';

		}catch(const std::out_of_range &e){
			std::cerr << "out of range " << e.what() << '\n';
		}
	}

	if(argc > 2){
		try{
			camera.time = (double) std::stoi(argv[2]) / camera.fps;

		}catch(const std::invalid_argument &e){
			std::cerr << "invalid arg " << e.what() << '\n';

		}catch(const std::out_of_range &e){
			std::cerr << "out of range " << e.what() << '\n';
		}
	}

	setup_camera();
	camera.t.init();
	target.init();

	setup_scene();
	for(sphere &s : spheres) s.init();
	for(rect &s : rects) s.init();
	for(fimage &s : fimages) s.init();
	for(mask &s : masks) s.init();

	if(argc > 1){
		try{
			int threads = std::stoi(argv[1]);
			if(threads > 1 && threads <= MAX_THREADS)
				camera.threads = threads;

		}catch(const std::invalid_argument &e){
			std::cerr << "invalid arg " << e.what() << '\n';
		}catch(const std::out_of_range &e){
			std::cerr << "out of range " << e.what() << '\n';
		}
	}

	std::cout << "tracing with " << camera.threads << " threads\n";

	std::thread threads[MAX_THREADS-1];

	for(int i=1; i<camera.threads; ++i)
		threads[i] = std::thread(render, i);

	render(0);

	for(int i=1; i<camera.threads; ++i)
		threads[i].join();

	for(int i=1; i<camera.threads; ++i)
		target.data += _buffers[i-1];

	target.write();
}

void backward_trace(buffer &buf, int x, int y);
void forward_trace(buffer &buf, const light &l);

void render(int id){
	buffer &buf = id ? _buffers[id-1] : target.data;
	if(id) _buffers[id-1].init();

	auto start = std::chrono::high_resolution_clock::now();

	bool use_global = sky({0, 0, 0}).x > -1;

	if(camera.threads == 1 && camera.sync){
		for(int i=0; i<camera.spp; ++i){
			if(use_global)
				for(int x=0; x<camera.w; ++x)
					for(int y=0; y<camera.h; ++y)
						backward_trace(buf, x, y);

			for(light &l : lights)
				forward_trace(buf, l);

			if(i%camera.sync == 0)
				target.write((double) (i+1) / (double) camera.spp);
		}
	}else{
		if(use_global)
			for(int x=0; x<camera.w; ++x)
				for(int y=0; y<camera.h; ++y)
					for(int i=0; i<camera.spp; ++i)
						backward_trace(buf, x, y);

		for(light &l : lights)
			for(int x=0; x<camera.spp; ++x)
				forward_trace(buf, l);
	}

	auto now = std::chrono::high_resolution_clock::now();
	double duration = ((std::chrono::duration<double>) (now - start)).count();
	std::cout << "thread " << id+1 << " finished in ";
	if(duration > 60.0) std::cout << std::floor(duration / 60.0) << 'm';
	std::cout << std::floor(std::fmod(duration, 60.0)*1e3)/1e3 << "s\n";
}

void backward_trace(buffer &buf, int x, int y){
	ray r; touch t;

	camera.get(x+rng::base(), y+rng::base(), r);

	for(int j=0; j<camera.bounces; ++j){
		if(hit(r, t, 1)){

			if(t.m->T & material::LIGHT){
				r.c *= t.m->light_color;
				camera.set(buf, x, y, r.c);
				break;
			}

			t.scatter(r);

		}else{
			camera.set(buf, x, y, r.c * sky(r.d));
			break;
		}
	}
}

void forward_trace(buffer &buf, const light &l){
	int samples = camera.w * camera.h;
	double factor = camera.c * camera.c / camera.w / camera.h;

	for(int x=0; x<samples; ++x){
		ray r; touch t; l.get(r);

		ray R; touch T;
		R.p = r.p; R.d = (camera.p-r.p).norm();
		if(!hit(R, T) || T.d*T.d > camera.p.distsq(R.p)){
			double E = factor/camera.p.distsq(R.p);
			camera.set(buf, R.p, l.c(R)*E);
		}

		for(int j=0; j<camera.bounces; ++j){
			if(hit(r, t)){
				r.c *= std::min(30.0, 0.5 / -t.n.dot(r.d));

				ray R; touch T;
				R.p = t.p; R.d = (camera.p-t.p).norm();
				if(!hit(R, T) || T.d*T.d > camera.p.distsq(R.p)){
					color E = t.scatter(r.d, R.d)*factor/camera.p.distsq(R.p);
					camera.set(buf, R.p, r.c*E);
					++x;
				}

				t.scatter(r);

			}else break;
		}
	}
}
