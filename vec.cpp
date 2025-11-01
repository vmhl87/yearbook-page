#pragma once

#include <cmath>

struct vec{
	double x, y, z;

#define op(oper) inline const vec operator oper(const vec &o) const{ \
	return {x oper o.x, y oper o.y, z oper o.z}; }

	op(+) op(-) op(*) op(/)

#undef op

#define op(oper) inline void operator oper##=(const vec &o){ \
	x oper##= o.x, y oper##= o.y, z oper##= o.z; }

	op(+) op(-) op(*) op(/)

#undef op

#define op(oper) inline const vec operator oper(const double d) const{ \
	return {x oper d, y oper d, z oper d}; }

	op(+) op(-) op(*) op(/)

#undef op

#define op(oper) inline void operator oper##=(const double d){ \
	x oper##= d, y oper##= d, z oper##= d; }

	op(+) op(-) op(*) op(/)

#undef op

	double magsq() const{
		return x*x + y*y + z*z;
	}

	double mag() const{
		return std::sqrt(magsq());
	}

	vec norm() const{
		double m = mag();
		return {x/m, y/m, z/m};
	}

	double dot(const vec o) const{
		return x*o.x + y*o.y + z*o.z;
	}

	vec cross(const vec o) const{
		return {
			y*o.z - z*o.y,
			z*o.x - x*o.z,
			x*o.y - y*o.x,
		};
	}

	double distsq(const vec o) const{
		return (*this-o).magsq();
	}

	double dist(const vec o) const{
		return (*this-o).mag();
	}
};

vec lerp(const vec a, const vec b, double v){
	return {
		b.x*v + a.x*(1.-v),
		b.y*v + a.y*(1.-v),
		b.z*v + a.z*(1.-v),
	};
}

using color = vec;

struct transform{
	vec p = {0, 0, 0},
		x = {1, 0, 0},
		y = {0, 1, 0},
		z = {0, 0, 1};

	transform *t = nullptr;

	void init(){
		z = z.norm();
		x = y.cross(z).norm();
		y = z.cross(x).norm();
	}

	vec apply(const vec &v) const{
		vec res = {
			p.x + z.x*v.z + y.x*v.y + x.x*v.x,
			p.y + z.y*v.z + y.y*v.y + x.y*v.x,
			p.z + z.z*v.z + y.z*v.y + x.z*v.x,
		};

		if(t != nullptr) return t->apply(res);
		return res;
	}

	vec revert(const vec &V) const{
		vec v = (t == nullptr ? V : t->revert(V)) - p;

		return {
			v.x*x.x + v.y*x.y + v.z*x.z,
			v.x*y.x + v.y*y.y + v.z*y.z,
			v.x*z.x + v.y*z.y + v.z*z.z,
		};
	}

	vec apply_d(const vec &v) const{
		vec res = {
			z.x*v.z + y.x*v.y + x.x*v.x,
			z.y*v.z + y.y*v.y + x.y*v.x,
			z.z*v.z + y.z*v.y + x.z*v.x,
		};

		return t == nullptr ? res : t->apply_d(res);
	}

	vec revert_d(const vec &V) const{
		vec v = t == nullptr ? V : t->revert_d(V);

		return {
			v.x*x.x + v.y*x.y + v.z*x.z,
			v.x*y.x + v.y*y.y + v.z*y.z,
			v.x*z.x + v.y*z.y + v.z*z.z,
		};
	}
};
