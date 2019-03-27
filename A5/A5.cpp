// Winter 2019

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <pthread.h>
#include <iomanip>
#include <thread>
#include <chrono>
#include "A5.hpp"
#include "Ray.hpp"
#include "HitRecord.hpp"
#include "GeometryNode.hpp"
#include "PhongMaterial.hpp"
#include "options.hpp"


using namespace std;
using namespace glm;

static const float Correction = 0.0001;

static float rand_float() {
    float x;
    do {
        x = (float) rand() / (RAND_MAX);
    } while (x == 1);
    return x;
}

static vec3 random_in_unit_disk() {
    vec3 p;
    do {
        p = 2.0 * (vec3(rand_float(), rand_float(), 0)) - vec3(1, 1, 0);
    } while (length2(p) >= 1.0);
    return p;
}

struct ThreadData {
    SceneNode * root;
    Image & image;
    const glm::vec3 & eye;
    const glm::vec3 & ambient;
    const std::list<Light *> & lights;
    const glm::vec3 &BL_corner_direction, &_u, &_v;
    const size_t &h, &w, x_start, x_end, y_start, y_end;
    size_t &done;
};

vec3 trace_color(Ray &ray,
            SceneNode *root,
            const glm::vec3 & eye,
            const glm::vec3 & ambient,
            const std::list<Light *> & lights,
            const int maxHits = 5) {
	HitRecord record;
    vec3 color;
	// cout << "ray from " << to_string(ray.origin) << endl;
	// cout << "ray direction " << to_string(normalize(ray.direction)) << endl;

	if ( root->hit( ray, 0, numeric_limits<float>::max(), record ) ) {
		// hit
		record.normal = normalize(record.normal);
		// cout << "normal " << to_string(record.normal) << endl;
		// cout << "hit point " << to_string(record.hit_point) << endl;
		record.hit_point += record.normal * Correction;

		PhongMaterial *material = static_cast<PhongMaterial *>(record.material);

		// ambient
		color += material->diffuse() * ambient;

		for (Light * light : lights) {
			Ray shadowRay(record.hit_point, light->position - record.hit_point);
			HitRecord shadowRay_record;
			
			// if light is blocked, skip calculating 
			if (root->hit( shadowRay, 0, numeric_limits<float>::max(), shadowRay_record)) {
			// length(shadowRay_record.hit_point - record.hit_point) <= length(light->position - record.hit_point)) {
				continue;
			}

			// cout << *light << " light isn't blocked" << endl;

			vec3 L = normalize(shadowRay.Get_direction());
			vec3 N = normalize(record.normal);
			vec3 R = normalize(2 * N * dot(N, L) - L);
			vec3 V = normalize(eye - record.hit_point);
			double r = length(shadowRay.Get_direction());

		// cout << "diffuse "<< dot(L, N) << endl;
		// cout << "specular " << dot(R, V) << ", " << material->shininess() << ", " << pow(glm::max(0.0, (double)dot(R, V)), material->shininess()) << endl;
		
			double attenuation = 1.0 / ( light->falloff[0] + light->falloff[1] * r + light->falloff[2] * r * r );

			// diffuse
			color += dot(L, N) * attenuation * material->diffuse() * light->colour;

			// specular
			color += pow(glm::max(0.0, (double)dot(R, V)), material->shininess()) * attenuation * material->specular() * light->colour;
		}

		if (maxHits > 0) {
			// reflection
			vec3 reflection_direction = ray.Get_direction() - 2 * record.normal * dot(ray.Get_direction(), record.normal);
            Ray reflection_ray(record.hit_point, reflection_direction);
			// cout << "relection ray " << to_string(reflection_ray.origin) << endl;
			float reflect_coef = 0.2;
			color = glm::mix(color, trace_color(reflection_ray, root, eye, ambient, lights, maxHits - 1), reflect_coef);
		}

	} else {
		// miss -> background
		vec3 unit_direction = glm::normalize(ray.Get_direction());
        float t = unit_direction.y;
        color += (1.0 - t) * vec3(0.5, 0.4, 0.3) + t * vec3(0.0, 0.7, 1.0);
	}

	return color; 
}

