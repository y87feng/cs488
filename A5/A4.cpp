// Winter 2019

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include "A4.hpp"
#include "Ray.hpp"
#include "HitRecord.hpp"
#include "GeometryNode.hpp"
#include "PhongMaterial.hpp"
#include "options.hpp"

using namespace std;
using namespace glm;

static const float Correction = 0.0001;

float rand_float() {
    float x;
    do {
        x = (float) rand() / (RAND_MAX);
    } while (x == 1);
    return x;
}

vec3 random_in_unit_disk() {
    vec3 p;
    do {
        p = 2.0 * (vec3(rand_float(), rand_float(), 0)) - vec3(1, 1, 0);
    } while (length2(p) >= 1.0);
    return p;
}

vec3 trace_color(Ray &ray,
            SceneNode *root,
            const glm::vec3 & eye,
            const glm::vec3 & ambient,
            const std::list<Light *> & lights,
            const int maxHits = 5) {
	HitRecord record;
    vec3 color;
	// if (maxHits != 5) cout << "reflect " << maxHits << " \n"; 
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
			
			// if light is blocked, skip calculating colhit_pointor
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

void A4_Render(
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

  std::cout << "Calling A4_Render(\n" <<
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
	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
			// cout << x << " " << y << endl;
			const vec3 direction = BL_corner_direction + (float)(h - y) * _v + (float)x * _u;
			// cout << direction.x << " " << direction.y << " " << direction.z << endl;
			Ray ray = Ray(eye, direction);

			vec3 color;

			// color += trace_color(ray, root, eye, ambient, lights);
			size_t AA = 1;
			if (ENABLE_ANTI_ALIASING == 1) {
				size_t AA = 10;
			}

            for ( unsigned int i = 0; i < AA; ++i ) { // calculate color
                 // anti aliasing
				if (ENABLE_ANTI_ALIASING == 1) {
                    ray.direction = direction + random_in_unit_disk() * ( _u + _v ) * 0.5;
				} else {
                    ray.direction = direction;
                }

                color += trace_color(ray, root, eye, ambient, lights);
            }
            color /= AA;

			// Red: 
			image(x, y, 0) = (double)color.r;
			// Green: 
			image(x, y, 1) = (double)color.g;
			// Blue: 
			image(x, y, 2) = (double)color.b;
		}
	}

}
