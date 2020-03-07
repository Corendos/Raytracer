#include <iostream>
#include <limits>
#include <vector>
#include <fstream>
#include <cmath>
#include <omp.h>

#include "geometry.hpp"

struct Material {
    Material(const float& refraction_index, const Vec4f& albedo, const Vec3f& color, const float& specular) :
	albedo(albedo), diffuse_color(color), specular_exponent(specular), refraction_index(refraction_index) {}
    Material() : albedo(1, 0, 0, 0), diffuse_color(), specular_exponent() {}
    Vec3f diffuse_color;
    Vec4f albedo;
    float specular_exponent;
    float refraction_index;
};

struct Light {
    Light(const Vec3f& position, const float& intensity) : position(position), intensity(intensity) {}
    Vec3f position;
    float intensity;
};

struct Sphere {
    Vec3f center;
    float radius;
    Material material;

    Sphere(const Vec3f& c, const float& r, const Material& m) : center(c), radius(r), material(m) {}

    bool ray_intersect(const Vec3f& origin, const Vec3f& direction, float& t0) const {
	Vec3f L = center - origin;
	float tca = L * direction;
	float d2 = L * L - tca * tca;
	if (d2 > radius * radius) return false;
	float thc = sqrtf(radius * radius - d2);
	t0 = tca - thc;
	float t1 = tca + thc;
	if (t0 < 0) t0 = t1;
	if (t0 < 0) return false;
	return true;
    }
};

Vec3f reflect(const Vec3f& incident, const Vec3f& N) {
    return incident - N * 2.0f * (incident * N);
}

Vec3f refract(const Vec3f& incident, const Vec3f& N, const float& refractive_index) {
    float cosi = -std::max(-1.f, std::min(1.f, incident * N));
    float etai = 1, etat = refractive_index;

    Vec3f n = N;
    if (cosi < 0) {
	cosi = -cosi;
	std::swap(etai, etat);
	n = -N;
    }

    float eta = etai / etat;
    float k = 1 - eta * eta * (1 - cosi * cosi);
    return k < 0 ? Vec3f(0, 0, 0) : incident * eta + n * (eta * cosi - sqrtf(k));
}

bool scene_intersect(const Vec3f& origin, const Vec3f& direction, const std::vector<Sphere>& spheres, Vec3f& hit, Vec3f& N, Material& material) {
    float sphere_dist = std::numeric_limits<float>::max();
    for (const auto& sphere : spheres) {
	float dist;
	if (sphere.ray_intersect(origin, direction, dist) && dist < sphere_dist) {
	    sphere_dist = dist;
	    hit = origin + direction * dist;
	    N = (hit - sphere.center).normalize();
	    material = sphere.material;
	}
    }

    float checkerboard_distance = std::numeric_limits<float>::max();
    if (fabs(direction.y) > 1e-3) {
      float d = -(origin.y + 4) / direction.y;

      Vec3f pt = origin + direction * d;

      if (d > 0 && fabs(pt.x) < 20 && pt.z < -10 && pt.z > -50 && d < sphere_dist) {
	checkerboard_distance = d;
	hit = pt;
	N = Vec3f(0, 1, 0);
	material.diffuse_color = (((int)(0.5 * hit.x + 1000) + (int)(0.5 * hit.z)) & 1 ? Vec3f(1, 1, 1) : Vec3f(1, .3, .7)) * 0.3;
      }
    }

    return std::min(sphere_dist, checkerboard_distance) < 1000;
}

