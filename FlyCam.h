#pragma once
#include "maths_funcs.h"
#include "input.h"

//This code is held together with hopes and dreams
//I got it working by jigging things about till it gave
//the desired behaviour, some parts don't make sense to me
//No warranty is implied, this code is not recommended for reuse

//IT'S FUCKED
//NONE OF THIS SHOULD WORK
//I HATE ROTATIONS
//JUST GO WITH IT UNTIL IT BREAKS
//IT'S FINE
//EVERYTHING'S FINE

struct FlyCam {
    vec3 pos;
	vec3 fwd, up, rgt;
	float pitch, yaw;
	float move_speed, turn_speed;
	mat4 V, P;

    void init();
    void init(vec3 cam_pos, vec3 target_pos);
    void update(double dt);
};

void FlyCam::init(){
    pos = vec3(0,1,5);
	fwd = vec3(0,0,-1);
	up = vec3(0,1,0);
	rgt = vec3(1,0,0);
	yaw = 0;
	pitch = 0;
	move_speed = 10;
	turn_speed = 100;
	V = look_at(pos, pos+fwd, up);
	P = perspective(90/gl_aspect_ratio, gl_aspect_ratio, 0.1f, 100.0f);
}

void FlyCam::init(vec3 cam_pos, vec3 target_pos){
    pos = cam_pos;
    V = look_at(cam_pos, target_pos, vec3(0,1,0));
	P = perspective(90/gl_aspect_ratio, gl_aspect_ratio, 0.1f, 100.0f);
	rgt = vec3(V.m[0], V.m[4], V.m[8]);
	up  = vec3(V.m[1], V.m[5], V.m[9]);
    fwd = vec3(-V.m[2], -V.m[6], -V.m[10]);
    yaw = acos(V.m[0])*ONE_RAD_IN_DEG;
    pitch = -acos(V.m[5])*ONE_RAD_IN_DEG; //no idea why you negate this and not yaw, it just works
	move_speed = 10;
	turn_speed = 100;
}

void FlyCam::update(double dt){
    //WASD Movement (constrained to the x-z plane)
    if(g_input[MOVE_FORWARD]) {
        vec3 xz_proj = normalise(vec3(fwd.v[0], 0, fwd.v[2]));
        pos += xz_proj*move_speed*dt;
    }
    if(g_input[MOVE_LEFT]) {
        vec3 xz_proj = normalise(vec3(rgt.v[0], 0, rgt.v[2]));
        pos -= xz_proj*move_speed*dt;
    }
    if(g_input[MOVE_BACK]) {
        vec3 xz_proj = normalise(vec3(fwd.v[0], 0, fwd.v[2]));
        pos -= xz_proj*move_speed*dt;			
    }
    if(g_input[MOVE_RIGHT]) {
        vec3 xz_proj = normalise(vec3(rgt.v[0], 0, rgt.v[2]));
        pos += xz_proj*move_speed*dt;			
    }
    //Increase/decrease elevation
    if(g_input[RAISE_CAM]) {
        pos.v[1] += move_speed*dt;			
    }
    if(g_input[LOWER_CAM]) {
        pos.v[1] -= move_speed*dt;			
    }
    //Rotation
    if(g_input[TURN_CAM_LEFT]) {
        yaw += turn_speed*dt;			
    }
    if(g_input[TURN_CAM_RIGHT]) {
        yaw -= turn_speed*dt;			
    }
    if(g_input[TILT_CAM_UP]) {
        pitch += turn_speed*dt;			
    }
    if(g_input[TILT_CAM_DOWN]) {
        pitch -= turn_speed*dt;			
    }
    //Update matrices
    pitch = MIN(MAX(pitch, -80), 80); //Clamp pitch 
    V = translate(identity_mat4(), pos*(-1));
    mat4 R = rotate_x_deg(rotate_y_deg(identity_mat4(), -yaw), -pitch); //how does this work? 
    //Surely rotating by the yaw will misalign the x-axis for the pitch rotation and screw this up?
    //Seems to work for now! ¯\_(ツ)_/¯
    V = R*V;
    rgt = inverse(R)*vec4(1,0,0,0); //I guess it makes sense that you have to invert
    up = inverse(R)*vec4(0,1,0,0);  //R to calculate these??? I don't know anymore
    fwd = inverse(R)*vec4(0,0,-1,0);
}
