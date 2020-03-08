#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <cmath>
#include <stdexcept>

struct Vec4f {
    Vec4f() : Vec4f(0.0f, 0.0f, 0.0f, 0.0f) {}
    Vec4f(float x, float y, float z, float w) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
    }

    float x;
    float y;
    float z;
    float w;

    Vec4f& normalize() {
	float norm = sqrtf(x * x + y * y + z * z + w * w);
	x /= norm;
	y /= norm;
	z /= norm;
	w /= norm;

	return *this;
    }

    float norm() {
	return sqrtf(x * x + y * y + z * z + w * w);
    }

    float operator[](int index) {
	switch(index) {
	    case 0:
		return x;
	    case 1:
		return y;
	    case 2:
		return z;
	    case 3:
		return w;
	    default:
		throw std::runtime_error("Invalid Index");
	}
    }

    Vec4f operator-(const Vec4f& b) const {
	return Vec4f(this->x - b.x, this->y - b.y, this->z - b.z, this->w - b.w);
    }

    Vec4f operator+(const Vec4f& b) const {
	return Vec4f(this->x + b.x, this->y + b.y, this->z + b.z, this->w - b.w);
    }

    float operator*(const Vec4f& b) const {
	return this->x * b.x + this->y * b.y + this->z * b.z + this->w * b.w;
    }

    Vec4f operator*(const float& f) const {
	return Vec4f(x * f, y * f, z * f, w * f);
    }

    Vec4f operator/(const float& f) const {
	return Vec4f(x / f, y / f, z / f, w / f);
    }

    Vec4f operator-() const {
	return Vec4f(-x, -y, -z, -w);
    }
};

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

Vec3f cross(Vec3f& a, Vec3f& b) {
    Vec3f result;

    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;

    return result;
}

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
