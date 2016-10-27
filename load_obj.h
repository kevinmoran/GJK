#include <stdio.h>
#include <stdlib.h>

//Kevin's wavefront obj loading functions

//unindexed
bool load_obj(const char* file_name, float** points, int* point_count);
bool load_obj(const char* file_name, float** points, float** tex_coords, float** normals, int* point_count);
//indexed
bool load_obj_indexed(const char* file_name, float** points, unsigned short** indices, 
					int* point_count, int* vert_count);
bool load_obj_indexed(const char* file_name, float** points, float** tex_coords, float** normals, 
					unsigned short** indices, int* point_count, int* vert_count);

//-------------------------------------------------------------------------------------------------------------

//Load unindexed points (i.e. returns a triangulated points array), ignore tex coords and normals if present
bool load_obj(const char* file_name, float** points, int* point_count){
	FILE* fp = fopen(file_name, "r");
	if(!fp) {
		printf("Error: Failed to open %s\n", file_name);
		fclose(fp);
		return false;
	}
	printf("Loading obj: '%s'\n", file_name);

	int num_vps = 0;
	int num_vts = 0;
	int num_vns = 0;
	int num_faces = 0;

	char line[1024];
	line[0] = '\0';

	//Count number of elements
	while(fgets(line, 1024, fp)){
		if(line[0]=='v'){
			if(line[1]==' ') num_vps++;
			else if(line[1]=='t') num_vts++;
			else if(line[1]=='n') num_vns++;
		}
		else if(line[0]=='f') num_faces++;
	}
	rewind(fp);

	printf("Num vps: %i\n", num_vps);
	printf("Num vts: %i\n", num_vts);
	printf("Num vns: %i\n", num_vns);
	printf("Num faces: %i\n", num_faces);

	*point_count = 3*num_faces;
	float* vp_unsorted = (float*)malloc(num_vps*3*sizeof(float)); //temp
	*points = (float*)malloc(*point_count*3*sizeof(float));
	printf("Allocated %u bytes for obj\n", (unsigned int)(*point_count)*3*sizeof(float));

	//Iterators
	int vp_index = 0; //for unsorted points
	int points_index = 0; //for sorted points

	while(fgets(line, 1024, fp)){\
		if(line[0]=='v' && line[1]==' '){
			sscanf(line, "v %f %f %f", &vp_unsorted[vp_index], &vp_unsorted[vp_index+1], &vp_unsorted[vp_index+2]);
			vp_index+=3;
		}
		else if(line[0]=='f'){
			int indices[3];
			//Scan the line depending on what parameters are included for faces
			if(num_vts==0 && num_vns==0){ //Just vertex positions
				sscanf(line, "f %i %i %i", &indices[0], &indices[1], &indices[2]);
			}
			else if(num_vts==0){ //vertex positions and normals
				sscanf(line, "f %i//%*i %i//%*i %i//%*i", &indices[0], &indices[1], &indices[2]);
			}
			else if(num_vns==0){ //vertex positions and tex coords
				sscanf(line, "f %i/%*i %i/%*i %i/%*i", &indices[0], &indices[1], &indices[2]);
			}
			else{ //vertex positions and tex coords and normals
				sscanf(line, "f %i/%*i/%*i %i/%*i/%*i %i/%*i/%*i", &indices[0], &indices[1], &indices[2]);
			}
			for(int i=0; i<3; i++){ //For the 3 vertices in this face
				(*points)[points_index+3*i  ] = vp_unsorted[3*(indices[i]-1)];   //x // (indices[i]-1) -> wavefront obj doesn't use zero indexing
				(*points)[points_index+3*i+1] = vp_unsorted[3*(indices[i]-1)+1]; //y
				(*points)[points_index+3*i+2] = vp_unsorted[3*(indices[i]-1)+2]; //z
			}
			points_index+=9;
		}//end elseif for 'f'
	}//endwhile

	fclose(fp);
	free(vp_unsorted);
	return true;
}

