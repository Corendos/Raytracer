#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <cmath>
#include <stdexcept>

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

    float norm() {
        return sqrtf(x * x + y * y + z * z);
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

    Vec3f operator-() const {
        return Vec3f(-x, -y, -z);
    }
};

struct Vec2f {
    Vec2f() : Vec2f(0.0f, 0.0f) {}
    Vec2f(float x, float y) {
        this->x = x;
        this->y = y;
    }

    float x;
    float y;

    Vec2f& normalize() {
        float norm = sqrtf(x * x + y * y);
        x /= norm;
        y /= norm;

        return *this;
    }

    float operator[](int index) {
        switch(index) {
            case 0:
                return x;
            case 1:
                return y;
            default:
                throw std::runtime_error("Invalid Index");
        }
    }

    Vec2f operator-(const Vec2f& b) const {
        return Vec2f(this->x - b.x, this->y);
    }

    Vec2f operator+(const Vec2f& b) const {
        return Vec2f(this->x + b.x, this->y);
    }

    float operator*(const Vec2f& b) const {
        return this->x * b.x + this->y * b.y;
    }

    Vec2f operator*(const float& f) const {
        return Vec2f(x * f, y * f);
    }

    Vec2f operator/(const float& f) const {
        return Vec2f(x / f, y / f);
    }
};

#endif