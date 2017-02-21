#pragma once
#include "GameMaths.h"

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

vec3 support(const Collider &shape, vec3 dir);
bool gjk(Collider coll1, Collider coll2, vec3* mtv);
void update_simplex3(vec3 &a, vec3 &b, vec3 &c, vec3 &d, int &simp_dim, vec3 &search_dir);
bool update_simplex4(vec3 &a, vec3 &b, vec3 &c, vec3 &d, int &simp_dim, vec3 &search_dir);
vec3 EPA(vec3 a, vec3 b, vec3 c, vec3 d, Collider coll1, Collider coll2);

struct Collider{
	vec3    pos;            //origin in world space
	float   *points;        //array of verts (x0 y0 z0 x1 y1 z1 etc)
	int     num_points;     //num vertices 
    mat3    matRS;          //rotation/scale component of model matrix
	mat3    matRS_inverse; 
};

vec3 support(const Collider &shape, vec3 dir){
    dir = shape.matRS_inverse*dir; //transform dir by shape's inverse rotation matrix

    vec3 furthest_point = vec3(shape.points[0], shape.points[1], shape.points[2]);
    float max_dot = dot(furthest_point, dir);

    for(int i=3; i<shape.num_points*3; i+=3){
        vec3 v = vec3(shape.points[i], shape.points[i+1], shape.points[i+2]);
        float d = dot(v, dir);
        if(d>max_dot){
            max_dot = d;
            furthest_point = v;
        }
    }
    vec3 final = shape.matRS*furthest_point + shape.pos;
    //printf("Support: ");
    //print(final);
    return final;
}

bool gjk(Collider coll1, Collider coll2, vec3* mtv){
    vec3 a, b, c, d; //Simplex: just a set of points (a is always most recently added)
    vec3 search_dir = coll1.pos - coll2.pos; //initial search direction between colliders

    //Get initial point for simplex
    c = support(coll2, search_dir) - support(coll1, -search_dir);
    search_dir = -c; //search in direction of origin

    //Get second point for a line segment simplex
    b = support(coll2, search_dir) - support(coll1, -search_dir);

    if(dot(b, search_dir)<0) {
        //printf("GJK No collision (search didn't reach origin). Exited before loop\n");
        return false; //we didn't reach the origin, won't enclose it
    }

    search_dir = cross(cross(c-b,-b),c-b); //search normal to line segment towards origin
    if(search_dir==vec3(0,0,0)){
        //origin is on this line segment
        //Apparently any normal search vector will do?
        search_dir = cross(c-b, vec3(1,0,0)); //normal with x-axis
        if(search_dir==vec3(0,0,0)) search_dir = cross(c-b, vec3(0,0,-1)); //normal with z-axis
    }
    int simp_dim = 2; //simplex dimension
    
    for(int iterations=0; iterations<64; iterations++){
        //printf("Search direction: ");
        // print(search_dir);

        a = support(coll2, search_dir) - support(coll1, -search_dir);
        if(dot(a, search_dir)<0) {
            //printf("GJK No collision (search didn't reach origin). Iterations: %d\n", iterations);
            return false; //we didn't reach the origin, won't enclose it
        }
        simp_dim++;

        if(simp_dim==3){
            update_simplex3(a,b,c,d,simp_dim,search_dir);
        }
        else if(update_simplex4(a,b,c,d,simp_dim,search_dir)) {
            //printf("GJK Collision. Iterations: %d\n", iterations);
            *mtv = EPA(a,b,c,d,coll1,coll2);
            printf("Minimum translation vector:\n");
            print(*mtv);
            return true;
        }
    }//endfor
    //printf("GJK No collision. Ran out of iterations\n");
    return false;
}