void * A5_Render_Thread(void * data) {
	ThreadData* d = (ThreadData *)(data);

	for (uint x = d->x_start; x < d->x_end; ++x) {
		for (uint y = d->y_start; y < d->y_end; ++y) {
			// cout << x << " " << y << endl;
			const vec3 direction = d->BL_corner_direction + (float)(d->h - y) * d->_v + (float)x * d->_u;
			// cout << direction.x << " " << direction.y << " " << direction.z << endl;
			Ray ray = Ray(d->eye, direction);

			vec3 color;

			// color += trace_color(ray, root, eye, ambient, lights);
			size_t AA = 1;
#ifdef ENABLE_ANTI_ALIASING
			AA = 10;
#endif

            for ( unsigned int i = 0; i < AA; ++i ) { // calculate color
                 // anti aliasing
#ifdef ENABLE_ANTI_ALIASING
                    ray.direction = direction + random_in_unit_disk() * ( d->_u + d->_v ) * 0.5;
#else
                    ray.direction = direction;
#endif
                color += trace_color(ray, d->root, d->eye, d->ambient, d->lights);
            }
            color /= AA;

			// Red: 
			d->image(x, y, 0) = (double)color.r;
			// Green: 
			d->image(x, y, 1) = (double)color.g;
			// Blue: 
			d->image(x, y, 2) = (double)color.b;

			// to show progress bar
			(d->done)++;
		}
	}
}

