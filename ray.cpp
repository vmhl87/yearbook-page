#pragma once
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#include <initializer_list>

struct ray{
	vec p, d;
	color c;
};

struct material{
	static const short LIGHT = 1, DIFFUSE = 2, SPECULAR = 4;

	short T;
	color light_color, diffuse_color, specular_color;
	double specular_fract;
	double smooth;

	static material diffuse(color c){
		return {
			.T = DIFFUSE,
			.diffuse_color = c,
		};
	}

	static material metal(color c, double smooth){
		return {
			.T = SPECULAR,
			.specular_color = c,
			.smooth = smooth,
		};
	}

	static material dielectric(color c, double specular, double smooth){
		double r = std::min(0.75, std::max(0.25, specular));

		return {
			.T = DIFFUSE | SPECULAR,
			.diffuse_color = c*(1-specular)/(1-r),
			.specular_color = {
				specular/r,
				specular/r,
				specular/r,
			},
			.specular_fract = r,
			.smooth = smooth,
		};
	}

	static material light(color c){
		return {
			.T = LIGHT,
			.light_color = c,
		};
	}
};

vec cosine_lobe(double m, vec n){
	double costheta = std::pow(rng::base(), 1.0 / (1.0 + m)),
		   sintheta = std::sqrt(1.0 - costheta*costheta);

	double sign = copysign(1.0, n.z);
	double a = -1.0 / (sign + n.z);
	double b = n.x * n.y * a;

	double phi = rng::base() * M_PI * 2.0,
		   cosphi = cos(phi), sinphi = sin(phi);

	return vec{
		sintheta * ((1.0 + sign*n.x*n.x*a)*cosphi + b*sinphi) + n.x*costheta,
		sintheta * (sign*b*cosphi + (sign+n.y*n.y*a)*sinphi) + n.y*costheta,
		n.z*costheta - sintheta * (sign*n.x*cosphi + n.y*sinphi),
	};
}

double cosine_dist(double m, vec n, vec v){
	//return std::pow(std::max(0.0, v.dot(n)), m) * (m + 1.0);
	return std::pow(std::abs(v.dot(n)), m) * (m + 1.0);
}

vec _sun_dir = (vec{2.6, 2.5, 7}).norm();

struct touch{
	const material *m;
	vec n, p;
	double d;

	void scatter(ray &in){
		in.p = p;

		bool s_diffuse = 0, s_specular = 0;

		if((m->T & material::DIFFUSE) && (m->T & material::SPECULAR)){
			double x = rng::base();
			if(x > m->specular_fract) s_diffuse = 1;
			else s_specular = 1;

		}else if(m->T & material::DIFFUSE) s_diffuse = 1;
		else if(m->T & material::SPECULAR) s_specular = 1;

		if(s_diffuse){
			in.c *= m->diffuse_color;
			in.d = (rng::uniform()+n).norm();
		}

		if(s_specular){
			in.c *= m->specular_color;
			vec surface = cosine_lobe(m->smooth, n);
			in.d = in.d - surface*2.0*in.d.dot(surface);
			if(in.d.dot(n) < 0.0) in.d = in.d - n*2.0*in.d.dot(n);
		}
	}

	color scatter(vec in, vec out){
		if((m->T & material::DIFFUSE) && (m->T & material::SPECULAR)){
			double fract = 0.0;

			vec s1 = (out-in).norm();
			double f1 = std::sqrt(0.5 / (1.0 - in.dot(out)));
			fract += cosine_dist(m->smooth, n, s1) * std::min(100.0, f1);

			vec iv = out - n*2.0*out.dot(n);
			vec s2 = (iv-in).norm();
			double f2 = std::sqrt(0.5 / (1.0 - in.dot(iv)));
			fract += cosine_dist(m->smooth, n, s2) * std::min(100.0, f2);

			return lerp(
				m->diffuse_color*out.dot(n)*2.0*M_PI,
				m->specular_color*fract/2.0,
				m->specular_fract);

		}else if(m->T & material::DIFFUSE){
			return m->diffuse_color*out.dot(n)*2.0*M_PI;

		}else if(m->T & material::SPECULAR){
			double fract = 0.0;

			vec s1 = (out-in).norm();
			double f1 = std::sqrt(0.5 / (1.0 - in.dot(out)));
			fract += cosine_dist(m->smooth, n, s1) * std::min(100.0, f1);

			vec iv = out - n*2.0*out.dot(n);
			vec s2 = (iv-in).norm();
			double f2 = std::sqrt(0.5 / (1.0 - in.dot(iv)));
			fract += cosine_dist(m->smooth, n, s2) * std::min(100.0, f2);

			return m->specular_color*fract/2.0;
		}

		return m->light_color;
	}
};

struct bbox{
	vec min, max;

	void set(vec p){
		min = p, max = p;
	}

	void add(vec p){
		min.x = std::min(min.x, p.x);
		min.y = std::min(min.y, p.y);
		min.z = std::min(min.z, p.z);

		max.x = std::max(max.x, p.x);
		max.y = std::max(max.y, p.y);
		max.z = std::max(max.z, p.z);
	}

	void operator()(std::initializer_list<vec> v){
		set(*v.begin()); for(vec x : v) add(x);
	}

	bool outside(const ray &r) const{
		double near, far, a, b;

		a = (min.x-r.p.x) / r.d.x;
		b = (max.x-r.p.x) / r.d.x;

		near = std::min(a, b);
		far = std::max(a, b);

		a = (min.y-r.p.y) / r.d.y;
		b = (max.y-r.p.y) / r.d.y;

		near = std::max(near, std::min(a, b));
		far = std::min(far, std::max(a, b));

		a = (min.z-r.p.z) / r.d.z;
		b = (max.z-r.p.z) / r.d.z;

		near = std::max(near, std::min(a, b));
		far = std::min(far, std::max(a, b));

		return near > far;
	}
};
