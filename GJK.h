
// "Implementing GJK" by Casey Muratori:
// https://www.youtube.com/watch?v=Qupqu1xe7Io

vec3 support(Shape shape1, Shape shape2, vec3 dir){
    vec3 p1 = get_farthest_point_in_dir(shape1, dir);
    vec3 p2 = get_farthest_point_in_dir(shape2, dir);

    return (p1-p2);
}

bool gjk(){
    vec3 simplex[4]; //Just a set of points
    simplex[0] = some_initial_point;
    vec3 search_dir = -simplex[0]; //search in direction of origin
    int i = 0;

    while(true){
        vec3 new_point = support(search_dir);
        if(dot(new_point, search_dir)<0) return false; //we didn't reach the origin, won't enclose it
        simplex[i++] = new_point; //add new point to our list
        if(do_simplex(simplex, i, search_dir)) return true;
    }
}

//If simplex is a tetrahedron return true if it encloses the origin
//Otherwise,  get part of simplex closest to origin.
//Make that the new simplex and search in direction of origin

bool do_simplex(float[4] &simplex, int dim, vec3 search_dir){
    if(dim = 1){
        //Simplex is a line segment AB
        //Which is closer to the origin, A or AB?

        //Take plane with point A and normal AB
        //Find out which side of the plane origin is on

        vec3 A = simplex[1];
        vec3 B = simplex[0];
        vec3 AB = simplex[0] - simplex[1];

        if(dot(AB,-A)>0){
            simplex = {A, B};
            search_dir = cross(cross(-A, AB), AB);
        }
        else {
            simplex = {A};
            search_dir = -A;
        }

    }
    else if(dim = 2){
        //Simplex is triangle ABC
        //Which is closer to origin?
        //point A, B or C, edge AB, BC or AC,
        //or triangle ABC (above or below)
        //Irrelevant cases: (BC was old simplex)
        //origin can't be closest to BC, B or C

        //So the cases we actually care about are:
        //Origin is closest to point A,
        //edges AB or AC or triangle ABC (above or below)

        vec3 ABC = cross(AB, AC); //orthogonal to face of triangle

        if(dot(cross(ABC, AC), -A)>0){
            //origin is in direction of AC's normal
            if(dot(AC, -A)>0){
                simplex = {A,C};
                search_dir = cross(cross(AC, -A), AC);
            }
            else *
        }
        else if (dot(cross(AB,ABC), -A)>0) *
        else if (dot(ABC, -A)>0){
            simplex = [A,B,C]; 
            search_dir = ABC;
        }
        else
            simplex = [A,C,B];
            search_dir = -ABC;
        }

        * if (dot(AB, -A)>0) {
            simplex = [A,B];
            search_dir = cross(cross(AB,-A), AB);
        }
        else {
            simplex = [A];
            search_dir = -A;
        }
    }
}
