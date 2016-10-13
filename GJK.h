#pragma once
#include <assert.h>
#include "maths_funcs.h"

//Kevin's implementation of the Gilbert-Johnson-Keerthi intersection algorithm
//Most useful references (Huge thanks to all the authors):

// "Implementing GJK" by Casey Muratori:
// The best description of the algorithm from the ground up
// https://www.youtube.com/watch?v=Qupqu1xe7Io

// "Implementing a GJK Intersection Query" by Phill Djonov
// Interesting tips for implementing the algorithm
// http://vec3.ca/gjk/implementation/

// "GJK Algorithm 3D" by Sergiu Craitoiu
// Has nice diagrams to visualise the tetrahedral case
// http://in2gpu.com/2014/05/18/gjk-algorithm-3d/

// "GJK + Expanding Polytope Algorithm - Implementation and Visualization"
// Good breakdown of EPA with demo for visualisation
// https://www.youtube.com/watch?v=6rgiPrzqt9w

struct Collider;

vec3 support(Collider shape, vec3 dir);
bool gjk(Collider coll1, Collider coll2, vec3 &mtv);
void update_simplex3(vec3 &a, vec3 &b, vec3 &c, vec3 &d, int &i, vec3 &search_dir);
bool update_simplex4(vec3 &a, vec3 &b, vec3 &c, vec3 &d, int &i, vec3 &search_dir);
vec3 EPA(vec3 a, vec3 b, vec3 c, vec3 d, Collider coll1, Collider coll2);

struct Collider{
	vec3 pos; //origin in world space
	float *points; //array of verts (x0 y0 z0 x1 y1 z1 etc)
	int num_points; //num vertices 
    mat3 M;         //rotation/scale component of model matrix
	mat3 M_inverse; 
};

vec3 support(Collider shape, vec3 dir){
    dir = shape.M_inverse*dir; //transform dir by shape's inverse rotation matrix
    float max_dot = -99;
    int support_index = 0;
    for(int i=0; i<shape.num_points*3; i+=3){
        vec3 v(shape.points[i], shape.points[i+1], shape.points[i+2]);
        float d = dot(v, dir);
        if(d>max_dot){
            max_dot = d;
            support_index = i;
        }
    }
    return (shape.M*vec3(shape.points[support_index], 
                shape.points[support_index+1], 
                shape.points[support_index+2])
            + shape.pos);
}

bool gjk(Collider coll1, Collider coll2, vec3 &mtv){
    vec3 a, b, c, d; //Simplex: just a set of points (a is always most recently added)
    vec3 search_dir = coll1.pos - coll2.pos; //initial search direction between colliders

    //Get initial point for simplex
    c = support(coll1, search_dir) - support(coll2, -search_dir);
    search_dir = -c; //search in direction of origin

    //Get second point for a line segment simplex
    b = support(coll1, search_dir) - support(coll2, -search_dir);
    if(dot(b, search_dir)<0) {
        //printf("GJK No collision (search didn't reach origin). Exited before loop\n");
        return false; //we didn't reach the origin, won't enclose it
    }
    search_dir = cross(cross(c-b,-b),c-b); //search normal to line segment towards origin
    int i = 2; //simplex dimension
    
    for(int iterations=0; iterations<64; iterations++){
        a = support(coll1, search_dir) - support(coll2, -search_dir);
        if(dot(a, search_dir)<0) {
            //printf("GJK No collision (search didn't reach origin). Iterations: %d\n", iterations);
            return false; //we didn't reach the origin, won't enclose it
        }
        i++;

        if(i==3){
            update_simplex3(a,b,c,d,i,search_dir);
        }
        else if(update_simplex4(a,b,c,d,i,search_dir)) {
                //printf("GJK Collision. Iterations: %d\n", iterations);
                mtv = EPA(a,b,c,d,coll1,coll2);
                printf("Minimum translation vector:\n");
                print(mtv);
                return true;
        }
    }//endfor
    //printf("GJK No collision. Ran out of iterations\n");
    return false;
}

