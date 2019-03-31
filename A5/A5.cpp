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
#include "random.hpp"


using namespace std;
using namespace glm;

static const float Correction = 0.001;

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
            const int maxHits = 2,
			const int refraction_counter = 0) {
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

			float soft_shadow_coef = 1;

#ifdef ENABLE_SOFTSHADOW
			// distrubute lights in a sphere
			int area_lights = 10+1;
			int hits = 0;
			for (int i = 0; i < area_lights-1; i++) {
				HitRecord area_shadowRay_record;
				vec3 light_pos = vec3(light->position.x + 50 * (rand_float() - 0.5f),
										light->position.y + 50 * (rand_float() - 0.5f),
										light->position.z + 50 * (rand_float() - 0.5f));
				Ray area_shadowRay(record.hit_point, light_pos - record.hit_point);
				if (root->hit( area_shadowRay, 0, numeric_limits<float>::max(), area_shadowRay_record)) {
					hits++;
				}
			}

			soft_shadow_coef = (area_lights - hits) * 1.0f / area_lights;
			// if (soft_shadow_coef != 1.0f) cout << "soft shadow coef is " << soft_shadow_coef << endl; 
#endif

			// cout << *light << " light isn't blocked" << endl;

			vec3 L = normalize(shadowRay.Get_direction());
			vec3 N = record.normal;
			vec3 R = normalize(2 * N * dot(N, L) - L);
			vec3 V = normalize(eye - record.hit_point);
			double r = length(shadowRay.Get_direction());

		// cout << "diffuse "<< dot(L, N) << endl;
		// cout << "specular " << dot(R, V) << ", " << material->shininess() << ", " << pow(glm::max(0.0, (double)dot(R, V)), material->shininess()) << endl;
		
			double attenuation = 1.0 / ( light->falloff[0] + light->falloff[1] * r + light->falloff[2] * r * r );

			// diffuse
			color += dot(L, N) * attenuation * material->diffuse() * light->colour * soft_shadow_coef;

			// specular
			color += pow(glm::max(0.0, (double)dot(R, V)), material->shininess()) * attenuation * material->specular() * light->colour * soft_shadow_coef;
		}

		if (maxHits > 0) {
			PhongMaterial* pm = dynamic_cast<PhongMaterial*>(record.material);
			if (pm != nullptr) {
				// reflection
				ray.direction = normalize(ray.Get_direction());
				float cos_theta = dot(ray.Get_direction(), record.normal);
				vec3 refleciton_color = vec3(0.0f);
#ifdef ENABLE_REFLECTION
				vec3 reflection_direction = ray.Get_direction() - 2 * record.normal * cos_theta;
				Ray reflection_ray(record.hit_point, reflection_direction);
				// cout << "relection ray " << to_string(reflection_ray.origin) << endl;
				// float reflect_coef = 0.2;
				refleciton_color = trace_color(reflection_ray, root, eye, ambient, lights, maxHits - 1);
#ifdef ENABLE_GLOSSY_REFLECTION
				reflection_direction = normalize(reflection_direction);
				vec3 reflect_orthonormal_u = cross(reflection_direction, record.normal);
				vec3 reflect_orthonormal_v = cross(reflect_orthonormal_u, reflection_direction);
				float reflection_a = 0.5f; // 2D square to blur
				srand(0);
				int reflective_rays = rand() % 10 + 1;
				// cout << "reflective_rays " << reflective_rays << endl;
				refleciton_color /= reflective_rays;
				for (int i = 0; i < reflective_rays - 1; i++) {
					float u = -reflection_a/2 + rand_float() * reflection_a;
					float v = -reflection_a/2 + rand_float() * reflection_a;

					vec3 glossy_reflection = reflection_direction + u * reflect_orthonormal_u + v * reflect_orthonormal_v;
					glossy_reflection = normalize(glossy_reflection);
					float cosi = dot(glossy_reflection, reflection_direction);
					Ray glossy_reflection_ray(record.hit_point, glossy_reflection);
					// cout << to_string(glossy_reflection) << " weight " << cosi << endl;
					// cout << i << " is " << to_string(refleciton_color) << endl;
					refleciton_color += cosi / reflective_rays * trace_color(glossy_reflection_ray, root, eye, ambient, lights, 0, refraction_counter);
				}
				// cout << to_string(refleciton_color) << endl;
#endif
#endif
				// refraction 
#ifdef ENABLE_REFRACTION
				vec3 refraciton_color;
				// if (refraction_counter % 2 == 1 && direction != vec3(0.0f)) {
				// 	// cout << refraction_counter << " straight back " << to_string(direction) << " \n";
				// 	Ray refraction_ray(record.hit_point - 2 * record.normal * Correction, direction);
				// 	refraciton_color = trace_color(refraction_ray, root, eye, ambient, lights, maxHits - 1, direction, refraction_counter + 1);
				// } else {
				double eta = 0.8f;
				if (refraction_counter % 2 == 1) eta = 1/eta;
				float c1 = -cos_theta;
				float c2 = (1-pow(eta,2)*(1-pow(c1,2)));
				// cout << "c2 is " << c2 << endl;
				if (c2 <= 0) return color;
				vec3 refraction_direction = eta * ray.Get_direction() + (eta * c1 - sqrt(c2)) * record.normal;
				Ray refraction_ray(record.hit_point - 2 * record.normal * Correction, refraction_direction);
				// cout << refraction_counter << " refraction " << c1 << " " << c2 << " " << to_string(refraction_direction) << " " <<
				// to_string(ray.direction) << " \n";
				refraciton_color = trace_color(refraction_ray, root, eye, ambient, lights, maxHits - 1, refraction_counter + 1);
				// }

#ifdef ENABLE_GLOSSY_REFRACTION
				refraction_direction = normalize(refraction_direction);
				// cout << to_string(refraction_direction) << endl;
				vec3 refract_orthonormal_u = cross(refraction_direction, -record.normal);
				vec3 refract_orthonormal_v = cross(refract_orthonormal_u, refraction_direction);
				float refract_a = 0.5f;

				int refractive_rays = rand() % 10 + 1;
				// cout << "refractive_rays " << refractive_rays << endl;
				refraciton_color /= refractive_rays;
				for (int i = 0; i < refractive_rays - 1; i++) {
					float u = -refract_a/2 + rand_float() * refract_a;
					float v = -refract_a/2 + rand_float() * refract_a;

					vec3 glossy_refraction = refraction_direction + u * refract_orthonormal_u + v * refract_orthonormal_v;
					glossy_refraction = normalize(glossy_refraction);
					float cosi = dot(glossy_refraction, refraction_direction);
					Ray glossy_refraction_ray(record.hit_point - 2 * record.normal * Correction, glossy_refraction);
					// cout << to_string(glossy_refraction) << " weight " << cosi << endl;
					// cout << i << " is " << to_string(refraction_direction) << endl;
					refraciton_color += cosi / refractive_rays * trace_color(glossy_refraction_ray, root, eye, ambient, lights, 0, refraction_counter);
				}
#endif

#endif
				float reflect_coef = 0.2f;
				float refract_coef = 0.4f;
				// cout << " color is " << to_string(color) << endl;
				color = (1-reflect_coef-refract_coef) * color + reflect_coef * refleciton_color + refract_coef * refraciton_color;
				// cout << " color is " << to_string(color) << endl;
			}
		}

	} else {
		// miss -> background
		vec3 unit_direction = glm::normalize(ray.Get_direction());
        float t = unit_direction.y;
        color += (1.0 - t) * vec3(0.9, 0.8, 0.7) + t * vec3(0.0, 0.7, 1.0);
	}

	return color; 
}

