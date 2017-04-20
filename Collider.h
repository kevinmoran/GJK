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

//Sphere: NB Does not use RS matrix, scale the radius directly!
struct Sphere : Collider {
    float r;

    vec3 support(vec3 dir){
        return normalise(dir)*r + pos;
    }
};

//Cylinder: Height-aligned with y-axis (rotate using matRS)
struct Cylinder : Collider {
    float r, y_base, y_cap;

    vec3 support(vec3 dir){
        dir = matRS_inverse*dir; //find support in model space

        vec3 dir_xz = vec3(dir.x, 0, dir.z);
        vec3 result = normalise(dir_xz)*r;
        result.y = (dir.y>0) ? y_cap : y_base;

        return matRS*result + pos; //convert support to world space
    }
};

//Capsule: Height-aligned with y-axis
struct Capsule : Collider {
    float r, y_base, y_cap;

    vec3 support(vec3 dir){
        dir = matRS_inverse*dir; //find support in model space

        vec3 result = normalise(dir)*r;
        result.y += (dir.y>0) ? y_cap : y_base;

        return matRS*result + pos; //convert support to world space
    }
};

//Triangle: Kind of a hack 
// "All physics code is an awful hack" - Will, #HandmadeDev
//Need to fake a prism for GJK to converge
//NB: Currently using world-space points, ignore matRS and pos from base class
//Don't use EPA with this! Might resolve collision along any one of prism's faces
//Only resolve around triangle normal
struct TriangleCollider : Collider {
    vec3 points[3];
    vec3 normal;

    vec3 support(vec3 dir){
        //Find which triangle vertex is furthest along dir
        float dot0 = dot(points[0], dir);
        float dot1 = dot(points[1], dir);
        float dot2 = dot(points[2], dir);
        vec3 furthest_point = points[0];
        if(dot1>dot0){
            furthest_point = points[1];
            if(dot2>dot1) 
                furthest_point = points[2];
        }
        else if(dot2>dot0) {
            furthest_point = points[2];
        }

        //fake some depth behind triangle so we have volume
        if(dot(dir, normal)<0) furthest_point-= normal; 

        return furthest_point;
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