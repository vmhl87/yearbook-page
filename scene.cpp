#pragma once
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wunused-parameter"

void setup_camera(){
	camera.w = 1350;
	camera.h = 1800;
	camera.c = 18000;

	float ratio = 1;
	ratio = (
			std::pow(std::abs(1.0-camera.time/3.0), 4) * std::copysign(1.0, 1.0 - camera.time/3.0)
		) * 0.95 + 0.05;

	camera.p = { -20/ratio, 10, -100 };

	float orig = ((vec){ -20, 10, -100 }).mag();

	camera.p *= ratio;
	camera.c *= std::pow(camera.p.mag()/orig, 1.1);

	camera.t.z = camera.p.norm() * -1.0;
	camera.t.init();
	
	camera.spp = 32;
	camera.sync = 1;
	camera.bounces = 5;
}

color sky(vec d){
	return vec{1, 1, 1};
}

void setup_scene(){
	// ground
	
	rects.push_back({
		.t = {
			.p = { 0, -3, 0 },
			.y = { 0, 1, 0 },
			.z = { 0, 0, 1 },
		},
		.m = material::metal({.75, .75, .75}, 5),
		.w = 7, .l = 15,
	});

	// wall

	rects.push_back({
		.t = {
			.p = { 0, 0, 0 },
			.y = { 0, 0, 1 },
			.z = { 0, 1, 0 },
		},
		.m = material::diffuse({.25, .25, .8}),
		.w = 4, .l = 6,
	});

	// grid
	
	if(0) for(int x=-4; x<4; ++x)
		for(int y=-3; y<6; ++y){
			float l = 1;
			if(x>=-3&&x<=-1&&y>=-2&&y<=1) l = 0.5;
			if(x>=1&&x<=2&&y>=-1&&y<=1) l = 0.5;
			rects.push_back({
				.t = {
					.p = { x+.5, y+.5, -0.25 },
					.y = { 0, 0, 1 },
					.z = { 0, 1, 0 },
				},
				.m = material::light({l, l, l}),
				.w = .475, .l = .475,
			});
		}

	// name
	
	{
		unsigned char *name_img;
		int name_w, name_h;
		read_bmp("name.bmp", &name_img, &name_w, &name_h);

		float lx = -3,
			  ly = 3,
			  lw = 3,
			  lh = lw * (float) name_h / (float) name_w;

		masks.push_back({
			.t = {
				.p = { lx+lw/2, ly+lh/2, -0.1 },
				.y = { 0, 0, 1 },
				.z = { 0, 1, 0 },
			},
			.m = material::light({2, 2, 4}),
			.image = name_img,
			.imgh = name_h,
			.imgw = name_w,
			.shiftx = 0,
			.shifty = 0,
			.sizex = name_w,
			.sizey = name_h,
			.w = lw/2, .l = lh/2,
		});
	};

	// quote
	
	{
		int pos[5] = {494, 500, 169+15, 96, 444}, sum = 0;
		int pos2[5] = {1, 1, 0, 1, 0}, sum2 = 0;
		for(int i=0; i<5; ++i) sum += pos[i], sum2 += pos2[i];

		unsigned char *quote_img;
		int quote_w, quote_h;
		read_bmp("quote.bmp", &quote_img, &quote_w, &quote_h);
		
		int lxheight = 85;

		float t_width = 6.5,
			  t_height = t_width * (float)lxheight / (float) sum,
			  depth = 6,
			  cy = -9.5,
			  cx = 0.25 -1,
			  cz = -0.05,
			  overlap = 0.4;

		cx += cy*0.2;
		overlap *= 4;

		float left_edge = cx - t_width/2;
		float depth_acc = 0;

		for(int i=0; i<5; ++i){
			float ls_width = t_width * (float)pos[i] / (float)sum;
			masks.push_back({
				.t = {
					.p = { left_edge + ls_width/2
					+ overlap - overlap*depth_acc/(float)sum2,
						-3.0 + t_height/2 + cz,
						cy+depth/2 - depth*depth_acc/(float)sum2 },
					.y = { 0, 0, 1 },
					.z = { 0, 1, 0 },
				},
				.m = material::light({2, 2, 4}),
				.image = quote_img,
				.imgh = quote_h,
				.imgw = quote_w,
				.shiftx = 0,
				.shifty = lxheight*i,
				.sizex = pos[i],
				.sizey = lxheight,
				.w = ls_width/2, .l = t_height/2,
			});

			left_edge += ls_width;
			depth_acc += pos2[i];
		}
	}

	// photos

	{
		unsigned char *portrait_img;
		int portrait_w, portrait_h;
		read_bmp("portrait-senior.bmp", &portrait_img, &portrait_w, &portrait_h);

		fimages.push_back({
			.t = {
				.p = { -1.5, 0, -0.25 },
				.y = { 0, 0, 1 },
				.z = { 0, 1, 0 },
			},
			.image = portrait_img,
			.imgh = portrait_h,
			.imgw = portrait_w,
			.shiftx = 0,
			.shifty = 0,
			.sizex = portrait_w,
			.sizey = portrait_h,
			.w = 1.5, .l = 2,
		});
	}

	{
		unsigned char *portrait_img;
		int portrait_w, portrait_h;
		read_bmp("portrait-baby.bmp", &portrait_img, &portrait_w, &portrait_h);

		fimages.push_back({
			.t = {
				.p = { 2, 2.5, -0.25 },
				.y = { 0, 0, 1 },
				.z = { 0, 1, 0 },
			},
			.image = portrait_img,
			.imgh = portrait_h,
			.imgw = portrait_w,
			.shiftx = 0,
			.shifty = 0,
			.sizex = portrait_w,
			.sizey = portrait_h,
			.w = 1, .l = 1.5,
		});
	}

	// backlighting for photos

	{
		rects.push_back({
			.t = {
				.p = { -1.5+0.125*0.3, -0.125*0.15, -0.125 },
				.y = { 0, 0, 1 },
				.z = { 0, 1, 0 },
			},
			.m = material::light({2, 2, 3}),
			.w = 1.45, .l = 1.95,
		});

		rects.push_back({
			.t = {
				.p = { 2+0.125*0.3, 2.5-0.125*0.15, -0.125 },
				.y = { 0, 0, 1 },
				.z = { 0, 1, 0 },
			},
			.m = material::light({3, 3, 5}),
			.w = .95, .l = 1.45,
		});
	}

	// desk

	rects.push_back({
		.t = {
			.p = { 2, -2, 0 },
			.y = { 0, 1, 0 },
			.z = { 0, 0, 1 },
		},
		//.m = material::light({1, 1, 1}),
		.m = material::diffuse({1, 1, 1}),
		.w = 1, .l = 1,
	});

	// chair
	
	{
		color C = {.1, .1, .15};

		thread_local transform t = {
			.p = { 2.3, -2.5, -1 },
			.x = ((vec){ 1, 0, 0.75 }).norm(),
			.y = { 0, 1, 0 },
			.z = ((vec){ -0.75, 0, 1 }).norm(),
		};

		t.init();

		rects.push_back({
			.t = {
				.p = { 0, 0, 0 },
				.y = { 0, 1, 0 },
				.z = { 0, 0, 1 },
				.t = &t,
			},
			.m = material::diffuse(C),
			.w = 0.3, .l = 0.3,
		});

		rects.push_back({
			.t = {
				.p = { 0, 0.5, -0.3-0.5*0.3 },
				.y = ((vec){ 0, 0.3, 1 }).norm(),
				.z = ((vec){ 0, 1, -0.3 }).norm(),
				.t = &t,
			},
			.m = material::diffuse(C),
			.w = 0.3, .l = 0.51,
		});
	};

	// monitor
	
	rects.push_back({
		.t = {
			.p = { 2, -1.5, -0.3 },
			.y = { 0, 0, 1 },
			.z = { 0, 1, 0 },
		},
		.m = material::diffuse({0.025, 0.025, 0.025}),
		.w = 0.5, .l = 0.3,
	});

	// keyboard
	
	rects.push_back({
		.t = {
			.p = { 2, -2 + 0.05, -0.6 },
			.y = ((vec){ 0, 1, -0.1 }).norm(),
			.z = ((vec){ 0, 0.1, 1 }).norm(),
		},
		.m = material::light({0.25, 0.25, 0.4}),
		.w = 0.3, .l = 0.2,
	});

	// clock
	
	{
		rects.push_back({
			.t = {
				.p = { 2, -0.2, -0.05 },
				.y = { 0, 0, 1 },
				.z = ((vec){ 1, 1, 0 }).norm(),
			},
			.m = material::diffuse({0.5, 0.5, 0.75}),
			.w = 0.15, .l = 0.15,
		});

		if(0) rects.push_back({
			.t = {
				.p = { 2, -0.2, -0.05 },
				.y = { 0, 0, 1 },
				.z = { 0, 1, 0 },
			},
			.m = material::diffuse({0.5, 0.5, 0.75}),
			.w = 0.15, .l = 0.15,
		});

		rects.push_back({
			.t = {
				.p = { 2, -0.2+0.07, -0.1 },
				.y = { 0, 0, 1 },
				.z = { 0, 1, 0 },
			},
			.m = material::light({0.1, 0.1, 0.1}),
			.w = 0.01, .l = 0.07,
		});

		rects.push_back({
			.t = {
				.p = { 2+0.035, -0.2-0.035, -0.1 },
				.y = { 0, 0, 1 },
				.z = ((vec){ -1, 1, 0 }).norm(),
			},
			.m = material::light({0.1, 0.1, 0.1}),
			.w = 0.01, .l = 0.035*std::sqrt(2.0),
		});
	}

	// custom lighting
	
	/*lights.push_back({
		.get = [] (ray &r) {
			r.c = {5, .2, .2};
			r.p = rng::uniform() * .25;
			r.d = (r.p.norm()+rng::uniform()).norm();
			r.p.x += 2;
		},
		.c = [] (ray &r) {
			return color{5, .2, .2} * 16 * std::max(0.0, ((r.p-vec{2,0,0}).dot(r.d)));
		},
	});*/
}
