#include <iostream>
#include <limits>
#include <vector>
#include <fstream>
#include <cmath>

struct Vec3f {
    Vec3f() : Vec3f(0.0f, 0.0f, 0.0f) {}
    Vec3f(float x, float y, float z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    float x;
    float y;
    float z;

    Vec3f& normalize() {
        float norm = sqrtf(x * x + y * y + z * z);
        x /= norm;
        y /= norm;
        z /= norm;

        return *this;
    }

    float operator[](int index) {
        switch(index) {
            case 0:
                return x;
            case 1:
                return y;
            case 2:
                return z;
            default:
                throw std::runtime_error("Invalid Index");
        }
    }

    Vec3f operator-(const Vec3f& b) const {
        return Vec3f(this->x - b.x, this->y - b.y, this->z - b.z);
    }

    Vec3f operator+(const Vec3f& b) const {
        return Vec3f(this->x + b.x, this->y + b.y, this->z + b.z);
    }

    float operator*(const Vec3f& b) const {
        return this->x * b.x + this->y * b.y + this->z * b.z;
    }

    Vec3f operator*(const float& f) const {
        return Vec3f(x * f, y * f, z * f);
    }

    Vec3f operator/(const float& f) const {
        return Vec3f(x / f, y / f, z / f);
    }
};

struct Material {
    Material(const Vec3f& color) : diffuseColor(color) {}
    Material() : diffuseColor() {}
    Vec3f diffuseColor;
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

Vec3f castRay(const Vec3f& origin, const Vec3f& direction, const std::vector<Sphere>& spheres, const std::vector<Light>& lights) {
    Vec3f point, N;
    Material material;

    if (!sceneIntersect(origin, direction, spheres, point, N, material)) {
        return Vec3f(0.2, 0.7, 0.8);
    }

    float diffuseLightIntensity = 0;
    for (const auto& light : lights) {
        Vec3f lightDirection = (light.position - point).normalize();
        diffuseLightIntensity += light.intensity * std::max(0.f, lightDirection * N);
    }

    return material.diffuseColor * diffuseLightIntensity;
}

void render(const std::vector<Sphere>& spheres, const std::vector<Light>& lights) {
    const int width{1920*1};
    const int height{1080*1};
    const double fov{70.0};
    std::vector<Vec3f> framebuffer(width * height);

    for (size_t j{0};j < height;++j) {
        for (size_t i{0};i < width;++i) {
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
            ofs << (char)(255 * std::max(0.0f, std::min(1.f, framebuffer[i][j])));
        }   
    }
    ofs.close();
}

int main(int, char**) {
    Material mat1(Vec3f(1.0f, 0.0f, 0.0f));
    Material mat2(Vec3f(0.0f, 0.0f, 1.0f));

    std::vector<Sphere> spheres;
    spheres.push_back(Sphere(Vec3f(-3, 0, -16), 2, mat1));
    spheres.push_back(Sphere(Vec3f(-2, 0, -16), 2, mat2));

    std::vector<Light> lights;
    lights.push_back(Light(Vec3f(-20, 20, 20), 1.5));
    render(spheres, lights);
    return 0;
}
