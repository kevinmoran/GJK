#define GL_LITE_IMPLEMENTATION
#include "gl_lite.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

GLFWwindow* window = NULL;
int gl_width = 360;
int gl_height = 240;
float gl_aspect_ratio = (float)gl_width/gl_height;

#include "GameMaths.h"
#include "Input.h"
#include "Camera3D.h"
#include "init_gl.h"
#include "Shader.h"
#include "load_obj.h"
#include "GJK.h"
#include "Player.h"

int main() {
	if (!init_gl(window, "GJK", gl_width, gl_height)){ return 1; }

	//Load cube mesh
	GLuint vao;
	unsigned int num_indices = 0;
	{
		float* vp = NULL;
		uint16_t* indices = NULL;
		unsigned int num_verts = 0;
		load_obj_indexed("cube.obj", &vp, &indices, &num_verts, &num_indices);

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		
		GLuint points_vbo;
		glGenBuffers(1, &points_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
		glBufferData(GL_ARRAY_BUFFER, num_verts*3*sizeof(float), vp, GL_STATIC_DRAW);
		glEnableVertexAttribArray(VP_ATTRIB_LOC);
		glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
		glVertexAttribPointer(VP_ATTRIB_LOC, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		free(vp);

		GLuint index_vbo;
		glGenBuffers(1, &index_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_vbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices*sizeof(unsigned short), indices, GL_STATIC_DRAW);
		free(indices);
	}

	//Set up level geometry
	#define NUM_BOXES 5
	mat4 box_M[NUM_BOXES];
	vec4 box_colour[NUM_BOXES];
	BBox box_collider[NUM_BOXES];
	{
		const vec3 box_pos[NUM_BOXES] = {
			vec3(-6, 0,-6),
			vec3(-6, 0, 6),
			vec3( 0, 0, 0),
			vec3( 6, 0,-6),
			vec3( 6, 0, 6)
		};

		const vec3 box_scale[NUM_BOXES] = {
			vec3(5.0f, 2.0f, 5.0f),
			vec3(5.0f, 1.0f, 5.0f),
			vec3(5.0f, 1.0f, 5.0f),
			vec3(5.0f, 3.0f, 5.0f),
			vec3(5.0f, 1.0f, 5.0f)
		};

		box_M[0] = translate(rotate_y_deg(scale(identity_mat4(),box_scale[0]), 45), box_pos[0]);
		box_M[1] = translate(scale(identity_mat4(),box_scale[1]), box_pos[1]);
		box_M[2] = translate(scale(identity_mat4(),box_scale[2]), box_pos[2]);
		box_M[3] = translate(rotate_x_deg(scale(identity_mat4(),box_scale[3]), 40), box_pos[3]);
		box_M[4] = translate(rotate_z_deg(scale(identity_mat4(),box_scale[4]), 50), box_pos[4]);
	
		//Set up physics objects
		box_collider[0].pos = box_pos[0];
		box_collider[1].pos = box_pos[1];
		box_collider[2].pos = box_pos[2];
		box_collider[3].pos = box_pos[3];
		box_collider[4].pos = box_pos[4];
		for(int i=0; i<NUM_BOXES; i++)
		{
			box_collider[i].min = vec3(-0.5, 0,-0.5);
			box_collider[i].max = vec3( 0.5, 1, 0.5);
			box_collider[i].matRS = box_M[i];
			box_collider[i].matRS_inverse = inverse(box_M[i]);
			box_colour[i] = vec4(0.8f, 0.1f, 0.1f, 1);
		}
	}

	//Set up player's physics collider
	BBox player_collider;
	player_collider.pos = player_pos;
	player_collider.min = vec3(-0.5, 0,-0.5);
	player_collider.max = vec3( 0.5, 1, 0.5);
	player_collider.matRS = identity_mat4();
	player_collider.matRS_inverse = identity_mat4();

	//Camera setup
	g_camera.init(vec3(0,3,6), vec3(0,0,0));

	//Load shader
	Shader box_shader = init_shader("MVP.vert", "uniform_colour.frag");
	GLuint colour_loc = glGetUniformLocation(box_shader.id, "colour");
	glUseProgram(box_shader.id);
	glUniformMatrix4fv(box_shader.V_loc, 1, GL_FALSE, g_camera.V.m);
	glUniformMatrix4fv(box_shader.P_loc, 1, GL_FALSE, g_camera.P.m);

	check_gl_error();

	double curr_time = glfwGetTime(), prev_time, dt;
	//-------------------------------------------------------------------------------------//
	//-------------------------------------MAIN LOOP---------------------------------------//
	//-------------------------------------------------------------------------------------//
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Get dt
		prev_time = curr_time;
		curr_time = glfwGetTime();
		dt = curr_time - prev_time;
		if (dt > 0.1) dt = 0.1;

		// static float fps_timer = 0.0f;
		// fps_timer+=dt;
		// if(fps_timer>0.2f){
		// 	char title_string[64];
		// 	sprintf(title_string, "GJK - %.2fHz", 1/dt);
		// 	glfwSetWindowTitle(window, title_string);
		// 	fps_timer = 0.0f;
		// }

		//Get Input
		g_mouse.prev_xpos = g_mouse.xpos;
    	g_mouse.prev_ypos = g_mouse.ypos;
		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, 1);
		}

		static bool freecam_mode = false;
		static bool F_was_pressed = false;
		if (glfwGetKey(window, GLFW_KEY_F)) {
			if(!F_was_pressed) { freecam_mode = !freecam_mode; }
			F_was_pressed = true;
		}
		else F_was_pressed = false;

		if(freecam_mode) 
			g_camera.update_debug(dt);
		else {
			player_update(dt);
		}

		//do collision detection
		{
			player_collider.pos = player_pos;
			bool hit_something = false;
			for(int i=0; i<NUM_BOXES; i++)
			{
				vec3 mtv(0,0,0); //minimum translation vector
				if(gjk(&player_collider, &box_collider[i], &mtv)){
					hit_something = true;
					float ground_slope = RAD2DEG(acos(dot(normalise(mtv), vec3(0,1,0))));
					if(ground_slope<player_max_stand_slope){
						player_vel.y = 0;
						player_is_on_ground = true;
						player_is_jumping = false;
					}
				}
				player_pos += mtv;

				player_M = translate(identity_mat4(), player_pos);
			}
			static float plat_fall_timer = 0;
			const float plat_fall_time = 0.15;
			if(!hit_something && player_pos.y>0){
				plat_fall_timer += dt;
				if(plat_fall_timer>plat_fall_time || length(player_vel)<player_top_speed/2){
					plat_fall_timer = 0;
					player_is_on_ground = false;
				}
			}
		}

		if(!freecam_mode) g_camera.update_player(player_pos, dt);

		static bool draw_wireframe = true;
		static bool slash_was_pressed = false;
		if (glfwGetKey(window, GLFW_KEY_SLASH)) {
			if(!slash_was_pressed) { draw_wireframe = !draw_wireframe; }
			slash_was_pressed = true;
		}
		else slash_was_pressed = false;

		//Rendering
		glUseProgram(box_shader.id);
		glBindVertexArray(vao);
		glUniformMatrix4fv(box_shader.V_loc, 1, GL_FALSE, g_camera.V.m);
		for(int i=0; i<5; i++){
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDepthFunc(GL_LESS);
			glUniform4fv(colour_loc, 1, box_colour[i].v);
			glUniformMatrix4fv(box_shader.M_loc, 1, GL_FALSE, box_M[i].m);
			glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, 0);

			if(draw_wireframe){
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glDepthFunc(GL_ALWAYS);
				glUniform4fv(colour_loc, 1, vec4(0,0,0,1).v);
				glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, 0);
			}
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDepthFunc(GL_LESS);
		glUniformMatrix4fv(box_shader.M_loc, 1, GL_FALSE, player_M.m);
		glUniform4fv(colour_loc, 1, player_colour.v);
		glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, 0);

		check_gl_error();

		glfwSwapBuffers(window);
	}//end main loop
	return 0;
}
