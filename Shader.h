#pragma once
#include <GL/glew.h>
#include <stdio.h>
#include <cstring> //strcat

#define SHADER_PATH "Shaders/"

class Shader {
public:
    GLuint prog_id, vs, fs;
    GLuint M_loc, V_loc, P_loc;
    bool compiled;
    
    Shader(const char* vert_file, const char* frag_file) {
        prog_id = -1;
        vs = -1;
        fs = -1;
        M_loc = -1;
        V_loc = -1;
        P_loc = -1;
        compiled = false;
        
        if(!load_shader_program(vert_file, frag_file)){
            //handle failure? default shader program?
        }
    }
    
    ~Shader(){
		delete_program();    
    }
    
    bool load_shader_program(const char* vert_file, const char* frag_file) {
        //Check inputs are valid?
        
        if(compiled){ //Need to delete existing program first!
            fprintf(stderr, "ERROR in load_shader_program; program %u is already compiled", prog_id);
            return false;
        }
        
        //Load vertex shader
            char vs_string[65536];
        {
            vs_string[0] = '\0';
            char vert_file_path[64];
            sprintf(vert_file_path, "%s%s", SHADER_PATH, vert_file);
            
            FILE* fp = fopen(vert_file_path, "r");
            if(!fp){
                fprintf(stderr, "ERROR opening vertex shader file: %s\n", vert_file_path);
                return false;
            }
            char line[1024];
            while(fgets(line, 1024, fp)){
                strcat(vs_string, line);
            }
            fclose(fp);
        }
        //Compile vertex shader
        {
            vs = glCreateShader(GL_VERTEX_SHADER);
            const char* vs_pointer = vs_string;
            glShaderSource(vs, 1, &vs_pointer, NULL);
            glCompileShader(vs);
            
            //Check for compilation errors
            GLint params = -1;
            glGetShaderiv(vs, GL_COMPILE_STATUS, &params);
            if(params!=GL_TRUE){
                fprintf(stderr,"ERROR: vertex shader `%s` did not compile\n", vert_file);
                int log_length = 0;
                char shader_log[2048];
                glGetShaderInfoLog(vs, 2048, &log_length, shader_log);
                fprintf(stderr, "shader info log for GL index %u: \n%s\n", vs, shader_log);
				glDetachShader(prog_id, vs);
				glDeleteShader(vs);
                return false;
            }
        }
        
        //Load fragment shader
            char fs_string[65536];
        {
            fs_string[0] = '\0';
            char frag_file_path[64];
            sprintf(frag_file_path, "%s%s", SHADER_PATH, frag_file);
            
            FILE* fp = fopen(frag_file_path, "r");
            if(!fp){
                fprintf(stderr, "ERROR opening fragment shader file: %s\n", frag_file_path);
				glDetachShader(prog_id, vs);
				glDeleteShader(vs);
                return false;
            }
            char line[1024];
            while(fgets(line, 1024, fp)){
                strcat(fs_string, line);
            }
            fclose(fp);
        }
        //Compile fragment shader
        {
            fs = glCreateShader(GL_FRAGMENT_SHADER);
            const char* fs_pointer = fs_string;
            glShaderSource(fs, 1, &fs_pointer, NULL);
            glCompileShader(fs);
            
            //Check for compilation errors
            GLint params = -1;
            glGetShaderiv(fs, GL_COMPILE_STATUS, &params);
            if(params!=GL_TRUE){
                fprintf(stderr,"ERROR: fragment shader `%s` did not compile\n", frag_file);
                int log_length = 0;
                char shader_log[2048];
                glGetShaderInfoLog(fs, 2048, &log_length, shader_log);
                fprintf(stderr, "shader info log for GL index %u: \n%s\n", fs, shader_log);
				glDetachShader(prog_id, vs);
				glDetachShader(prog_id, fs);
				glDeleteShader(vs);
				glDeleteShader(fs);
                return false;
            }
        }
        
        //Create and link program
        prog_id = glCreateProgram();
        glAttachShader(prog_id, vs);
        glAttachShader(prog_id, fs);
        glLinkProgram(prog_id);
        
        //Check for linking errors
        {
            GLint params = -1;
            glGetProgramiv(prog_id, GL_LINK_STATUS, &params);
            if(params != GL_TRUE){
                fprintf(stderr, "ERROR linking shader program with GL index %u\n", prog_id);
                int log_length = 0;
                char prog_log[2048];
                glGetProgramInfoLog(prog_id, 2048, &log_length, prog_log);
                fprintf(stderr, "program info log for GL index %u:\n%s\n", prog_id, prog_log);
                return false;
            }
        }
        
        M_loc = glGetUniformLocation(prog_id, "M");
        V_loc = glGetUniformLocation(prog_id, "V");
        P_loc = glGetUniformLocation(prog_id, "P");

		glDetachShader(prog_id, vs);
		glDetachShader(prog_id, fs);
		glDeleteShader(vs);
		glDeleteShader(fs);
        
        compiled = true;
        return true;
    }
    
	bool reload_shader_program(const char* vert_file, const char* frag_file) {
		delete_program();
		if (!load_shader_program(vert_file, frag_file)) {
			fprintf(stderr, "ERROR in reload_shader_program using vert shader %s and frag shader %s", vert_file, frag_file);
			return false;
		}
		return true;
	}

    void delete_program(){
		if (compiled) {
			glDeleteProgram(prog_id);

			prog_id = -1;
			vs = -1;
			fs = -1;
			M_loc = -1;
			V_loc = -1;
			P_loc = -1;
			compiled = false;
		}
    }
};