//Triangle case
void update_simplex3(vec3 &a, vec3 &b, vec3 &c, vec3 &d, int &i, vec3 &search_dir){
    /* Required winding order:
    //  b
    //  | \
    //  |   \
    //  |    a
    //  |   /
    //  | /
    //  c
    */
    vec3 n = cross(b-a, c-a); //triangle's normal
    vec3 AO = -a; //direction to origin

    //Determine which facet is closest to origin, make that new simplex

    i = 2; //hoisting this just cause
    //Closest to edge AB
    if(dot(cross(b-a, n), AO)>0){
        c = a;
        //i = 2;
        search_dir = cross(cross(b-a, AO), b-a);
        return;
    }
    //Closest to edge AC
    if(dot(cross(n, c-a), AO)>0){
        b = a;
        //i = 2;
        search_dir = cross(cross(c-a, AO), c-a);
        return;
    }
    
    i = 3; //hoisting this just cause
    //Above triangle
    if(dot(n, AO)>0){
        d = c;
        c = b;
        b = a;
        //i = 3;
        search_dir = n;
        return;
    }
    //else
    //Below triangle
    d = b;
    b = a;
    //i = 3;
    search_dir = -n;
    return;
}

//Tetrahedral case
bool update_simplex4(vec3 &a, vec3 &b, vec3 &c, vec3 &d, int &i, vec3 &search_dir){
    // a is peak/tip of pyramid, BCD is the base (counterclockwise winding order)
	//We know a priori that origin is above BCD and below a

    //Get normals of three new faces
    vec3 ABC = cross(b-a, c-a);
    vec3 ACD = cross(c-a, d-a);
    vec3 ADB = cross(d-a, b-a);

    vec3 AO = -a; //dir to origin
    i = 3; //hoisting this just cause

    //Plane-test origin with 3 faces
    //TODO: Not sure what to do with search direction here, or
    //whether to call update_simplex3...
    //Makes no difference for AABBS, test with more complex colliders!!!
    if(dot(ABC, AO)>0){
    	//In front of ABC
    	d = c;
    	c = b;
    	b = a;
        //search_dir = ABC;
        //update_simplex3(a,b,c,d,i,search_dir);
    	return false;
    }
    if(dot(ACD, AO)>0){
    	//In front of ACD
    	b = a;
        //search_dir = ABC;
        //update_simplex3(a,b,c,d,i,search_dir);
    	return false;
    }
    if(dot(ADB, AO)>0){
    	//In front of ADB
    	c = d;
    	d = b;
    	b = a;
        //search_dir = ABC;
        //update_simplex3(a,b,c,d,i,search_dir);
    	return false;
    }

    //else inside tetrahedron; enclosed!
    return true;

    //Note: in the case where two of the faces have similar normals,
    //The origin could conceivably be closest to an edge on the tetrahedron
    //Right now I don't think it'll make a difference to limit our new simplices
    //to just one of the faces, maybe test it later.
}

