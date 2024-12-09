#include "camera.h"

#define SPEED_DEFAULT 2.5f
#define SENSITIVITY_DEFAULT 0.1f
#define ZOOM_DEFAULT 90.0f

void camera_update_vectors(camera* c) {
    //recalculate front
    glm_quat_rotatev(c->rotation,(vec3){0,0,-1},c->Front);
    glm_vec3_normalize(c->Front);
    //right
    glm_quat_rotatev(c->rotation,(vec3){1,0,0},c->Right);
    glm_vec3_normalize(c->Right);
    //up
    glm_quat_rotatev(c->rotation,(vec3){0,1,0},c->Up);
    glm_vec3_normalize(c->Up);

    //forward
    glm_vec3_crossn(c->WorldUp,c->Right,c->Forward);
}


void camera_get_view_matrix(camera* c, mat4 dest){
    glm_quat_look(c->Position,c->rotation,dest);
}

void camera_init(camera* c,vec3 position,vec3 up, vec3 initial_look){
    glm_vec3_copy(position, c->Position);
    glm_vec3_copy(up, c->WorldUp);
    glm_vec3_copy(up, c->Up);
    c->MovementSpeed = SPEED_DEFAULT;
    c->MouseSensitivity = SENSITIVITY_DEFAULT;
    c->Zoom = ZOOM_DEFAULT;

    c->Yaw = glm_deg(atan2(initial_look[0], -initial_look[2])); // Yaw: atan2(x, -z)
    c->Pitch = glm_deg(asin(initial_look[1])); // Pitch: asin(y)

    glm_vec3_copy(initial_look,c->Front);

    glm_vec3_crossn(c->Front,c->Up,c->Right);

    //initial quaternion
    glm_quat_for(initial_look,up,c->rotation);

    camera_update_vectors(c);
}

void camera_process_input(camera *c, enum Camera_Motion m, float deltaTime){
    float velocity = c->MovementSpeed * deltaTime*20;
    switch (m){
        case FORWARD:
            //glm_vec3_muladds(c->Front, velocity, c->Position);
            glm_vec3_muladds(c->Forward, velocity, c->Position);
            break;
        case BACKWARD:
            //glm_vec3_mulsubs(c->Front, velocity, c->Position);
            glm_vec3_mulsubs(c->Forward, velocity, c->Position);
            break;
        case LEFT:
            glm_vec3_mulsubs(c->Right, velocity, c->Position);
            break;
        case RIGHT:
            glm_vec3_muladds(c->Right, velocity, c->Position);
            break;
        case UP:
            glm_vec3_muladds(c->WorldUp, velocity, c->Position);
            break;
        case DOWN:
            glm_vec3_mulsubs(c->WorldUp, velocity, c->Position);
            break;
    }
}


void camera_process_mouse(camera *c, float dx, float dy,bool constrainPitch ){
    c->Yaw   -= dx/10;
    c->Pitch += dy/10;
    if(constrainPitch)
        CLAMP(c->Pitch,-90.0f,90.0f);
    //create initial quaternion
    glm_quat_for((vec3){0,0,-1},c->WorldUp,c->rotation);

    versor yawQuat, pitchQuat;

    // Create a quaternion representing the yaw rotation (around the Y axis)
    glm_quatv(yawQuat, glm_rad(c->Yaw), (vec3){0.0f, 1.0f, 0.0f});
    glm_quat_mul(yawQuat, c->rotation, c->rotation); // Apply yaw
    glm_quat_normalize(c->rotation);
    //recalculate right vector
    glm_quat_rotatev(c->rotation,(vec3){1,0,0},c->Right);
    glm_vec3_normalize(c->Right);

    // Create a quaternion representing the pitch rotation (around the Z axis)
    glm_quatv(pitchQuat, glm_rad(c->Pitch), c->Right);
    glm_quat_mul(pitchQuat, c->rotation, c->rotation); // Apply pitch
    glm_quat_normalize(c->rotation);

    camera_update_vectors(c);
}

void camera_process_scroll(camera *c, float dy){
    c->Zoom= CLAMP(c->Zoom-dy,1.0f,135.0f);
    printf("%f\n",c->Zoom);
}