void A5_Render(
		// What to render  
		SceneNode * root,

		// Image to write to, set to a given width and height  
		Image & image,

		// Viewing parameters  
		const glm::vec3 & eye,
		const glm::vec3 & view,
		const glm::vec3 & up,
		double fovy,

		// Lighting parameters  
		const glm::vec3 & ambient,
		const std::list<Light *> & lights
) {

  // Fill in raytracing code here...  

  std::cout << "Calling A5_Render(\n" <<
		  "\t" << *root <<
          "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
          "\t" << "eye:  " << glm::to_string(eye) << std::endl <<
		  "\t" << "view: " << glm::to_string(view) << std::endl <<
		  "\t" << "up:   " << glm::to_string(up) << std::endl <<
		  "\t" << "fovy: " << fovy << std::endl <<
          "\t" << "ambient: " << glm::to_string(ambient) << std::endl <<
		  "\t" << "lights{" << std::endl;

	for(const Light * light : lights) {
		std::cout << "\t\t" <<  *light << std::endl;
	}
	std::cout << "\t}" << std::endl;
	std:: cout <<")" << std::endl;

	size_t h = image.height();
	size_t w = image.width();

	 // translate viewport

	vec3 _w = normalize(view); // z-axis
	vec3 _u = normalize(cross(_w, up)); // x-axis
	vec3 _v = cross(_u, _w); // y-axis
	float d = h / 2 / glm::tan(glm::radians(fovy / 2));
	vec3 BL_corner_direction = _w * d - _u * (float)w / 2 - _v * (float)h / 2;


	// {
	// 	const vec3 d = BL_corner_direction + (float)(h - 140) * _v + (float) 157 * _u;
	// 	Ray ray = Ray(eye, d);

	// 	trace_color(ray, root, eye, ambient, lights);

	// 	exit(1);
	// }


	// cout << BL_corner_direction.x << " " << BL_corner_direction.y << " " << BL_corner_direction.z << endl;
	// for (uint y = 0; y < h; ++y) {
	// 	for (uint x = 0; x < w; ++x) {
	// 		// cout << x << " " << y << endl;
	// 		const vec3 direction = BL_corner_direction + (float)(h - y) * _v + (float)x * _u;
	// 		// cout << direction.x << " " << direction.y << " " << direction.z << endl;
	// 		Ray ray = Ray(eye, direction);

	// 		vec3 color;

	// 		// color += trace_color(ray, root, eye, ambient, lights);
	// 		size_t AA = 1;
	// 		if (ENABLE_ANTI_ALIASING == 1) {
	// 			size_t AA = 10;
	// 		}

    //         for ( unsigned int i = 0; i < AA; ++i ) { // calculate color
    //              // anti aliasing
	// 			if (ENABLE_ANTI_ALIASING == 1) {
    //                 ray.direction = direction + random_in_unit_disk() * ( _u + _v ) * 0.5;
	// 			} else {
    //                 ray.direction = direction;
    //             }

    //             color += trace_color(ray, root, eye, ambient, lights);
    //         }
    //         color /= AA;

	// 		// Red: 
	// 		image(x, y, 0) = (double)color.r;
	// 		// Green: 
	// 		image(x, y, 1) = (double)color.g;
	// 		// Blue: 
	// 		image(x, y, 2) = (double)color.b;
	// 	}
	// }

#ifdef ENABLE_MULTITHREAD
    const size_t num_threads = 16;
#else
    const size_t num_threads = 1;
#endif
	pthread_t threads[num_threads];
    size_t progress[num_threads];
    ThreadData * datas[num_threads];

	size_t x_per_thread = w / (uint)sqrt(num_threads);

	size_t y_per_thread = h / (uint)sqrt(num_threads);

    for ( uint i = 0; i < (uint)sqrt(num_threads); i++ ) {
		for (uint j = 0; j < (uint)sqrt(num_threads); j++) {
			progress[i * (uint)sqrt(num_threads) + j] = 0;
			datas[i * (uint)sqrt(num_threads) + j] = new ThreadData{root, image, eye, ambient, lights, BL_corner_direction, _u, _v, h, w,
				i * x_per_thread, i == (uint)sqrt(num_threads) - 1 ? w : (i + 1) * x_per_thread,
				j * y_per_thread, j == (uint)sqrt(num_threads) - 1 ? h : (j + 1) * y_per_thread,
				progress[i * (uint)sqrt(num_threads) + j]};
			cout << "creating thread " << i * (uint)sqrt(num_threads) + j << endl;
			int ret = pthread_create(&threads[i * (uint)sqrt(num_threads) + j], NULL, A5_Render_Thread, datas[i * (uint)sqrt(num_threads) + j]);
			if ( ret ) {
				cerr << "Abort: pthread_create failed with error code: " << ret << endl;
				exit(EXIT_FAILURE);
			}
		}
    }

	size_t total = h * w;
    auto begin = time(NULL);
    for ( ;; ) {
        size_t done = 0;
        for ( uint i = 0; i < num_threads; ++i ) {
            done += progress[i];
        }
        if ( done == total ) break;

        // if ( ( done % 1000 ) == 0 ) {
            auto end = time(NULL);
            int elapsed_secs = end - begin;
            int eta_secs = done ? int(float(elapsed_secs) * (total - done) / done) : numeric_limits<int>::max();
            int digits = to_string(total).length();
            cerr << fixed << setprecision(2) << setfill('0') << setw(5) << 100 * float(done) / total << setfill(' ');
            cerr << "%: " << setw(digits) << done << " / " << total << " | ";
            int a, b, c;
            a = elapsed_secs / 3600; b = elapsed_secs % 3600 / 60; c = elapsed_secs % 60;
            cerr << "elapsed: " << setfill('0') << setw(2) << a << ":" << setw(2) << b << ":" << setw(2) << c << " | ";
            a = eta_secs / 3600; b = eta_secs % 3600 / 60; c = eta_secs % 60;
            cerr << "eta: " << setw(2) << a << ":" << setw(2) << b << ":" << setw(2) << c << " | " << endl << setfill(' ');
        // }
        // pthread_yield();
        std::this_thread::sleep_for (std::chrono::seconds(1));
    }

    for ( uint i = 0; i < num_threads; ++i ) {
        pthread_join( threads[i], NULL );
        delete datas[i];
    }
}
