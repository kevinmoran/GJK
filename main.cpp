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

int main() {
	if (!init_gl(window, "GJK", gl_width, gl_height)){ return 1; }
	float* vp = NULL;
	unsigned int point_count = 0;
	load_obj("cube.obj", &vp, &point_count);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	GLuint points_vbo;
	glGenBuffers(1, &points_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glBufferData(GL_ARRAY_BUFFER, point_count*3*sizeof(float), vp, GL_STATIC_DRAW);
	glEnableVertexAttribArray(VP_ATTRIB_LOC);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glVertexAttribPointer(VP_ATTRIB_LOC, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	free(vp);

	//Collision mesh for GJK
	// float box_points[] = {
	// 	-0.5f, -0.5f, -0.5f,
	// 	0.5f, -0.5f, -0.5f,
	// 	-0.5f, -0.5f, 0.5f,
	// 	0.5f, -0.5f, 0.5f,
	// 	-0.5f, 0.5f, -0.5f,
	// 	0.5f, 0.5f, -0.5f,
	// 	-0.5f, 0.5f, 0.5f,
	// 	0.5f, 0.5f, 0.5f
	// };

	mat4 box_M[5];
	box_M[0] = translate(rotate_y_deg(identity_mat4(), 45), vec3(-1.5f, 0, -1.5f));
	box_M[1] = translate(identity_mat4(), vec3(-1.5f, 0, 1.5f));
	box_M[2] = translate(identity_mat4(), vec3(0, 0, 0));
	box_M[3] = translate(identity_mat4(), vec3(1.5f, 0, -1.5f));
	box_M[4] = translate(identity_mat4(), vec3(1.5f, 0, 1.5f));
	vec4 box_colour[5];

	BBox box_collider[5];
	box_collider[0].pos = vec3(-1.5f, 0, -1.5f);
	box_collider[1].pos = vec3(-1.5f, 0, 1.5f);
	box_collider[2].pos = vec3(0, 0, 0);
	box_collider[3].pos = vec3(1.5f, 0, -1.5f);
	box_collider[4].pos = vec3(1.5f, 0, 1.5f);
	for(int i=0; i<5; i++){
		// box_collider[i].points = box_points;
		// box_collider[i].num_points = 8;
		box_collider[i].min = vec3(-0.5,-0.5,-0.5);
		box_collider[i].max = vec3( 0.5, 0.5, 0.5);
		box_collider[i].matRS = box_M[i];
		box_collider[i].matRS_inverse = inverse(box_M[i]);
		box_colour[i] = vec4(0.8f, 0.1f, 0.1f, 1);
	}

	vec3 player_pos = vec3(0,0,2.5f); //vec3(-1.0f,0,-1.5f); //this is a broken case, fix!
	mat4 player_M = translate(identity_mat4(), player_pos);
	vec4 player_colour = vec4(0.1f, 0.8f, 0.3f, 1.0f);
	float player_speed = 10;

	BBox player_collider;
	player_collider.pos = player_pos;
	//player_collider.points = box_points;
	//player_collider.num_points = 8;
	player_collider.min = vec3(-0.5,-0.5,-0.5);
	player_collider.max = vec3( 0.5, 0.5, 0.5);
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
		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, 1);
		}

		static bool camera_enabled = false;
		static bool F_was_pressed = false;
		if (glfwGetKey(window, GLFW_KEY_F)) {
			if(!F_was_pressed) { camera_enabled = !camera_enabled; }
			F_was_pressed = true;
		}
		else F_was_pressed = false;

		if(camera_enabled) g_camera.update(dt);
		else{
			//Move player
			if(g_input[MOVE_FORWARD]) {
				vec3 xz_proj = normalise(vec3(g_camera.fwd.v[0], 0, g_camera.fwd.v[2]));
				player_pos += xz_proj*player_speed*dt;
			}
			if(g_input[MOVE_LEFT]) {
				vec3 xz_proj = normalise(vec3(g_camera.rgt.v[0], 0, g_camera.rgt.v[2]));
				player_pos -= xz_proj*player_speed*dt;
			}
			if(g_input[MOVE_BACK]) {
				vec3 xz_proj = normalise(vec3(g_camera.fwd.v[0], 0, g_camera.fwd.v[2]));
				player_pos -= xz_proj*player_speed*dt;			
			}
			if(g_input[MOVE_RIGHT]) {
				vec3 xz_proj = normalise(vec3(g_camera.rgt.v[0], 0, g_camera.rgt.v[2]));
				player_pos += xz_proj*player_speed*dt;			
			}
			player_M = translate(identity_mat4(), player_pos);
		}

		//if (glfwGetKey(window, GLFW_KEY_G)) {
			player_collider.pos = player_pos;
			for(int i=0; i<5; i++){
				vec3 mtv(0,0,0); //minimum translation vector
				Collider* p = &player_collider;
				Collider* b = &box_collider[i];
				if(gjk(p, b, &mtv)) 
					box_colour[i] = vec4(0.8f, 0.7f, 0.0f, 1);
				else 
					box_colour[i] = vec4(0.8f, 0.1f, 0.1f, 1);
				
				player_pos += mtv;
				player_M = translate(identity_mat4(), player_pos);
			}
		//}

		//Rendering
		glUseProgram(box_shader.id);
		glBindVertexArray(vao);
		glUniformMatrix4fv(box_shader.V_loc, 1, GL_FALSE, g_camera.V.m);
		for(int i=0; i<5; i++){
			glUniform4fv(colour_loc, 1, box_colour[i].v);
			glUniformMatrix4fv(box_shader.M_loc, 1, GL_FALSE, box_M[i].m);
			glDrawArrays(GL_TRIANGLES, 0, point_count);
		}

		glUniformMatrix4fv(box_shader.M_loc, 1, GL_FALSE, player_M.m);
		glUniform4fv(colour_loc, 1, player_colour.v);
		glDrawArrays(GL_TRIANGLES, 0, point_count);

		check_gl_error();

		glfwSwapBuffers(window);
	}//end main loop
	return 0;
}
