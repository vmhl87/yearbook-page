#pragma once

#include "bmp.cpp"

struct buffer{
	color *data;

	void init();
	const color operator[](size_t i) const;
	void operator+=(buffer &o);
	void add(int x, int y, color c);
};

struct _camera{
	vec p;
	transform t;

	int w, h;
	double c;

	int spp = 1, sync = 0;
	double exposure = 1.0,
		   gamma = 2.2;

	int bounces = 5;

	double time = 0.0, fps = 15.0;

	int threads = 1;

	void get(double X, double Y, ray &r){
		vec dir = {
			(X-w/2)/c,
			(h/2-Y)/c,
			1.0,
		};

		r.d = t.apply(dir).norm();
		r.c = {1, 1, 1};
		r.p = p;
	}

	void set(buffer &b, int x, int y, color col){
		b.add(x, y, col);
	}

	void set(buffer &b, vec &ray, color col){
		vec coord = t.revert(ray-p);
		if(coord.z <= 0.0) return;
		coord /= coord.z;
		coord *= c;
		int x = w/2 + std::floor(coord.x),
			y = h/2 - std::floor(coord.y) - 1;

		if(x >= 0 && x < w && y >= 0 && y < h)
			b.add(x, y, col);
	}
};

_camera camera;

void buffer::init(){
	data = new color[camera.w*camera.h];
}

const color buffer::operator[](size_t i) const{ return data[i]; }

void buffer::operator+=(buffer &o){
	for(int i=0; i<camera.w*camera.h; ++i)
		data[i] += o.data[i];
}

void buffer::add(int x, int y, color c){
	data[x+y*camera.w] += c;
}

struct _target{
	unsigned char *image = nullptr;
	double *error = nullptr;
	buffer data;

	void init(){
		data.init();
		image = new unsigned char[camera.w*camera.h*3];
		error = new double[camera.w*camera.h*3];
	}

	void write(double progress = 1.0){
		double exp2 = camera.threads * camera.spp * progress / camera.exposure;

		auto c = [&] (double v) {
			return std::pow(v, 1.0/camera.gamma) * 255.0;
		};

		for(int i=0; i<camera.w*camera.h; ++i){
			color E = data[i]/exp2;
			E /= (E + vec{1, 1, 1});
			error[i*3+2] = c(E.x);
			error[i*3+1] = c(E.y);
			error[i*3] = c(E.z);
		}

		for(int x=0; x<camera.w; ++x){
			for(int y=0; y<camera.h; ++y){
				int i = x+y*camera.w;
				for(int j=0; j<3; ++j){
					double E2 = error[i*3+j]+rng::base()*2-1;
					int E = std::round(E2);
					double err = error[i*3+j]-E;
					image[i*3+j] = std::max(0, std::min(255, E));
					if(x<camera.w-1) error[i*3+j+3] += err * 7.0/16.0;
					if(x>0 && y<camera.h-1) error[i*3+j+camera.w*3-3] += err * 3.0/16.0;
					if(y<camera.h-1) error[i*3+j+camera.w*3] += err * 5.0/16.0;
					if(x<camera.w-1 && y<camera.h-1) error[i*3+j+camera.w*3+3] += err * 1.0/16.0;
				}
			}
		}

		writeBMP("image.bmp", image, camera.w, camera.h);
	}
};

_target target;
