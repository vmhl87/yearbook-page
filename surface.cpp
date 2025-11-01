#pragma once

#include <fstream>
#include <vector>

struct sphere{
	transform t;
	material m;
	// bbox b;
	vec final_pos;

	double r;

	void init(){
		final_pos = t.apply({0, 0, 0});
		// vec p = t.apply(vec{0, 0, 0});
		// b.min = p - vec{r, r, r};
		// b.max = p + vec{r, r, r};
	}

	bool hit(ray &o, touch &res) const{
		// if(b.outside(o)) return false;
		//bool x = hit(t.revert(o.p), t.revert_d(o.d), res);
		//if(x) res.n = t.apply_d(res.n), res.p = t.apply(res.p);
		//return x;
		bool x = hit(o.p-final_pos, o.d, res);
		if(x) res.p += final_pos;
		return x;
	}

	bool hit(vec lp, vec ld, touch &res) const{
		double b = 2.0*lp.dot(ld),
			   c = lp.dot(lp)-r*r,
			   D = b*b - 4*c;

		if(D<0) return false;
		else{
			double t1 = (-b-std::sqrt(D)) / 2.0,
				   t2 = (-b+std::sqrt(D)) / 2.0;

			if(t1 > 0.001) res.d = t1;
			else if(t2 > 0.001) res.d = t2;
			else return false;

			res.n = (res.p = lp+ld*res.d).norm(),
			res.m = &m;

			return true;
		}
	}
};

std::vector<sphere> spheres;

struct rect{
	transform t;
	material m;
	bbox b;

	double w, l;

	void init(){
		t.init();

		b({
			t.apply({-w, 0, -l}),
			t.apply({w, 0, -l}),
			t.apply({-w, 0, l}),
			t.apply({w, 0, l}),
		});
	}

	bool hit(ray &o, touch &res) const{
		if(b.outside(o)) return false;
		bool x = hit(t.revert(o.p), t.revert_d(o.d), res);
		if(x) res.n = t.apply_d(res.n), res.p = t.apply(res.p);
		return x;
	}

	bool hit(vec lp, vec ld, touch &res) const{
		if(std::abs(ld.y) <= 0.001) return false;
		if(copysign(1.0, ld.y) == copysign(1.0, lp.y)) return false;
		res.d = -ld.mag()/ld.y * lp.y;

		res.p = {lp.x+ld.x*res.d, copysign(1e-6, lp.y), lp.z+ld.z*res.d};
		if(std::abs(res.p.x) > w || std::abs(res.p.z) > l) return false;

		res.n = {0, copysign(1.0, lp.y), 0};
		res.m = &m;

		return true;
	}
};

std::vector<rect> rects;

namespace _READ_BMP {

#pragma pack(push, 1)
	struct BMPFileHeader {
		uint16_t file_type;     // File identifier (should be 0x4D42 'BM')
		uint32_t file_size;     // Size of the file in bytes
		uint16_t reserved1;
		uint16_t reserved2;
		uint32_t pixel_offset;  // Offset to the start of the pixel data
	};
#pragma pack(pop)

#pragma pack(push, 1)
	struct BMPInfoHeader {
		uint32_t header_size;   // Size of this header (should be 40)
		int32_t width;          // Image width in pixels
		int32_t height;         // Image height in pixels
		uint16_t color_planes;  // Number of color planes (must be 1)
		uint16_t bits_per_pixel;// Bits per pixel (e.g., 24 for 24-bit color)
		uint32_t compression;   // Compression method (0 for uncompressed)
		uint32_t image_size;    // Image size in bytes (can be 0 for uncompressed)
		int32_t x_pixels_per_meter;
		int32_t y_pixels_per_meter;
		uint32_t total_colors;
		uint32_t important_colors;
	};
#pragma pack(pop)

};

void read_bmp(const char *filename, unsigned char **out, int *width, int *height) {
    std::ifstream file(filename, std::ios::binary);

    if(!file){
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return;
    }

	_READ_BMP::BMPFileHeader file_header;
	_READ_BMP::BMPInfoHeader info_header;

    file.read(reinterpret_cast<char*>(&file_header), sizeof(file_header));
    if(file_header.file_type != 0x4D42){
        std::cerr << "Error: Not a valid BMP file." << std::endl;
        file.close();
        return;
    }

    file.read(reinterpret_cast<char*>(&info_header), sizeof(info_header));
    if(info_header.bits_per_pixel != 24){
        std::cerr << "Error: Only 24-bit BMP images are supported." << std::endl;
        file.close();
        return;
    }

    *width = info_header.width;
    *height = info_header.height;

	unsigned char *data = new unsigned char[info_header.width*info_header.height*3];
	*out = data;

    int row_padding = (4-(info_header.width*3)%4)%4;
    // int image_row_size = info_header.width*3;

    file.seekg(file_header.pixel_offset, std::ios::beg);

	for(int y=*height-1; y>=0; --y){
		for(int x=0; x<*width; ++x){
			for(int z=0; z<3; ++z){
				unsigned char c;
				file.read(reinterpret_cast<char*>(&c), 1);
				data[3*(x+y*(*width))+z] = c;
			}
        }

        file.seekg(row_padding, std::ios::cur);
    }
    
    file.close();
}

thread_local material _mat_alt = material::diffuse({1, 1, 1});

