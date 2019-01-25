#include <iostream>
#include <limits>
#include <vector>
#include <fstream>
#include <cmath>
#include <omp.h>

#include "geometry.hpp"

struct Material {
    Material(const Vec3f& albedo, const Vec3f& color, const float& specular) :
        albedo(albedo), diffuseColor(color), specularExponent(specular) {}
    Material() : albedo(1, 0, 0), diffuseColor(), specularExponent() {}
    Vec3f diffuseColor;
    Vec3f albedo;
    float specularExponent;
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

    bool rayIntersect(const Vec3f& origin, const Vec3f& direction, float& t0) const {
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

bool sceneIntersect(const Vec3f& origin, const Vec3f& direction, const std::vector<Sphere>& spheres, Vec3f& hit, Vec3f& N, Material& material) {
    float sphereDist = std::numeric_limits<float>::max();
    for (const auto& sphere : spheres) {
        float dist;
        if (sphere.rayIntersect(origin, direction, dist) && dist < sphereDist) {
            sphereDist = dist;
            hit = origin + direction * dist;
            N = (hit - sphere.center).normalize();
            material = sphere.material;
        }
    }

    return sphereDist < 1000;
}

Vec3f castRay(const Vec3f& origin, const Vec3f& direction, const std::vector<Sphere>& spheres, const std::vector<Light>& lights, size_t depth = 0) {
    Vec3f point, N;
    Material material;

    if (depth > 32 || !sceneIntersect(origin, direction, spheres, point, N, material)) {
        return Vec3f(0.2, 0.7, 0.8);
    }

    Vec3f reflectDirection = reflect(direction, N).normalize();
    Vec3f reflectOrigin = reflectDirection * N < 0 ? point - N * 1e-3 : point + N * 1e-3;
    Vec3f reflectColor = castRay(reflectOrigin, reflectDirection, spheres, lights, depth + 1);

    float diffuseLightIntensity = 0, specularLightIntensity = 0;
    for (const auto& light : lights) {
        Vec3f lightDirection = (light.position - point).normalize();
        float lightDistance = (light.position - point).norm();

        Vec3f shadowOrigin = lightDirection * N < 0 ? point - N * 1e-3 : point + N * 1e-3;
        Vec3f shadowPoint, shadowN;
        Material tempMaterial;
        if (sceneIntersect(shadowOrigin, lightDirection, spheres, shadowPoint, shadowN, tempMaterial) && (shadowPoint - shadowOrigin).norm() < lightDistance) {
            continue;
        }

        diffuseLightIntensity += light.intensity * std::max(0.f, lightDirection * N);
        specularLightIntensity += powf(std::max(0.0f, -reflect(-lightDirection, N) * direction), material.specularExponent) * light.intensity;
    }

    return material.diffuseColor * diffuseLightIntensity * material.albedo[0] + Vec3f(1.0, 1.0, 1.0) * specularLightIntensity * material.albedo[1] + reflectColor * material.albedo[2];
}

void render(const std::vector<Sphere>& spheres, const std::vector<Light>& lights) {
    const int width{1920*10};
    const int height{1080*10};
    const double fov{70.0};
    std::vector<Vec3f> framebuffer(width * height);

    size_t i, j;
    #pragma omp parallel for private(i), private(j)
    for (j = 0;j < height;++j) {
        for (i = 0;i < width;++i) {
            float x = (2 * (i + 0.5) / (float)width - 1) * tan(fov/2.) * width / (float)height;
            float y = -(2 * (j + 0.5) / (float)height - 1) * tan(fov/2.);
            Vec3f dir = Vec3f(x, y, -1).normalize();
            framebuffer[j * width + i] = castRay(Vec3f(0, 0, 0), dir, spheres, lights);
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
    Material ivory(Vec3f(0.6, 0.3, 0.1), Vec3f(0.4f, 0.4f, 0.3f), 50);
    Material red_rubber(Vec3f(0.9, 0.1, 0.0), Vec3f(0.3f, 0.1f, 0.1f), 10);
    Material mirror(Vec3f(0.2, 10.0, 0.95), Vec3f(0.9f, 1.0f, 0.9f), 1425);

    std::vector<Sphere> spheres;
    spheres.push_back(Sphere(Vec3f(-3, 0, -16), 2, ivory));
    spheres.push_back(Sphere(Vec3f(-1.0, -1.5, -12), 2, mirror));
    spheres.push_back(Sphere(Vec3f(1.5, -0.5, -18), 3, red_rubber));
    spheres.push_back(Sphere(Vec3f(7, 5, -18), 4, mirror));

    std::vector<Light> lights;
    lights.push_back(Light(Vec3f(-20, 20,  20), 1.5));
    lights.push_back(Light(Vec3f( 30, 50, -25), 1.8));
    lights.push_back(Light(Vec3f( 30, 20,  30), 1.7));
    render(spheres, lights);
    return 0;
}