Vec3f cast_ray(const Vec3f& origin, const Vec3f& direction, const std::vector<Sphere>& spheres, const std::vector<Light>& lights, size_t depth = 0) {
    Vec3f point, N;
    Material material;

    if (depth > 6 || !scene_intersect(origin, direction, spheres, point, N, material)) {
	return Vec3f(0.2, 0.7, 0.8);
    }

    Vec3f reflect_direction = reflect(direction, N).normalize();
    Vec3f reflect_origin = reflect_direction * N < 0 ? point - N * 1e-3 : point + N * 1e-3;
    Vec3f reflect_color = cast_ray(reflect_origin, reflect_direction, spheres, lights, depth + 1);

    Vec3f refract_direction = refract(direction, N, material.refraction_index).normalize();
    Vec3f refract_origin = refract_direction * N < 0 ? point - N * 1e-3 : point + N * 1e-3;
    Vec3f refract_color = cast_ray(refract_origin, refract_direction, spheres, lights, depth + 1);

    float diffuse_light_intensity = 0, specular_light_intensity = 0;
    for (const auto& light : lights) {
	Vec3f light_direction = (light.position - point).normalize();
	float light_distance = (light.position - point).norm();

	Vec3f shadow_origin = light_direction * N < 0 ? point - N * 1e-3 : point + N * 1e-3;
	Vec3f shadow_point, shadow_n;
	Material temp_material;
	if (scene_intersect(shadow_origin, light_direction, spheres, shadow_point, shadow_n, temp_material) && (shadow_point - shadow_origin).norm() < light_distance) {
	    continue;
	}

	diffuse_light_intensity += light.intensity * std::max(0.f, light_direction * N);
	specular_light_intensity += powf(std::max(0.0f, -reflect(-light_direction, N) * direction), material.specular_exponent) * light.intensity;
    }

    return material.diffuse_color * diffuse_light_intensity * material.albedo[0] +
						  Vec3f(1.0, 1.0, 1.0) * specular_light_intensity * material.albedo[1] +
						  reflect_color * material.albedo[2] +
						  refract_color * material.albedo[3];
}

void render(const std::vector<Sphere>& spheres, const std::vector<Light>& lights) {
    const int width{1920 * 1};
    const int height{1080 * 1};
    const double fov{70.0};
    std::vector<Vec3f> framebuffer(width * height);

    size_t i, j;
    #pragma omp parallel for private(i), private(j)
    for (j = 0;j < height;++j) {
	for (i = 0;i < width;++i) {
	    float x = (2 * (i + 0.5) / (float)width - 1) * tan(fov/2.) * width / (float)height;
	    float y = -(2 * (j + 0.5) / (float)height - 1) * tan(fov/2.);
	    Vec3f dir = Vec3f(x, y, -1).normalize();
	    framebuffer[j * width + i] = cast_ray(Vec3f(0, 0, 0), dir, spheres, lights);
	}
    }

    std::ofstream ofs;
    ofs.open("./out.ppm");
    ofs << "P6\n" << width << " " << height << "\n255\n";

    for (size_t i{0};i < width * height;++i) {
	for (size_t j{0};j < 3;++j) {
	    Vec3f& c = framebuffer[i];
	    float max = std::max(c[0], std::max(c[1], c[2]));
	    if (max > 1) c = c * (1. / max);
	    ofs << (char)(255 * std::max(0.0f, std::min(1.f, framebuffer[i][j])));
	}
    }
    ofs.close();
}

int main(int, char**) {
    Material ivory(1.0, Vec4f(0.6, 0.3, 0.1, 0.0), Vec3f(0.4f, 0.4f, 0.3f), 50);
    Material glass(1.05, Vec4f(0.1, 0.9, 0.1, 0.8), Vec3f(0.9f, 0.1f, 0.1f), 1205);
    Material red_rubber(1.0, Vec4f(0.9, 0.1, 0.0, 0.0), Vec3f(0.3f, 0.1f, 0.1f), 10);
    Material mirror(1.0, Vec4f(0.0, 10.0, 0.8, 0.0), Vec3f(1.f, 1.0f, 1.f), 1425);

    std::vector<Sphere> spheres;
    spheres.push_back(Sphere(Vec3f(-3, 0, -16), 2, ivory));
    spheres.push_back(Sphere(Vec3f(-1.0, -1.5, -12), 2, glass));
    spheres.push_back(Sphere(Vec3f(1.5, -0.5, -18), 3, red_rubber));
    spheres.push_back(Sphere(Vec3f(7, 5, -18), 4, mirror));

    std::vector<Light> lights;
    lights.push_back(Light(Vec3f(-20, 20,  20), 1.5));
    lights.push_back(Light(Vec3f( 30, 50, -25), 1.8));
    lights.push_back(Light(Vec3f( 30, 20,  30), 1.7));
    render(spheres, lights);
    return 0;
}