//Load unindexed points, tex coords and normals
bool load_obj(const char* file_name, float** points, float** tex_coords, float** normals, int* point_count){
	FILE* fp = fopen(file_name, "r");
	if(!fp) {
		printf("Error: Failed to open %s\n", file_name);
		fclose(fp);
		return false;
	}
	printf("Loading obj: '%s'\n", file_name);

	int num_vps = 0;
	int num_vts = 0;
	int num_vns = 0;
	int num_faces = 0;

	char line[1024];
	line[0] = '\0';

	//Count number of elements
	while(fgets(line, 1024, fp)){
		if(line[0]=='v'){
			if(line[1]==' ') num_vps++;
			else if(line[1]=='t') num_vts++;
			else if(line[1]=='n') num_vns++;
		}
		else if(line[0]=='f') num_faces++;
	}
	rewind(fp);

	printf("Num vps: %i\n", num_vps);
	printf("Num vts: %i\n", num_vts);
	printf("Num vns: %i\n", num_vns);
	printf("Num faces: %i\n", num_faces);

	*point_count = 3*num_faces;
	//points, tex coords and normals arrays that will be sorted based on indices in file
	*points = (float*)malloc(*point_count*3*sizeof(float));
	if(num_vts>0) *tex_coords = (float*)malloc(*point_count* 2*sizeof(float));
	if(num_vns>0) *normals = (float*)malloc(*point_count* 3*sizeof(float));

	unsigned int mem_alloced = *point_count* 2*sizeof(float);
	if(num_vts>0) mem_alloced += *point_count* 2*sizeof(float);
	if(num_vns>0) mem_alloced += *point_count* 3*sizeof(float);
	printf("Allocated %u bytes for obj\n", mem_alloced);

	//Arrays to hold the unsorted data from the obj
	float* vp_unsorted = (float*)malloc(num_vps*3*sizeof(float));
	float* vt_unsorted = (float*)malloc(num_vts*2*sizeof(float));
	float* vn_unsorted = (float*)malloc(num_vns*3*sizeof(float));

	//Iterators
	int vp_index = 0; //unsorted points
	int vt_index = 0; //unsorted tex coords
	int vn_index = 0; //unsorted normals
	int points_index = 0; //for sorted points
	int tex_coords_index = 0; //for sorted tex coords
	int normals_index = 0; //for sorted normals

	while(fgets(line, 1024, fp)){\
		if(line[0]=='v'){
			//Read in unsorted data (vp, vt, vn)
			if(line[1]==' '){
				sscanf(line, "v %f %f %f", &vp_unsorted[vp_index], &vp_unsorted[vp_index+1], &vp_unsorted[vp_index+2]);
				vp_index+=3;
			}
			else if(line[1]=='t'){
				sscanf(line, "vt %f %f", &vt_unsorted[vt_index], &vt_unsorted[vt_index+1]);
				vt_index+=2;
			}
			else if(line[1]=='n'){
				sscanf(line, "vn %f %f %f", &vn_unsorted[vn_index], &vn_unsorted[vn_index+1], &vn_unsorted[vn_index+2]);
				vn_index+=3;
			}
		}
		else if(line[0]=='f'){
			int indices[3];
			//Scan the line depending on what parameters are included for faces
			if(num_vts==0 && num_vns==0){ //Just vertex positions
				sscanf(line, "f %i %i %i", &indices[0], &indices[1], &indices[2]);
				for(int i=0; i<3; i++){ //For the 3 vertices in this face
					(*points)[points_index+3*i  ] = vp_unsorted[3*(indices[i]-1)];   //x // (indices[i]-1) -> wavefront obj doesn't use zero indexing
					(*points)[points_index+3*i+1] = vp_unsorted[3*(indices[i]-1)+1]; //y
					(*points)[points_index+3*i+2] = vp_unsorted[3*(indices[i]-1)+2]; //z
				}
				points_index+=9;
			}
			else if(num_vts==0){ //vertex positions and normals
				int vn[3];
				sscanf(line, "f %i//%i %i//%i %i//%i", &indices[0], &vn[0], &indices[1], &vn[1], &indices[2], &vn[2]);
				for(int i=0; i<3; i++){ //For the 3 vertices in this face
					(*points)[points_index+3*i  ] = vp_unsorted[3*(indices[i]-1)  ]; //x // (indices[i]-1) -> wavefront obj doesn't use zero indexing
					(*points)[points_index+3*i+1] = vp_unsorted[3*(indices[i]-1)+1]; //y
					(*points)[points_index+3*i+2] = vp_unsorted[3*(indices[i]-1)+2]; //z
					(*normals)[normals_index+3*i  ] = vn_unsorted[3*(vn[i]-1)  ];		//x
					(*normals)[normals_index+3*i+1] = vn_unsorted[3*(vn[i]-1)+1];		//y
					(*normals)[normals_index+3*i+2] = vn_unsorted[3*(vn[i]-1)+2];		//z
				}
				points_index+=9;
				normals_index+=9;
			}
			else if(num_vns==0){ //vertex positions and tex coords
				int vt[3];
				sscanf(line, "f %i/%i %i/%i %i/%i",	&indices[0], &vt[0], &indices[1], &vt[1], &indices[2], &vt[2]);
				for(int i=0; i<3; i++){ //For the 3 vertices in this face
					(*points)[points_index+3*i  ] = vp_unsorted[3*(indices[i]-1)  ]; //x // (indices[i]-1) -> wavefront obj doesn't use zero indexing
					(*points)[points_index+3*i+1] = vp_unsorted[3*(indices[i]-1)+1]; //y
					(*points)[points_index+3*i+2] = vp_unsorted[3*(indices[i]-1)+2]; //z
					(*tex_coords)[tex_coords_index+2*i  ] = vt_unsorted[2*(vt[i]-1)  ];	//u
					(*tex_coords)[tex_coords_index+2*i+1] = vt_unsorted[2*(vt[i]-1)+1];	//v
				}
				points_index+=9;
				tex_coords_index+=6;
			}
			else{ //vertex positions and tex coords and normals
				int vt[3], vn[3];
				sscanf(line, "f %i/%i/%i %i/%i/%i %i/%i/%i",&indices[0], &vt[0], &vn[0], 
															&indices[1], &vt[1], &vn[1], 
															&indices[2], &vt[2], &vn[2]);
				for(int i=0; i<3; i++){ //For the 3 vertices in this face
					(*points)[points_index+3*i  ] = vp_unsorted[3*(indices[i]-1)  ]; //x // (indices[i]-1) -> wavefront obj doesn't use zero indexing
					(*points)[points_index+3*i+1] = vp_unsorted[3*(indices[i]-1)+1]; //y
					(*points)[points_index+3*i+2] = vp_unsorted[3*(indices[i]-1)+2]; //z
					(*tex_coords)[tex_coords_index+2*i  ] = vt_unsorted[2*(vt[i]-1)  ];	//u
					(*tex_coords)[tex_coords_index+2*i+1] = vt_unsorted[2*(vt[i]-1)+1];	//v
					(*normals)[normals_index+3*i  ] = vn_unsorted[3*(vn[i]-1)  ];		//x
					(*normals)[normals_index+3*i+1] = vn_unsorted[3*(vn[i]-1)+1];		//y
					(*normals)[normals_index+3*i+2] = vn_unsorted[3*(vn[i]-1)+2];		//z
				}
				points_index+=9;
				tex_coords_index+=6;
				normals_index+=9;
			}
		}
	}
	fclose(fp);

	free(vp_unsorted);
	free(vt_unsorted);
	free(vn_unsorted);

	return true;
}

