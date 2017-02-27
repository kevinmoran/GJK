#pragma once
#include "GameMaths.h"

//Kevin's simple collider objects for collision detection
//Different shapes which inherit from Collider and implement
//support() function for use in GJK

//Base struct for all collision shapes
struct Collider {
	vec3    pos;            //origin in world space
    mat3    matRS;          //rotation/scale component of model matrix
	mat3    matRS_inverse; 
    virtual vec3 support(vec3 dir) = 0;
};

//BBox: AABB + Orientation matrix
struct BBox : Collider {
    vec3 min, max; //Assume these are axis aligned!

    vec3 support(vec3 dir){
        dir = matRS_inverse*dir; //find support in model space

        vec3 result;
        result.x = (dir.x>0) ? max.x : min.x;
        result.y = (dir.y>0) ? max.y : min.y;
        result.z = (dir.z>0) ? max.z : min.z;

        return matRS*result + pos; //convert support to world space
    }
};

//Polytope: Just a set of points
struct Polytope : Collider {
	float   *points;    //(x0 y0 z0 x1 y1 z1 etc)
	int     num_points;

    //Dumb O(n) support function, just brute force check all points
    vec3 support(vec3 dir){
        dir = matRS_inverse*dir;

        vec3 furthest_point = vec3(points[0], points[1], points[2]);
        float max_dot = dot(furthest_point, dir);

        for(int i=3; i<num_points*3; i+=3){
            vec3 v = vec3(points[i], points[i+1], points[i+2]);
            float d = dot(v, dir);
            if(d>max_dot){
                max_dot = d;
                furthest_point = v;
            }
        }
        vec3 result = matRS*furthest_point + pos; //convert support to world space
        return result;
    }
};