//Triangle case
void update_simplex3(vec3 &a, vec3 &b, vec3 &c, vec3 &d, int &simp_dim, vec3 &search_dir){
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

    simp_dim = 2; //hoisting this just cause
    //Closest to edge AB
    if(dot(cross(b-a, n), AO)>0){
        c = a;
        //simp_dim = 2;
        search_dir = cross(cross(b-a, AO), b-a);
        return;
    }
    //Closest to edge AC
    if(dot(cross(n, c-a), AO)>0){
        b = a;
        //simp_dim = 2;
        search_dir = cross(cross(c-a, AO), c-a);
        return;
    }
    
    simp_dim = 3; //hoisting this just cause
    //Above triangle
    if(dot(n, AO)>0){
        d = c;
        c = b;
        b = a;
        //simp_dim = 3;
        search_dir = n;
        return;
    }
    //else
    //Below triangle
    d = b;
    b = a;
    //simp_dim = 3;
    search_dir = -n;
    return;
}

//Tetrahedral case
bool update_simplex4(vec3 &a, vec3 &b, vec3 &c, vec3 &d, int &simp_dim, vec3 &search_dir){
    // a is peak/tip of pyramid, BCD is the base (counterclockwise winding order)
	//We know a priori that origin is above BCD and below a

    //Get normals of three new faces
    vec3 ABC = cross(b-a, c-a);
    vec3 ACD = cross(c-a, d-a);
    vec3 ADB = cross(d-a, b-a);

    vec3 AO = -a; //dir to origin
    simp_dim = 3; //hoisting this just cause

    //Plane-test origin with 3 faces
    /*
    // Note: Kind of primitive approach used here; If origin is in front of a face, just use it as the new simplex.
    // We just go through the faces sequentially and exit at the first one which satisfies dot product. Not sure this 
    // is optimal or if edges should be considered as possible simplices? Thinking this through in my head I feel like 
    // this method is good enough. Makes no difference for AABBS, should test with more complex colliders.
    */
    if(dot(ABC, AO)>0){
    	//In front of ABC
    	d = c;
    	c = b;
    	b = a;
        search_dir = ABC;
        //update_simplex3(a,b,c,d,simp_dim,search_dir);
    	return false;
    }
    if(dot(ACD, AO)>0){
    	//In front of ACD
    	b = a;
        search_dir = ACD;
        //update_simplex3(a,b,c,d,simp_dim,search_dir);
    	return false;
    }
    if(dot(ADB, AO)>0){
    	//In front of ADB
    	c = d;
    	d = b;
    	b = a;
        search_dir = ADB;
        //update_simplex3(a,b,c,d,simp_dim,search_dir);
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
#define EPA_MAX_NUM_FACES 256
#define EPA_MAX_NUM_LOOSE_EDGES 128
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
    faces[3][1] = d;
    faces[3][2] = c;
    faces[3][3] = normalise(cross(d-b, c-b)); //BDC
    // printf("\nStarted EPA\n");
    // printf("a: ");
    // print(a);
    // printf("b: ");
    // print(b);
    // printf("c: ");
    // print(c);
    // printf("d: ");
    // print(d);

    int num_faces=4;
    vec3 p; //new point used to expand polytope
    
    while(num_faces<EPA_MAX_NUM_FACES){
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
        // printf("min dist: %f\n", min_dist);

        //search normal to face that's closest to origin
        vec3 search_dir = faces[closest_face][3]; 
        // printf("Search dir: :");
        // print(search_dir);
        
        p = support(coll2, search_dir) - support(coll1, -search_dir);
        // printf("New point: ");
        // print(p);

        if(dot(p, search_dir)-min_dist<EPA_TOLERANCE){
            //Convergence (new point is not significantly further from origin)
            printf("EPA converged with %d faces\n", num_faces);
            return faces[closest_face][3]*dot(p, search_dir); //dot vertex with normal to resolve collision along normal!
        }

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
        printf("Num loose edges: %d\n", num_loose_edges);
        
        //Reconstruct polytope with p added
        for(int i=0; i<num_loose_edges; i++){
            assert(num_faces<EPA_MAX_NUM_FACES);
            //TODO: verify that this maintains CCW winding
            faces[num_faces][0] = loose_edges[i][0];
            faces[num_faces][1] = loose_edges[i][1];
            faces[num_faces][2] = p;
            faces[num_faces][3] = normalise(cross(loose_edges[i][0]-loose_edges[i][1], loose_edges[i][0]-p));
            num_faces++;
        }
    }
    printf("EPA did not converge\n");
    return p;
}