struct fimage{
	transform t;
	bbox b;

	unsigned char *image;
	int imgh, imgw;
	int shiftx, shifty,
		sizex, sizey;

	double w, l;

	void init(){
		t.init();

		b({
			t.apply({-w, 0, -l}),
			t.apply({w, 0, -l}),
			t.apply({-w, 0, l}),
			t.apply({w, 0, l}),
		});
	}

	bool hit(ray &o, touch &res) const{
		if(b.outside(o)) return false;
		bool x = hit(t.revert(o.p), t.revert_d(o.d), res);
		if(x) res.n = t.apply_d(res.n), res.p = t.apply(res.p);
		return x;
	}

	bool hit(vec lp, vec ld, touch &res) const{
		if(std::abs(ld.y) <= 0.001) return false;
		if(copysign(1.0, ld.y) == copysign(1.0, lp.y)) return false;
		res.d = -ld.mag()/ld.y * lp.y;

		res.p = {lp.x+ld.x*res.d, copysign(1e-6, lp.y), lp.z+ld.z*res.d};

		int xcoord = std::floor((-res.p.x+w)/w/2.0 * (double) sizex);
		int ycoord = std::floor((-res.p.z+l)/l/2.0 * (double) sizey);

		if(xcoord < 0 || ycoord < 0 || xcoord >= sizex || ycoord >= sizey)
			return false;
		
		xcoord += shiftx, ycoord += shifty;
		int idx = 3*(xcoord+ycoord*imgw);

		_mat_alt.diffuse_color = {
			std::pow((float) image[idx+2] / 255, camera.gamma),
			std::pow((float) image[idx+1] / 255, camera.gamma),
			std::pow((float) image[idx] / 255, camera.gamma),
		};

		res.n = {0, copysign(1.0, lp.y), 0};
		res.m = &_mat_alt;

		return true;
	}
};

std::vector<fimage> fimages;


struct mask{
	transform t;
	material m;
	bbox b;

	unsigned char *image;
	int imgh, imgw;
	int shiftx, shifty,
		sizex, sizey;

	double w, l;

	void init(){
		t.init();

		b({
			t.apply({-w, 0, -l}),
			t.apply({w, 0, -l}),
			t.apply({-w, 0, l}),
			t.apply({w, 0, l}),
		});
	}

	bool hit(ray &o, touch &res) const{
		if(b.outside(o)) return false;
		bool x = hit(t.revert(o.p), t.revert_d(o.d), res);
		if(x) res.n = t.apply_d(res.n), res.p = t.apply(res.p);
		return x;
	}

	bool hit(vec lp, vec ld, touch &res) const{
		if(std::abs(ld.y) <= 0.001) return false;
		if(copysign(1.0, ld.y) == copysign(1.0, lp.y)) return false;
		res.d = -ld.mag()/ld.y * lp.y;

		res.p = {lp.x+ld.x*res.d, copysign(1e-6, lp.y), lp.z+ld.z*res.d};

		float xcoord = (-res.p.x+w)/w/2.0 * (float) sizex;
		float ycoord = (-res.p.z+l)/l/2.0 * (float) sizey;

		int fxcoord = std::floor(xcoord),
			fycoord = std::floor(ycoord);
		float dxcoord = xcoord - fxcoord,
			  dycoord = ycoord - fycoord;

		if(fxcoord < 0 || fycoord < 0 || fxcoord+1 >= sizex || fycoord+1 >= sizey)
			return false;

		fxcoord += shiftx, fycoord += shifty;
		float sum = 0;
		for(int a=0; a<2; ++a){
			for(int b=0; b<2; ++b){
				float S = 0;
				for(int i=0; i<3; ++i){
					S += image[3*((fxcoord+a)+(fycoord+b)*imgw)+i];
				}
				S *= a ? dxcoord : 1.0 - dxcoord;
				S *= b ? dycoord : 1.0 - dycoord;
				sum += S;
			}
		}
		if(sum > 384) return false;

		res.n = {0, copysign(1.0, lp.y), 0};
		res.m = &m;

		return true;
	}
};

std::vector<mask> masks;

#ifdef ignore

struct _obj_template{
	transform t;
	material m;
	bbox b;

	bool hit(ray &o, touch &res) const{
		if(b.outside(o)) return false;
		bool x = hit(t.revert(o.p), t.revert_d(o.d), res);
		if(x) res.n = t.apply_d(res.n), res.p = t.apply(res.p);
		return x;
	}

	bool hit(vec lp, vec ld, touch &res) const;
};

#endif

bool hit(ray &r, touch &t, bool use_light = false){
	touch alt; t.d = 1e18;
	bool res = 0;

	for(const sphere &s : spheres) if(use_light || !(s.m.T & material::LIGHT))
		if(s.hit(r, alt) && alt.d < t.d) t = alt, res |= 1;

	for(const rect &s : rects) if(use_light || !(s.m.T & material::LIGHT))
		if(s.hit(r, alt) && alt.d < t.d) t = alt, res |= 1;

	for(const fimage &s : fimages)
		if(s.hit(r, alt) && alt.d < t.d) t = alt, res |= 1;

	for(const mask &s : masks) if(use_light || !(s.m.T & material::LIGHT))
		if(s.hit(r, alt) && alt.d < t.d) t = alt, res |= 1;

	return res;
}
