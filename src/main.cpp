#include <math.h>

#include <iostream>
#include <fstream>

#include <vector>

#include <thread>
#include <barrier>
#include <condition_variable>

#include "framebuffer.h"
#include "geometry.h"
#include "obj.h"

bool ready = false;

size_t max_threads = std::thread::hardware_concurrency();
std::barrier sync_point(max_threads);
std::barrier main_barrier(max_threads + 1, [] { ready = true; });

class model {
	public:
		double angle = 0.0;
		std::vector<triangle> triangles;

		model(const std::string& filename) : triangles(load_obj(filename)) {}
};

void draw_triangle(triangle& t, framebuffer& fb, int color) {
	rect box = t.bounding_box();

	// Only need to check pixels within the bounding box
	for(int x = box.x; x <= box.x + box.w; ++x) {
		for(int y = box.y; y <= box.y + box.h; ++y) {
			// Retrieve beta/gamma barycentric coordinates
			point coordinates = t.barycentric({(double)x, (double)y});
			double beta = coordinates.x;
			double gamma = coordinates.y;
			double alpha = 1.0 - beta - gamma;

			double z = alpha * t.a.z + beta * t.b.z + gamma * t.c.z;

			double epsilon = -1e-5;
			if(alpha >= epsilon && beta >= epsilon && gamma >= epsilon) {
				fb.set_pixel(x, y, z, color);
			}
		}
	}
}

void render(framebuffer& fb, const model& obj,
			size_t start, size_t end) {
	int translate_x = fb.width() / 2, translate_y = fb.height() / 2;

	while(true) {
		// Wait for all rendering threads to arrive
		sync_point.arrive_and_wait();

		// Spin until buffers swapped
		while(ready) continue;

		for(size_t i = start; i <= start + end; ++i) {
			// Make a triangle copy
			triangle t = obj.triangles[i];

			// Apply transformations
			t.rotate(obj.angle);
			t.translate(translate_x, translate_y, 0);

			// Compute face normal
			point3d normal = t.compute_normal();

			// Map normal axes from 0-255
			normal.x = (normal.x + 1.0) / 2.0 * 255;
			normal.y = (normal.y + 1.0) / 2.0 * 255;
			normal.z = (normal.z + 1.0) / 2.0 * 255;

			int normal_rgb = ((int)normal.x << 16) | ((int)normal.y << 8) | (int)normal.z;

			draw_triangle(t, fb, normal_rgb);
		}

		main_barrier.arrive_and_wait();
	}
}

int main() {
	framebuffer fb("/dev/fb0");

	if(fb.valid()) {
		model teapot("teapot.obj");

		// Scale up vertices
		int scale = 100;
		for(triangle& t : teapot.triangles) {
			t.scale(scale);
		}

		std::vector<std::thread> threads;

		size_t size = teapot.triangles.size();
		for(size_t i = 0; i < max_threads; ++i) {
			threads.emplace_back(render, std::ref(fb), std::ref(teapot),
								 i * size / max_threads, size / max_threads);
		}

		while(true) {
			teapot.angle += 0.1;

			main_barrier.arrive_and_wait();

			// Swap framebuffer and drawing buffer
			fb.swap();

			// Clear back buffer
			fb.clear();

			ready = false;
		}

		// Wait for all threads to join
		for(size_t i = 0; i < max_threads; ++i) threads[i].join();
	} else std::cerr << fb.get_error() << "\n";

	return 0;
}
