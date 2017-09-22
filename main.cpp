#define GL_LITE_IMPLEMENTATION
#include "gl_lite.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

GLFWwindow* window = NULL;
int gl_width = 1080;
int gl_height = 720;
float gl_aspect_ratio = (float)gl_width/gl_height;
float gl_fullscreen = false;

#include "GameMaths.h"
#include "Input.h"
#include "Camera3D.h"
#include "init_gl.h"
#include "Shader.h"
#include "load_obj.h"
#include "GJK.h"
#include "Player.h"

int main() {
	if(!init_gl(window, "GJK", gl_width, gl_height)){ return 1; }

	//Load cube mesh
	GLuint cube_vao;
	unsigned int cube_num_indices = 0;
	{
		float* vp = NULL;
		uint16_t* indices = NULL;
		unsigned int num_verts = 0;
		load_obj_indexed("cube.obj", &vp, &indices, &num_verts, &cube_num_indices);

		glGenVertexArrays(1, &cube_vao);
		glBindVertexArray(cube_vao);
		
		GLuint points_vbo;
		glGenBuffers(1, &points_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
		glBufferData(GL_ARRAY_BUFFER, num_verts*3*sizeof(float), vp, GL_STATIC_DRAW);
		glEnableVertexAttribArray(VP_ATTRIB_LOC);
		glVertexAttribPointer(VP_ATTRIB_LOC, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		free(vp);

		GLuint index_vbo;
		glGenBuffers(1, &index_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_vbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube_num_indices*sizeof(unsigned short), indices, GL_STATIC_DRAW);
		free(indices);
	}
	//Load sphere mesh
	GLuint sphere_vao;
	unsigned int sphere_num_indices = 0;
	{
		float* vp = NULL;
		uint16_t* indices = NULL;
		unsigned int num_verts = 0;
		load_obj_indexed("sphere.obj", &vp, &indices, &num_verts, &sphere_num_indices);

		glGenVertexArrays(1, &sphere_vao);
		glBindVertexArray(sphere_vao);
		
		GLuint points_vbo;
		glGenBuffers(1, &points_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
		glBufferData(GL_ARRAY_BUFFER, num_verts*3*sizeof(float), vp, GL_STATIC_DRAW);
		glEnableVertexAttribArray(VP_ATTRIB_LOC);
		glVertexAttribPointer(VP_ATTRIB_LOC, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		free(vp);

		GLuint index_vbo;
		glGenBuffers(1, &index_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_vbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere_num_indices*sizeof(unsigned short), indices, GL_STATIC_DRAW);
		free(indices);
	}
	//Load cube mesh
	GLuint cylinder_vao;
	unsigned int cylinder_num_indices = 0;
	{
		float* vp = NULL;
		uint16_t* indices = NULL;
		unsigned int num_verts = 0;
		load_obj_indexed("cylinder.obj", &vp, &indices, &num_verts, &cylinder_num_indices);

		glGenVertexArrays(1, &cylinder_vao);
		glBindVertexArray(cylinder_vao);
		
		GLuint points_vbo;
		glGenBuffers(1, &points_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
		glBufferData(GL_ARRAY_BUFFER, num_verts*3*sizeof(float), vp, GL_STATIC_DRAW);
		glEnableVertexAttribArray(VP_ATTRIB_LOC);
		glVertexAttribPointer(VP_ATTRIB_LOC, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		free(vp);

		GLuint index_vbo;
		glGenBuffers(1, &index_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_vbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylinder_num_indices*sizeof(unsigned short), indices, GL_STATIC_DRAW);
		free(indices);
	}
	//Load player mesh
	GLuint player_vao;
	unsigned int capsule_num_indices = 0;
	{
		float* vp = NULL;
		uint16_t* indices = NULL;
		unsigned int num_verts = 0;
		load_obj_indexed("capsule.obj", &vp, &indices, &num_verts, &capsule_num_indices);

		glGenVertexArrays(1, &player_vao);
		glBindVertexArray(player_vao);
		
		GLuint points_vbo;
		glGenBuffers(1, &points_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
		glBufferData(GL_ARRAY_BUFFER, num_verts*3*sizeof(float), vp, GL_STATIC_DRAW);
		glEnableVertexAttribArray(VP_ATTRIB_LOC);
		glVertexAttribPointer(VP_ATTRIB_LOC, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		free(vp);

		GLuint index_vbo;
		glGenBuffers(1, &index_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_vbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, capsule_num_indices*sizeof(unsigned short), indices, GL_STATIC_DRAW);
		free(indices);
	}

	//Load a triangle
	GLuint triangle_vao;
	int triangle_num_points = 3;
	TriangleCollider triangle_collider;
	{
		vec3 p0 = vec3(-10,0,10);
		vec3 p1 = vec3(-10,0,-10);
		vec3 p2 = vec3(-10,10,0);

		triangle_collider.points[0] = p0;
		triangle_collider.points[1] = p1;
		triangle_collider.points[2] = p2;
		triangle_collider.normal = normalise(cross(p1-p0,p2-p0));

		glGenVertexArrays(1, &triangle_vao);
		glBindVertexArray(triangle_vao);
		
		GLuint points_vbo;
		glGenBuffers(1, &points_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
		glBufferData(GL_ARRAY_BUFFER, 3*3*sizeof(float), triangle_collider.points, GL_STATIC_DRAW);
		glEnableVertexAttribArray(VP_ATTRIB_LOC);
		glVertexAttribPointer(VP_ATTRIB_LOC, 3, GL_FLOAT, GL_FALSE, 0, NULL);
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
		for(int i=0; i<NUM_BOXES; i++)
		{
			box_collider[i].pos = box_pos[i];
			box_collider[i].min = vec3(-0.5, 0,-0.5);
			box_collider[i].max = vec3( 0.5, 1, 0.5);
			box_collider[i].matRS = box_M[i];
			box_collider[i].matRS_inverse = inverse(box_M[i]);
			box_colour[i] = vec4(0.8f, 0.1f, 0.1f, 1);
		}
	}
	#define NUM_SPHERES 3
	mat4 sphere_M[NUM_SPHERES];
	vec4 sphere_colour[NUM_SPHERES];
	Sphere sphere_collider[NUM_SPHERES];
	{
		const vec3 sphere_pos[NUM_SPHERES] = {
			vec3(-6, 3,-6),
			vec3(-6, 2, 6),
			vec3( 6, 5, -6)
		};

		sphere_M[0] = translate(identity_mat4(), sphere_pos[0]);
		sphere_M[1] = translate(identity_mat4(), sphere_pos[1]);
		sphere_M[2] = translate(identity_mat4(), sphere_pos[2]);
	
		//Set up physics objects
		for(int i=0; i<NUM_SPHERES; i++)
		{
			sphere_collider[i].pos = sphere_pos[i];
			sphere_collider[i].r = 1;
			sphere_collider[i].matRS = sphere_M[i];
			sphere_collider[i].matRS_inverse = inverse(sphere_M[i]);
			sphere_colour[i] = vec4(0.1f, 0.8f, 0.1f, 1);
		}
	}
	#define NUM_CYLINDERS 3
	mat4 cylinder_M[NUM_CYLINDERS];
	vec4 cylinder_colour[NUM_CYLINDERS];
	Cylinder cylinder_collider[NUM_CYLINDERS];
	{
		const vec3 cylinder_pos[NUM_CYLINDERS] = {
			vec3( 6, 0, 0),
			vec3(-6, 0, 0),
			vec3( 0, 0,-6)
		};
		const float cylinder_r[NUM_CYLINDERS] = {
			2, 1, 3
		};
		const float cylinder_h[NUM_CYLINDERS] = {
			2, 3, 3
		};

		cylinder_M[0] = translate(scale(identity_mat4(), vec3(cylinder_r[0], cylinder_h[0], cylinder_r[0])), cylinder_pos[0]);
		cylinder_M[1] = translate(scale(identity_mat4(), vec3(cylinder_r[1], cylinder_h[1], cylinder_r[1])), cylinder_pos[1]);
		cylinder_M[2] = translate(scale(identity_mat4(), vec3(cylinder_r[2], cylinder_h[2], cylinder_r[2])), cylinder_pos[2]);
	
		//Set up physics objects
		for(int i=0; i<NUM_CYLINDERS; i++)
		{
			cylinder_collider[i].pos = cylinder_pos[i];
			cylinder_collider[i].r = 1;
			cylinder_collider[i].y_base = cylinder_pos[i].y;
			cylinder_collider[i].y_cap = cylinder_pos[i].y + 1;
			cylinder_collider[i].matRS = cylinder_M[i];
			cylinder_collider[i].matRS_inverse = inverse(cylinder_M[i]);
			cylinder_colour[i] = vec4(0.8f, 0.1f, 0.8f, 1);
		}
	}

	//Set up player's physics collider
	Capsule player_collider;
	player_collider.pos = player_pos;
	player_collider.matRS = player_M;
	player_collider.matRS_inverse = inverse(player_M);
	player_collider.r = 1;
	player_collider.y_base = 1;
	player_collider.y_cap = 2;

	//Camera setup
	g_camera.init(vec3(0,3,6), vec3(0,0,0));

	//Load shader
	Shader basic_shader = init_shader("MVP.vert", "uniform_colour.frag");
	GLuint colour_loc = glGetUniformLocation(basic_shader.id, "colour");
	glUseProgram(basic_shader.id);
	glUniformMatrix4fv(basic_shader.V_loc, 1, GL_FALSE, g_camera.V.m);
	glUniformMatrix4fv(basic_shader.P_loc, 1, GL_FALSE, g_camera.P.m);

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
		if(dt > 0.1) dt = 0.1;

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
		if(glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, 1);
		}

		static bool freecam_mode = false;
		static bool tab_was_pressed = false;
		if(glfwGetKey(window, GLFW_KEY_TAB)) {
			if(!tab_was_pressed) { freecam_mode = !freecam_mode; }
			tab_was_pressed = true;
		}
		else tab_was_pressed = false;

		//Ctrl/Command-F to toggle fullscreen
		//Note: window_resize_callback takes care of resizing viewport/recalculating P matrix
		static bool F_was_pressed = false;
		if(glfwGetKey(window, GLFW_KEY_F)) {
			if(!F_was_pressed){
				if(glfwGetKey(window, CTRL_KEY_LEFT) || glfwGetKey(window, CTRL_KEY_RIGHT)){
					gl_fullscreen = !gl_fullscreen;
					static int old_win_x, old_win_y, old_win_w, old_win_h;
					if(gl_fullscreen){
						glfwGetWindowPos(window, &old_win_x, &old_win_y);
						glfwGetWindowSize(window, &old_win_w, &old_win_h);
						GLFWmonitor* mon = glfwGetPrimaryMonitor();
						const GLFWvidmode* vidMode = glfwGetVideoMode(mon);
						glfwSetWindowMonitor(window, mon, 0, 0, vidMode->width, vidMode->height, vidMode->refreshRate);
					}
					else glfwSetWindowMonitor(window, NULL, old_win_x, old_win_y, old_win_w, old_win_h, GLFW_DONT_CARE);
				}
			}
			F_was_pressed = true;
		}
		else F_was_pressed = false;

		//Move player
		if(!freecam_mode) player_update(dt);

		//do collision detection
		{
			player_collider.pos = player_pos;
			bool hit_ground = false;
			//BOXES
			for(int i=0; i<NUM_BOXES; i++)
			{
				vec3 mtv(0,0,0); //minimum translation vector
				if(gjk(&player_collider, &box_collider[i], &mtv)){
					float ground_slope = RAD2DEG(acos(dot(normalise(mtv), vec3(0,1,0))));
					if(ground_slope<player_max_stand_slope){
						hit_ground = true;
						player_vel.y = 0;
						player_is_on_ground = true;
						player_is_jumping = false;
					}
				}
				player_collider.pos += mtv;
			}
			//SPHERES
			for(int i=0; i<NUM_SPHERES; i++)
			{
				if(gjk(&player_collider, &sphere_collider[i])){
					sphere_colour[i] = vec4(0.8f,0.8f,0.1f,1);
				}
				else sphere_colour[i] = vec4(0.1f,0.8f,0.1f,1);
			}
			//CYLINDERS
			for(int i=0; i<NUM_CYLINDERS; i++)
			{
				vec3 mtv(0,0,0); //minimum translation vector
				if(gjk(&player_collider, &cylinder_collider[i], &mtv)){
					float ground_slope = RAD2DEG(acos(dot(normalise(mtv), vec3(0,1,0))));
					if(ground_slope<player_max_stand_slope){
						hit_ground = true;
						player_vel.y = 0;
						player_is_on_ground = true;
						player_is_jumping = false;
					}
				}
				player_collider.pos += mtv;
			}
			//Triangle
			if(gjk(&player_collider, &triangle_collider)){
				vec3 support = player_collider.support(-triangle_collider.normal);
				float penetration_depth = dot(support-triangle_collider.points[0], -triangle_collider.normal);
				vec3 mtv = triangle_collider.normal*penetration_depth;

				player_collider.pos += mtv;
			}

			//Grace Period for jumping when running off platforms
			{
				static float grace_timer = 0;
				const float grace_time = 0.25;
				if(!hit_ground && player_pos.y>0)
				{
					grace_timer += dt;
					if(grace_timer>grace_time*1.5*length(player_vel)/player_top_speed)
					{
						grace_timer = 0;
						player_is_on_ground = false;
					}
				}
			}

			player_pos = player_collider.pos;
			player_M = translate(scale(identity_mat4(), player_scale), player_pos);
		}

		if(freecam_mode) g_camera.update_debug(dt);
		else g_camera.update_player(player_pos, dt);

		static bool draw_wireframe = true;
		static bool slash_was_pressed = false;
		if(glfwGetKey(window, GLFW_KEY_SLASH)) {
			if(!slash_was_pressed) { draw_wireframe = !draw_wireframe; }
			slash_was_pressed = true;
		}
		else slash_was_pressed = false;

		//Rendering
		glUseProgram(basic_shader.id);
		glUniformMatrix4fv(basic_shader.V_loc, 1, GL_FALSE, g_camera.V.m);

		//Cubes
		glBindVertexArray(cube_vao);
		for(int i=0; i<NUM_BOXES; i++){
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDepthFunc(GL_LESS);
			glUniform4fv(colour_loc, 1, box_colour[i].v);
			glUniformMatrix4fv(basic_shader.M_loc, 1, GL_FALSE, box_M[i].m);
			glDrawElements(GL_TRIANGLES, cube_num_indices, GL_UNSIGNED_SHORT, 0);

			if(draw_wireframe){
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glDepthFunc(GL_ALWAYS);
				glUniform4fv(colour_loc, 1, vec4(0,0,0,1).v);
				glDrawElements(GL_TRIANGLES, cube_num_indices, GL_UNSIGNED_SHORT, 0);
			}
		}
		//Spheres
		glBindVertexArray(sphere_vao);
		for(int i=0; i<NUM_SPHERES; i++){
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDepthFunc(GL_LESS);
			glUniform4fv(colour_loc, 1, sphere_colour[i].v);
			glUniformMatrix4fv(basic_shader.M_loc, 1, GL_FALSE, sphere_M[i].m);
			glDrawElements(GL_TRIANGLES, sphere_num_indices, GL_UNSIGNED_SHORT, 0);
		}
		//Cylinders
		glBindVertexArray(cylinder_vao);
		for(int i=0; i<NUM_CYLINDERS; i++){
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDepthFunc(GL_LESS);
			glUniform4fv(colour_loc, 1, cylinder_colour[i].v);
			glUniformMatrix4fv(basic_shader.M_loc, 1, GL_FALSE, cylinder_M[i].m);
			glDrawElements(GL_TRIANGLES, cylinder_num_indices, GL_UNSIGNED_SHORT, 0);

			if(draw_wireframe){
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glDepthFunc(GL_ALWAYS);
				glUniform4fv(colour_loc, 1, vec4(0,0,0,1).v);
				glDrawElements(GL_TRIANGLES, cylinder_num_indices, GL_UNSIGNED_SHORT, 0);
			}
		}
		//Player
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDepthFunc(GL_LESS);
		glUniformMatrix4fv(basic_shader.M_loc, 1, GL_FALSE, player_M.m);
		glUniform4fv(colour_loc, 1, player_colour.v);
		glBindVertexArray(player_vao);
		glDrawElements(GL_TRIANGLES, capsule_num_indices, GL_UNSIGNED_SHORT, 0);

		//Triangle
		glUniformMatrix4fv(basic_shader.M_loc, 1, GL_FALSE, identity_mat4().m);
		glUniform4fv(colour_loc, 1, vec4(0.6,0,0.8,1).v);
		glBindVertexArray(triangle_vao);
		glDrawArrays(GL_TRIANGLES, 0, triangle_num_points);

		check_gl_error();

		glfwSwapBuffers(window);
	}//end main loop
	return 0;
}