void * A5_Render_Thread(void * data) {
	ThreadData* d = (ThreadData *)(data);

	for (uint x = d->x_start; x < d->x_end; ++x) {
		for (uint y = d->y_start; y < d->y_end; ++y) {
			// cout << x << " " << y << endl;
			vec3 direction = d->BL_corner_direction + (float)(d->h - y) * d->_v + (float)x * d->_u;

#ifdef ENABLE_DEPTH_OF_FIELD
			vec3 color;

			int random_eye_pos = 10 + 1;

			float focal_plane = 1000.0f; // relative to eye position
			for (int i = 0; i < random_eye_pos; i++) {
				vec3 relative_move = vec3((rand_float()-0.5f)*20, (rand_float()-0.5f)*20, 0);
				vec3 eye_pos = d->eye + relative_move;
				float ratio = (direction.z-focal_plane) / direction.z;
				vec3 focal_direction = direction * ratio;

				focal_direction = focal_direction - relative_move;

				Ray ray = Ray(eye_pos, focal_direction);
				color += trace_color(ray, d->root, eye_pos, d->ambient, d->lights) / random_eye_pos;
			}
			

			
			// color += trace_color(ray, root, eye, ambient, lights);
// 			size_t AA = 1;
// #ifdef ENABLE_ANTI_ALIASING
// 			AA = 10;
// #endif

//             for ( unsigned int i = 0; i < AA; ++i ) { // calculate color
//                  // anti aliasing
// #ifdef ENABLE_ANTI_ALIASING
//                     ray.direction = direction + random_in_unit_disk() * ( d->_u + d->_v ) * 0.5;
// #else
//                     ray.direction = direction;
// #endif
//                 color += trace_color(ray, d->root, d->eye, d->ambient, d->lights);
//             }
//             color /= AA;

#else // ENABLE_DEPTH_OF_FIELD
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
#endif // end ENABLE_DEPTH_OF_FIELD

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


	{
		vec3 direction = BL_corner_direction + (float)(h - 104) * _v + (float) 101 * _u;
		// vec3 direction = d->BL_corner_direction + (float)(d->h - y) * d->_v + (float)x * d->_u;
		vec3 color;

		int random_eye_pos = 10 + 1;

		float focal_plane = 1000.0f;
		for (int i = 0; i < random_eye_pos; i++) {
			vec3 relative_move = vec3((rand_float()-0.5f)*20, (rand_float()-0.5f)*20, 0);
			vec3 eye_pos = eye + relative_move;

			cout << "relative move " << to_string(relative_move) << endl;
			cout << "eye pos " << to_string(eye_pos) << endl;
			cout << "direction is " << to_string(direction) << endl;

			float ratio = (direction.z-focal_plane) / direction.z;
			vec3 new_direction = direction * ratio;

			cout << "new direction is " << to_string(new_direction) << endl;

			new_direction = new_direction - relative_move;

			Ray ray = Ray(eye_pos, new_direction);
			auto tmpcolor = trace_color(ray, root, eye_pos, ambient, lights);

			color += tmpcolor / random_eye_pos;
			cout << " color " << to_string(tmpcolor) << " \n";
		}
		// const vec3 d = BL_corner_direction + (float)(h - 104) * _v + (float) 101 * _u;
		Ray ray = Ray(eye, direction);

		auto color1 = trace_color(ray, root, eye, ambient, lights);
		cout << " color1 " << to_string(color1) << " \n";

		// exit(1);
	}


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
    const size_t num_threads = 64;
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
