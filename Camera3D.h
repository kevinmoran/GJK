#pragma once
// #include "GameMaths.h"
// #include "Input.h"

bool cam_mouse_controls = false;

struct Camera3D {
    vec3 pos;
	vec3 fwd, up, rgt;
	float pitch, yaw;
	float move_speed, turn_speed;
	mat4 V, P;

    void init();
    void init(vec3 cam_pos);
    void init(vec3 cam_pos, vec3 target_pos);
    void update_debug(double dt);
    void update_player(vec3 player_pos, double dt);
};
Camera3D g_camera;

void Camera3D::init(){
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

    if(cam_mouse_controls) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
}

void Camera3D::init(vec3 cam_pos){
    pos = cam_pos;
    V = look_at(cam_pos, cam_pos+vec3(0,0,-1), vec3(0,1,0));
	P = perspective(90/gl_aspect_ratio, gl_aspect_ratio, 0.1f, 100.0f);
	rgt = vec3(V.m[0], V.m[4], V.m[8]);
	up  = vec3(V.m[1], V.m[5], V.m[9]);
    fwd = vec3(-V.m[2], -V.m[6], -V.m[10]);
    yaw   =  acos(V.m[0])*ONE_RAD_IN_DEG;
    pitch = -acos(V.m[5])*ONE_RAD_IN_DEG; //no idea why you negate this and not yaw, it just works
	move_speed = 10;
	turn_speed = 100;

    if(cam_mouse_controls) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
}

void Camera3D::init(vec3 cam_pos, vec3 target_pos){
    pos = cam_pos;
    V = look_at(cam_pos, target_pos, vec3(0,1,0));
	P = perspective(90/gl_aspect_ratio, gl_aspect_ratio, 0.1f, 100.0f);
	rgt = vec3(V.m[0], V.m[4], V.m[8]);
	up  = vec3(V.m[1], V.m[5], V.m[9]);
    fwd = vec3(-V.m[2], -V.m[6], -V.m[10]);
    yaw   =  acos(V.m[0])*ONE_RAD_IN_DEG;
    pitch = -acos(V.m[5])*ONE_RAD_IN_DEG; //no idea why you negate this and not yaw, it just works
	move_speed = 10;
	turn_speed = 100;

    if(cam_mouse_controls) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
}

void Camera3D::update_debug(double dt){
    //WASD Movement (constrained to the x-z plane)
    if(g_input[MOVE_FORWARD]) {
        vec3 xz_proj = normalise(vec3(fwd.x, 0, fwd.z));
        pos += xz_proj*move_speed*dt;
    }
    if(g_input[MOVE_LEFT]) {
        vec3 xz_proj = normalise(vec3(rgt.x, 0, rgt.z));
        pos -= xz_proj*move_speed*dt;
    }
    if(g_input[MOVE_BACK]) {
        vec3 xz_proj = normalise(vec3(fwd.x, 0, fwd.z));
        pos -= xz_proj*move_speed*dt;			
    }
    if(g_input[MOVE_RIGHT]) {
        vec3 xz_proj = normalise(vec3(rgt.x, 0, rgt.z));
        pos += xz_proj*move_speed*dt;			
    }
    //Increase/decrease elevation
    if(g_input[RAISE_CAM]) {
        pos.y += move_speed*dt;			
    }
    if(g_input[LOWER_CAM]) {
        pos.y -= move_speed*dt;			
    }
    //Rotation
    if(!cam_mouse_controls){
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
    }
    else {
        yaw   += (g_mouse.prev_xpos-g_mouse.xpos) * g_mouse.sensitivity * turn_speed*dt;
        pitch += (g_mouse.prev_ypos-g_mouse.ypos) * g_mouse.sensitivity * turn_speed*dt;
    }
    //Update matrices
    pitch = MIN(MAX(pitch, -89), 80); //Clamp pitch 
    mat4 R = rotate_x_deg(rotate_y_deg(identity_mat4(), -yaw), -pitch); //how does this work? 
    //Surely rotating by the yaw will misalign the x-axis for the pitch rotation and screw this up?
    //Seems to work for now! ¯\_(ツ)_/¯
    rgt = inverse(R)*vec4(1,0,0,0); //I guess it makes sense that you have to invert
    up  = inverse(R)*vec4(0,1,0,0);  //R to calculate these??? I don't know anymore
    fwd = inverse(R)*vec4(0,0,-1,0);

    V = translate(identity_mat4(), pos*(-1));
    V = R*V;
}

void Camera3D::update_player(vec3 player_pos, double dt){
    //Rotation
    if(!cam_mouse_controls){
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
    }
    else {
        yaw   += (g_mouse.prev_xpos-g_mouse.xpos) * g_mouse.sensitivity * turn_speed*dt;
        pitch += (g_mouse.prev_ypos-g_mouse.ypos) * g_mouse.sensitivity * turn_speed*dt;
    }
    //Update matrices
    pitch = MIN(MAX(pitch, -89), 80); //Clamp pitch 
    mat4 R = rotate_x_deg(rotate_y_deg(identity_mat4(), -yaw), -pitch); //how does this work? 
    //Surely rotating by the yaw will misalign the x-axis for the pitch rotation and screw this up?
    //Seems to work for now! ¯\_(ツ)_/¯
    rgt = inverse(R)*vec4(1,0,0,0); //I guess it makes sense that you have to invert
    up  = inverse(R)*vec4(0,1,0,0);  //R to calculate these??? I don't know anymore
    fwd = inverse(R)*vec4(0,0,-1,0);

    pos = player_pos-fwd*5 + up*2;

    V = translate(identity_mat4(), pos*(-1));
    V = R*V;
}

void window_resize_callback(GLFWwindow* window, int width, int height){
    gl_width = width;
    gl_height = height;
    gl_aspect_ratio = (float)gl_width/gl_height;
    g_camera.P = perspective(90/gl_aspect_ratio, gl_aspect_ratio, 0.1f, 100.0f);
    int fb_w, fb_h;
	glfwGetFramebufferSize(window,&fb_w,&fb_h);
	glViewport(0,0,fb_w,fb_h);
}
