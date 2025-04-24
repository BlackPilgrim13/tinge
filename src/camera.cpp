#include "camera.h"
#include "math.h"
#include <cmath>
#include <ctime>

Ray ::Ray() {} // Parameterized constructor

Ray ::Ray(Vec3 origin, Vec3 direction)
    : origin(origin), direction(normalize(direction)) {
    inv_dir.x = 1 / direction.x;
    inv_dir.y = 1 / direction.y;
    inv_dir.z = 1 / direction.z;
} // Parameterized constructor
// Direction must be normalized while taking in

Vec3 Ray::at(float t) const {
    return origin + direction * t;
} // Should return origin + t*direction;

Camera ::Camera(float vertical_fov, int film_width, int film_height,
                float focal_length, float aperture_size)
    : vertical_fov(vertical_fov), film_width(film_width),
      film_height(film_height), focal_length(focal_length),
      aperture_size(aperture_size) {}

Camera ::~Camera() {}

Ray Camera ::generate_ray(float u, float v, Random &random_gen) {

    // Given NDC coordinates (u, v), should generate the corresponding ray
    float aspect_ratio = static_cast<float>(film_width) / film_height;
    float half_height = (focal_length)*tan((vertical_fov) / 2);
    float half_width = aspect_ratio * half_height;

    float x_coord = (2 * u - 1) * half_width;
    float y_coord = (2 * v - 1) * half_height;

    Vec3 origin(0, 0, 0); // All rays start from the pin-hole(assumed at origin)
    Vec3 direction(x_coord, y_coord, -focal_length);
    direction = (frame.frameToWorld & direction).normalized();
    origin = frame.frameToWorld * origin;
    // Position vector for the given NDC{assumed range [0,1]}

    Vec3 focal_point = Ray(origin, direction.normalized()).at(focal_length);
    Vec3 n_origin =
        origin + aperture_size * random_gen.GenerateUniformPointDisc();

    Vec3 n_direction = (focal_point - n_origin).normalized();

    return Ray(n_origin, n_direction);
}

void Camera::look_at(Vec3 from, Vec3 to) {
    Vec3 direction = to - from;
    focal_length = direction.length();
    Vec3 A = Vec3(0, 0, -1);
    Vec3 B = direction / direction.length();
    frame.origin = from;

    Vec3 axis = cross(A, B).normalized();
    float dp = dot(A, B);
    float angle = std::acos(dp);

    Mat3 K;
    Mat3 I;
    K.m[0][0] = 0;
    K.m[0][1] = -axis.z;
    K.m[0][2] = axis.y;
    K.m[1][0] = axis.z;
    K.m[1][1] = 0;
    K.m[1][2] = -axis.x;
    K.m[2][0] = -axis.y;
    K.m[2][1] = axis.x;
    K.m[2][2] = 0;

    K = I - K * std::sin(angle) + K * K * (1 - dp);
    float pitch = -std::asin(K.m[2][0]);
    float yaw = std::atan2(K.m[1][0], K.m[1][1]);
    float roll = std::atan2(K.m[2][1], K.m[2][2]);

    frame.rotation.x = -roll;
    frame.rotation.y = -pitch;
    frame.rotation.z = yaw;
    frame.lockFrame();
}