//Load vertex points with index buffer, ignore tex coords and normals if present
bool load_obj_indexed(const char* file_name, float** points, unsigned short** indices, 
					int* point_count, int* vert_count){
	FILE* fp = fopen(file_name, "r");
	if(!fp) {
		printf("Error: Failed to open %s\n", file_name);
		fclose(fp);
		return false;
	}
	printf("Loading obj: '%s'\n", file_name);

	int num_vps = 0;
	int num_vts = 0;
	int num_vns = 0;
	int num_faces = 0;

	char line[1024];
	line[0] = '\0';

	//Count number of elements
	while(fgets(line, 1024, fp)){
		if(line[0]=='v'){
			if(line[1]==' ') num_vps++;
			else if(line[1]=='t') num_vts++;
			else if(line[1]=='n') num_vns++;
		}
		else if(line[0]=='f') num_faces++;
	}
	rewind(fp);

	printf("Num vps: %i\n", num_vps);
	printf("Num vts: %i\n", num_vts);
	printf("Num vns: %i\n", num_vns);
	printf("Num faces: %i\n", num_faces);

	if(num_faces>(65536)/3){
		printf("ERROR loading obj: Too many faces for index buffer of shorts\n");
		return false;
	}

	*point_count = 3*num_faces;
	*vert_count = num_vps;
	*points = (float*)malloc(num_vps*3*sizeof(float));
	*indices = (unsigned short*)malloc(*point_count*sizeof(unsigned short));
	printf("Allocated %u bytes for obj\n", (unsigned int)(num_vps*3*sizeof(float) + (*point_count)*sizeof(unsigned short)));

	//Iterators
	int vp_index = 0;
	int idxs_i = 0; //iterator for index buffer

	while(fgets(line, 1024, fp)){\
		if(line[0]=='v'){
			if(line[1]==' '){
				sscanf(line, "v %f %f %f", &(*points)[vp_index], &(*points)[vp_index+1], &(*points)[vp_index+2]);
				vp_index+=3;
			}
		}
		else if(line[0]=='f'){
			//Scan the line depending on what parameters are included for faces
			if(num_vts==0 && num_vns==0){ //Just vertex positions
				sscanf(line, "f %hu %hu %hu",  &(*indices)[idxs_i], &(*indices)[idxs_i+1], &(*indices)[idxs_i+2]);
			}
			else if(num_vts==0){ //vertex positions and normals
				sscanf(line, "f %hu//%*u %hu//%*u %hu//%*u", &(*indices)[idxs_i], &(*indices)[idxs_i+1], &(*indices)[idxs_i+2]);
			}
			else if(num_vns==0){ //vertex positions and tex coords
				sscanf(line, "f %hu/%*u %hu/%*u %hu/%*u", &(*indices)[idxs_i], &(*indices)[idxs_i+1], &(*indices)[idxs_i+2]);
			}
			else{ //vertex positions and tex coords and normals
				sscanf(line, "f %hu/%*u/%*u %hu/%*u/%*u %hu/%*u/%*u", &(*indices)[idxs_i], &(*indices)[idxs_i+1], &(*indices)[idxs_i+2]);
			}
			for(int i=0; i<3; i++){
				(*indices)[idxs_i+i]--; //wavefront obj doesn't use zero indexing
			}
			idxs_i+=3;
		}//end elseif for 'f'
	}//endwhile
	fclose(fp);
	return true;
}

