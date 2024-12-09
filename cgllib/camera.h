#pragma once
#include <cglm/cglm.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define CLAMP(n,l,h) ((n)>(h))?(h):((n)<(l))?(l):(n)


typedef struct{
    // Camera Attributes
    vec3 Position;
    vec3 Front;
    vec3 Up;
    vec3 Right;
    vec3 WorldUp;

    vec3 Forward;

    // Euler Angles
    float Yaw;
    float Pitch;
    // Quaternion rotation
    versor rotation;
    // Camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
}camera;


enum Camera_Motion {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN,
};

void camera_get_view_matrix(camera* c, mat4 dest);

void camera_init(camera* c,vec3 position,vec3 up, vec3 initial_look);

void camera_process_input(camera *c, enum Camera_Motion m, float deltaTime);

void camera_process_mouse(camera *c, float dx, float dy, bool constrainPitch);

void camera_process_scroll(camera *c, float dy);