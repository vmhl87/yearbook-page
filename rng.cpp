#pragma once

#include <climits>
#include <random>
#include <cmath>

namespace rng{
	std::random_device rd;
	std::default_random_engine gen2(rd());
	thread_local std::default_random_engine gen(rd());
	std::uniform_real_distribution<double> uniform_gen(0.0, 1.0);
	std::normal_distribution<double> gaussian_gen(0.0, 1.0);

	inline double base(){
		return uniform_gen(gen);
	}

	inline double base2(){
		return uniform_gen(gen2);
	}

	inline double norm(){
		return gaussian_gen(gen);
	}

	inline double norm2(){
		return gaussian_gen(gen2);
	}

	vec uniform(){
		double u = base() * 2.0 - 1.0;
		double phi = base() * 2.0 * M_PI;
		double sqrt_1_minus_u2 = sqrt(1.0 - u * u);

		return {
			sqrt_1_minus_u2 * cos(phi),
			sqrt_1_minus_u2 * sin(phi),
			u
		};
	}

	vec uniform2(){
		double u = base2() * 2.0 - 1.0;
		double phi = base2() * 2.0 * M_PI;
		double sqrt_1_minus_u2 = sqrt(1.0 - u * u);

		return {
			sqrt_1_minus_u2 * cos(phi),
			sqrt_1_minus_u2 * sin(phi),
			u
		};
	}

	vec gaussian(){
		return uniform() * norm();
	}

	vec gaussian2(){
		return uniform2() * norm2();
	}

#ifdef ignore

	inline double IGN(double x, double y, double Z){
		double z = Z*5.588238f;
		return std::fmod(
			52.9829189f * std::fmod(0.06711056f*(x+z) + 0.00583715f*(y+z), 1.0),
			1.0);
	}

#endif

}
