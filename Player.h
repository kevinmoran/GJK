#pragma once

//Player data
vec3 player_pos = vec3(0,2,0);
vec3 player_scale = vec3(0.5, 0.9, 0.5);
mat4 player_M = translate(scale(identity_mat4(), player_scale), player_pos);
vec3 player_vel = vec3(0,0,0);
vec4 player_colour = vec4(0.1f, 0.8f, 0.3f, 1.0f);
bool player_is_on_ground = false;
bool player_is_jumping = false;
float player_max_stand_slope = 45;
//Physics stuff
//Thanks to Kyle Pittman for his GDC talk:
// http://www.gdcvault.com/play/1023559/Math-for-Game-Programmers-Building
float player_top_speed = 10.0f;
float player_time_till_top_speed = 0.25f; //Human reaction time?
float player_acc = player_top_speed/player_time_till_top_speed;
float friction_factor = 0.3f; //higher is slippier
float player_jump_height = 2.0f;
float player_jump_dist_to_peak = 2.0f; //how far on xz p moves before reaching peak jump height
float g = -2*player_jump_height*player_top_speed*player_top_speed/(player_jump_dist_to_peak*player_jump_dist_to_peak);
float jump_vel = 2*player_jump_height*player_top_speed/player_jump_dist_to_peak;

void player_update(double dt){

    bool player_moved = false;
    //Find player's forward and right movement directions
    vec3 fwd_xz_proj = normalise(vec3(g_camera.fwd.x, 0, g_camera.fwd.z));
    vec3 rgt_xz_proj = normalise(vec3(g_camera.rgt.x, 0, g_camera.rgt.z));

    //WASD Movement (constrained to the x-z plane)
    if(g_input[MOVE_FORWARD]) {
        player_vel += fwd_xz_proj*player_acc*dt;
        player_moved = true;
    }
    else if(dot(fwd_xz_proj,player_vel)>0) player_vel -= fwd_xz_proj*player_acc*dt;

    if(g_input[MOVE_LEFT]) {
        player_vel += -rgt_xz_proj*player_acc*dt;
        player_moved = true;
    }
    else if(dot(-rgt_xz_proj,player_vel)>0) player_vel += rgt_xz_proj*player_acc*dt;

    if(g_input[MOVE_BACK]) {
        player_vel += -fwd_xz_proj*player_acc*dt;
        player_moved = true;			
    }
    else if(dot(-fwd_xz_proj,player_vel)>0) player_vel += fwd_xz_proj*player_acc*dt;

    if(g_input[MOVE_RIGHT]) {
        player_vel += rgt_xz_proj*player_acc*dt;
        player_moved = true;		
    }
    else if(dot(rgt_xz_proj,player_vel)>0) player_vel -= rgt_xz_proj*player_acc*dt;
    // NOTE about the else statements above: Checks if we aren't pressing a button 
    // but have velocity in that direction, if so slows us down faster w/ subtraction
    // This improves responsiveness and tightens up the feel of moving
    // Mult by friction_factor is good to kill speed when idle but feels drifty while moving

    if(player_is_on_ground){
        //Clamp player speed
        if(length2(player_vel) > player_top_speed*player_top_speed) {
            player_vel = normalise(player_vel);
            player_vel *= player_top_speed;
        }
        //Deceleration
        if(!player_moved) player_vel = player_vel*friction_factor;

        static bool jump_button_was_pressed = false;
        if(g_input[JUMP]){
            if(!jump_button_was_pressed){
                player_vel.y += jump_vel;
                player_is_on_ground = false;
                player_is_jumping = true;
                jump_button_was_pressed = true;
            }
        }
        else jump_button_was_pressed = false;
    }
    else { //Player is not on ground
        if(player_is_jumping){
            //If you don't hold jump you don't jump as high
            if(!g_input[JUMP] && player_vel.y>0) player_vel.y += 5*g*dt;
        }

        //Clamp player's xz speed
        vec3 xz_vel = vec3(player_vel.x, 0, player_vel.z);
        if(length(xz_vel) > player_top_speed) {
            xz_vel = normalise(xz_vel);
            xz_vel *= player_top_speed;
            player_vel.x = xz_vel.x;
            player_vel.z = xz_vel.z;
        }
        player_vel.y += g*dt;
    }

    //Update player position
    player_pos += player_vel*dt;

    //Collided into ground
    if(player_pos.y < 0) {
        player_pos.y = 0;
        player_vel.y = 0.0f;
        player_is_on_ground = true;
        player_is_jumping = false;
    }

    if(player_pos.x < -15) player_pos.x = -15;
    if(player_pos.x >  15) player_pos.x =  15;
    if(player_pos.z < -15) player_pos.z = -15;
    if(player_pos.z >  15) player_pos.z =  15;

    //Update matrices
    player_M = translate(scale(identity_mat4(), player_scale), player_pos);
}