//Expanding Polytope Algorithm
//Find minimum translation vector to resolve collision
#define EPA_TOLERANCE 0.0001
#define EPA_MAX_NUM_FACES 128
#define EPA_MAX_NUM_LOOSE_EDGES 256
vec3 EPA(vec3 a, vec3 b, vec3 c, vec3 d, Collider coll1, Collider coll2){
    vec3 faces[EPA_MAX_NUM_FACES][4]; //Array of faces, each with 3 verts and a normal
    
    //Init with final simplex from GJK
    faces[0][0] = a;
    faces[0][1] = b;
    faces[0][2] = c;
    faces[0][3] = normalise(cross(b-a, c-a)); //ABC
    faces[1][0] = a;
    faces[1][1] = c;
    faces[1][2] = d;
    faces[1][3] = normalise(cross(c-a, d-a)); //ACD
    faces[2][0] = a;
    faces[2][1] = d;
    faces[2][2] = b;
    faces[2][3] = normalise(cross(d-a, b-a)); //ADB
    faces[3][0] = b;
    faces[3][1] = c;
    faces[3][2] = d;
    faces[3][3] = normalise(cross(c-b, d-b)); //BCD

    int num_faces=4;
    vec3 p; //new point used to expand polytope
    
    while(num_faces<EPA_MAX_NUM_FACES){
        printf("EPA loop\n");
        printf("num face: %d\n", num_faces);
        //Find face that's closest to origin
        float min_dist = dot(faces[0][0], faces[0][3]);
        int closest_face = 0;
        for(int i=1; i<num_faces; i++){
            float dist = dot(faces[i][0], faces[i][3]);
            if(dist<min_dist){
                min_dist = dist;
                closest_face = i;
            }
        }
        //search normal to face that's closest to origin
        vec3 search_dir = faces[closest_face][3]; 
        p = support(coll1, search_dir) - support(coll2, -search_dir);
        if(dot(p, search_dir)-min_dist<EPA_TOLERANCE){
            //Convergence (new point is not significantly further from origin)
            printf("EPA converged with %d faces\n", num_faces);
            return p;
        }

<<<<<<< HEAD
        vec3 loose_edges[EPA_MAX_NUM_LOOSE_EDGES][2]; //keep track of edges we need to fix
        int num_loose_edges = 0;

        //Find all triangles that are facing p
        for(int i=0; i<num_faces; i++){
            if(dot(faces[i][3], p)>0){ //triangle i faces p, remove it
                //Update list of loose edges
                for(int j=0; j<3; j++){ //Three edges per face
                    vec3 current_edge[2] = {faces[i][j], faces[i][(j+1)%3]};
                    bool found_edge = false;
                    for(int k=0; k<num_loose_edges; k++){ //Check if current edge is already in list
                        if(loose_edges[k][1]==current_edge[0] && loose_edges[k][0]==current_edge[1]){
                            //Edge is already in the list, remove it
                            //THIS ASSUMES EDGE CAN ONLY BE SHARED BY 2 TRIANGLES (which should be true)
                            //THIS ALSO ASSUMES SHARED EDGE WILL BE REVERSED IN THE TRIANGLES (which 
                            //should be true provided every triangle is wound CCW)
                            loose_edges[k][0] = loose_edges[num_loose_edges-1][0]; //Overwrite current edge
                            loose_edges[k][1] = loose_edges[num_loose_edges-1][1]; //with last edge in list
                            num_loose_edges--;
                            found_edge = true;
                            k=num_loose_edges; //exit loop because edge can only be shared once
                        }
                    }//endfor loose_edges

                    if(!found_edge){ //add current edge to list
                        assert(num_loose_edges<EPA_MAX_NUM_LOOSE_EDGES);
                        loose_edges[num_loose_edges][0] = current_edge[0];
                        loose_edges[num_loose_edges][1] = current_edge[1];
                        num_loose_edges++;
                    }
                }

                //Remove triangle i from list
                faces[i][0] = faces[num_faces-1][0];
                faces[i][1] = faces[num_faces-1][1];
                faces[i][2] = faces[num_faces-1][2];
                faces[i][3] = faces[num_faces-1][3];
                num_faces--;
                i--;
            }//endif p can see triangle i
        }//endfor num_faces

        //Reconstruct polytope with p added
        for(int i=0; i<num_loose_edges; i++){
            assert(num_faces<EPA_MAX_NUM_FACES);
            //TODO: verify that this maintains CCW winding
            faces[num_faces][0] = loose_edges[i][0];
            faces[num_faces][1] = loose_edges[i][1];
            faces[num_faces][2] = p;
            faces[num_faces][3] = normalise(cross(loose_edges[i][0]-loose_edges[i][1], p-loose_edges[i][1]));
            num_faces++;
        }
=======
        //Add face p,v1,v2
        faces[num_faces][0] = p;
        faces[num_faces][1] = v1;
        faces[num_faces][2] = v2;
        faces[num_faces][3] = normalise(cross(v1-p, v2-p));
        //Add face p,v2,v0
        faces[num_faces+1][0] = p;
        faces[num_faces+1][1] = v2;
        faces[num_faces+1][2] = v0;
        faces[num_faces+1][3] = normalise(cross(v2-p, v0-p));
        //Overwrite v0,v1,v2 with v0,v1,p
        faces[closest_face][2] = p;
        faces[closest_face][3] = normalise(cross(v0-p, v1-p));
        num_faces+=2;
>>>>>>> 4a93b28c0901f8ac8b5ba9366b5975a1bcc8b65c
    }
    printf("EPA did not converge\n");
    return p;
}
