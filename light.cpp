#pragma once

#include <functional>
#include <vector>

struct light{
	std::function<void(ray&)> get;
	std::function<color(ray&)> c;
};

std::vector<light> lights;

#define point_light(p1,p2,p3, c1,c2,c3) lights.push_back({ \
	.get = [=] (ray &r) { \
		r.d = rng::uniform(); \
		r.p = p1,p2,p3; \
		r.c = c1,c2,c3; \
	}, \
	.c = [=] (ray&) { return (color) c1,c2,c3; }, \
});

#define simple_light(v1,v2,v3,v4, c1,c2,c3) lights.push_back({ \
	.get = [=] (ray &r) { \
		r.d = rng::uniform(); \
		r.p = v1,v2,v3,v4, \
		r.c = c1,c2,c3; \
	}, \
	.c = [=] (ray&) { return (color) c1,c2,c3; }, \
});