//Load points, tex coords and normals with index buffer
bool load_obj_indexed(const char* file_name, float** points, float** tex_coords, float** normals, 
				unsigned short** indices, int* point_count, int* vert_count){
	FILE* fp = fopen(file_name, "r");
	if(!fp) {
		printf("Error: Failed to open %s\n", file_name);
		fclose(fp);
		return false;
	}
	printf("Loading obj: '%s'\n", file_name);

	int num_vps = 0;
	int num_vts = 0;
	int num_vns = 0;
	int num_faces = 0;

	char line[1024];
	line[0] = '\0';

	//Count number of elements
	while(fgets(line, 1024, fp)){
		if(line[0]=='v'){
			if(line[1]==' ') num_vps++;
			else if(line[1]=='t') num_vts++;
			else if(line[1]=='n') num_vns++;
		}
		else if(line[0]=='f') num_faces++;
	}
	rewind(fp);

	printf("Num vps: %i\n", num_vps);
	printf("Num vts: %i\n", num_vts);
	printf("Num vns: %i\n", num_vns);
	printf("Num faces: %i\n", num_faces);

	if(num_faces>(65536)/3){
		printf("ERROR loading obj: Too many faces for index buffer of shorts\n");
		return false;
	}
	
	*point_count = 3*num_faces;
	*vert_count = num_vps;
	*points = (float*)malloc(num_vps*3*sizeof(float));
	*indices = (unsigned short*)malloc(*point_count*sizeof(unsigned short));
	//vt and vn arrays that will be sorted based on index buffer
	if(num_vts>0) *tex_coords = (float*)malloc(*point_count * 2*sizeof(float));
	if(num_vns>0) *normals = (float*)malloc(*point_count * 3*sizeof(float));
	
	unsigned int mem_alloced = num_vps*3*sizeof(float) + (*point_count)*sizeof(unsigned short);
	if(num_vts>0) mem_alloced += *point_count * 2*sizeof(float);
	if(num_vns>0) mem_alloced += *point_count * 3*sizeof(float);
	printf("Allocated %u bytes for obj\n", mem_alloced);

	//Arrays to hold the unsorted data from the obj
	float* vt_unsorted = (float*)malloc(2*num_vts*sizeof(float));
	float* vn_unsorted = (float*)malloc(3*num_vns*sizeof(float));

	//Iterators
	int vp_index = 0;
	int vt_index = 0;
	int vn_index = 0;
	int idxs_i = 0; //iterator for index buffer

	while(fgets(line, 1024, fp)){\
		if(line[0]=='v'){
			if(line[1]==' '){
				sscanf(line, "v %f %f %f", &(*points)[vp_index], &(*points)[vp_index+1], &(*points)[vp_index+2]);
				vp_index+=3;
			}
			else if(line[1]=='t'){
				sscanf(line, "vt %f %f", &vt_unsorted[vt_index], &vt_unsorted[vt_index+1]);
				vt_index+=2;
			}
			else if(line[1]=='n'){
				sscanf(line, "vn %f %f %f", &vn_unsorted[vn_index], &vn_unsorted[vn_index+1], &vn_unsorted[vn_index+2]);
				vn_index+=3;
			}
		}
		else if(line[0]=='f'){
			//Scan the line depending on what parameters are included for faces
			if(num_vts==0 && num_vns==0){ //Just vertex positions
				sscanf(line, "f %hu %hu %hu",  &(*indices)[idxs_i],
											&(*indices)[idxs_i+1],
											&(*indices)[idxs_i+2]);
				//wavefront doesn't use zero indexing
				(*indices)[idxs_i]--; (*indices)[idxs_i+1]--; (*indices)[idxs_i+2]--;
				idxs_i+=3;
			}
			else if(num_vts==0){ //vertex positions and normals
				int vn[3];
				sscanf(line, "f %hu//%i %hu//%i %hu//%i",  &(*indices)[idxs_i],	&vn[0], 
														&(*indices)[idxs_i+1],	&vn[1], 
														&(*indices)[idxs_i+2],	&vn[2]);
				for(int i=0; i<3; i++){ //add normals for the 3 verts in this face
					(*indices)[idxs_i+i]--; //wavefront doesn't use zero indexing
					vn[i]--;

					(*normals)[3*(idxs_i+i)]   = vn_unsorted[3*vn[i]];
					(*normals)[3*(idxs_i+i)+1] = vn_unsorted[3*vn[i]+1];
					(*normals)[3*(idxs_i+i)+2] = vn_unsorted[3*vn[i]+2];
				}
				idxs_i+=3;
			}
			else if(num_vns==0){ //vertex positions and tex coords
				int vt[3];
				sscanf(line, "f %hu/%i %hu/%i %hu/%i",	&(*indices)[idxs_i], 	&vt[0], 
														&(*indices)[idxs_i+1],	&vt[1], 
														&(*indices)[idxs_i+2],	&vt[2]);
				for(int i=0; i<3; i++){ //add tex coords for the 3 verts in this face
					(*indices)[idxs_i+i]--; //wavefront doesn't use zero indexing
					vt[i]--;

					(*tex_coords)[2*(idxs_i+i)]   = vt_unsorted[2*vt[i]];
					(*tex_coords)[2*(idxs_i+i)+1] = vt_unsorted[2*vt[i]+1];
				}
				idxs_i+=3;
			}
			else{ //vertex positions and tex coords and normals
				int vt[3], vn[3];
				sscanf(line, "f %hu/%i/%i %hu/%i/%i %hu/%i/%i",	&(*indices)[idxs_i],	&vt[0], &vn[0], 
																&(*indices)[idxs_i+1],	&vt[1], &vn[1], 
																&(*indices)[idxs_i+2],	&vt[2], &vn[2]);
				for(int i=0; i<3; i++){ //add normals and tex coords for the 3 verts in this face
					(*indices)[idxs_i+i]--; //wavefront doesn't use zero indexing
					vt[i]--;
					vn[i]--;

					(*tex_coords)[2*(idxs_i+i)]   = vt_unsorted[2*vt[i]];
					(*tex_coords)[2*(idxs_i+i)+1] = vt_unsorted[2*vt[i]+1];

					(*normals)[3*(idxs_i+i)]   = vn_unsorted[3*vn[i]];
					(*normals)[3*(idxs_i+i)+1] = vn_unsorted[3*vn[i]+1];
					(*normals)[3*(idxs_i+i)+2] = vn_unsorted[3*vn[i]+2];
				}
				idxs_i+=3;
			}
		}
	}
	fclose(fp);

	free(vt_unsorted);
	free(vn_unsorted);

	return true;
}
