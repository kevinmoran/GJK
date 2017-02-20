#pragma once
// #include <stdio.h>
#include <cstring> //strcat

#define SHADER_PATH "Shaders/"
#define VP_ATTRIB_LOC 0
#define VT_ATTRIB_LOC 1
#define VN_ATTRIB_LOC 2

struct Shader {
    GLuint id;
    GLuint M_loc, V_loc, P_loc;
    bool compiled;
};
Shader init_shader(const char* vert_file, const char* frag_file);
static bool load_shader_program(const char* vert_file, const char* frag_file, GLuint* id);
bool reload_shader_program(const char* vert_file, const char* frag_file, Shader* s);
void delete_program(Shader* s);

Shader init_shader(const char* vert_file, const char* frag_file){
    Shader temp;
    if(!load_shader_program(vert_file, frag_file, &temp.id)){
        //handle failure? default shader program?
        temp.compiled = false;
        return temp;
    }
    temp.compiled = true;
    temp.M_loc = glGetUniformLocation(temp.id, "M");
    temp.V_loc = glGetUniformLocation(temp.id, "V");
    temp.P_loc = glGetUniformLocation(temp.id, "P");
    return temp;
}

static bool load_shader_program(const char* vert_file, const char* frag_file, GLuint* id){

    GLuint vs, fs;
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
            glDetachShader(*id, vs);
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
            glDetachShader(*id, vs);
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
            glDetachShader(*id, vs);
            glDetachShader(*id, fs);
            glDeleteShader(vs);
            glDeleteShader(fs);
            return false;
        }
    }
    
    //Create and link program
    *id = glCreateProgram();
    glAttachShader(*id, vs);
    glAttachShader(*id, fs);

    glBindAttribLocation(*id, VP_ATTRIB_LOC, "vp");
    glBindAttribLocation(*id, VT_ATTRIB_LOC, "vt");
    glBindAttribLocation(*id, VN_ATTRIB_LOC, "vn");

    glLinkProgram(*id);
    
    //Check for linking errors
    {
        GLint params = -1;
        glGetProgramiv(*id, GL_LINK_STATUS, &params);
        if(params != GL_TRUE){
            fprintf(stderr, "ERROR linking shader program with GL index %u\n", *id);
            int log_length = 0;
            char prog_log[2048];
            glGetProgramInfoLog(*id, 2048, &log_length, prog_log);
            fprintf(stderr, "program info log for GL index %u:\n%s\n", *id, prog_log);
            return false;
        }
    }

    glDetachShader(*id, vs);
    glDetachShader(*id, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    
    return true;
}

bool reload_shader_program(const char* vert_file, const char* frag_file, Shader* s){
    delete_program(s);
    if(!load_shader_program(vert_file, frag_file, &(s->id))) {
        fprintf(stderr, "ERROR in reload_shader_program using vert shader %s and frag shader %s", vert_file, frag_file);
        return false;
    }
    s->M_loc = glGetUniformLocation(s->id, "M");
    s->V_loc = glGetUniformLocation(s->id, "V");
    s->P_loc = glGetUniformLocation(s->id, "P");
    return true;
}

void delete_program(Shader* s){
    if(s->compiled) {
        glDeleteProgram(s->id);

        s->id = -1;
        s->M_loc = -1;
        s->V_loc = -1;
        s->P_loc = -1;
        s->compiled = false;
    }
}
