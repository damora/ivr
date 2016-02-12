//
//  main.c
//  IBM Ray Caster
//
//  Created by Bruce D'Amora on 10/20/12.
//  Copyright (c) 2012 Bruce D'Amora. All rights reserved->
//
//  Notes:
//  - use a right-handed coord system - consistent with OpenGL and heck I'm right handed :-)
//  - could use unit cube, but more convenient to have origin centered cube from -1,-1,-1 to 1,1,1
//  - quaternion based camera for viewing
//  - tri-linear sampling, but also have point sampling code in place
//  - added gradient over scalar field for lighint later on, but for now it isn't used
//  - working on color lut still, but really need a transfer function editor

// using this to initialize profile.h variables for timer
#define MAIN 1
#ifdef BGQ
#define HOSTNAME "172.16.3.2"
#else
#define HOSTNAME "localhost"
#endif
#define PORT "1400"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#ifdef MPI
#include <mpi.h>
#include "tiffio.h"
#endif
#ifdef __linux
#include <values.h>
#endif
#include <math.h>


#include "types.h"
#include "mathutil.h"
#include "sysutil.h"
#include "utils.h"
#include "transferfuncs.h"
#include "profile.h"
#include "colormap.h"
#include "connection.h"
#include "ray.h"
#ifdef LOCAL
#include "glui.h"
#ifdef __linux
#include <GL/gl.h>
#include <GL/glut.h>
#else
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#endif
#endif

#ifdef REMOTE
unsigned char*		tiffimage=NULL;
#endif

int			sockfd;
int 		lutoffset;
int			maxlutindx;
int 		cnt=0;
int 		startindices[3][2];
int			owneroftile = -1;
int			winwidth = WINDOWWIDTH;
int			winheight = WINDOWHEIGHT;
float       deltasample;
float 		*tilebuffer = NULL;
float 		*initbuffer = NULL;;
float		axis[4] = {0.0f, 0.0f, 1.0f, 0.0f}, angle=0.0f;
float 		*recvbuffer = NULL;
float       scale = 1.0f;
// bounding volume  face d values where n dot p = d
float pzero[6] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
color4f     *colorbuf = NULL;
color4f     *lut = NULL;
matrix3f    vwm;		// view to world matrix
matrix3f    wvm; 		// world to view matrix
matrix4f    vpm;		// ndc to window coordinates matrix
matrix4f    vvm;		// eye to view volume (ndc) matrix
matrix4f    mvm;		// model-view matrix
matrix4f    pm;			// projection  matrix
matrix4f    rotation = {{1.0f, 0.0f, 0.0f, 0.0f},
						{0.0f, 1.0f, 0.0f, 0.0f},
						{0.0f, 0.0f, 1.0f, 0.0f},
						{0.0f, 0.0f, 0.0f, 1.0f}};	// rotation  matrix
vector3f    eyepos;
vector3f    scalefactor;
#ifdef ORTHO
vector3f	delta = {0.0f, 0.0f, 0.0f};
#else
vector3f	delta = {{0.0f}, {0.0f}, {-5.0f}};
#endif
vector3f 	dir, up, e, vrp;
VOLUME      volume;
AABB        aabb;
CAMERA      camera;
FRUSTUM     viewvolume, frustum;
WINDOW      viewport;
STATE		mystate;
CMDARGS 	args;


//-----------------------------------------------------------------------------------------------------------------------//
// ray casting functions    
//-----------------------------------------------------------------------------------------------------------------------//
int  compositelocalimage(void);
int  writetomemory(int, int);
int  compositetiles(float *, float *, int,  int,  int); // composite tile function
void setplaneconst(float *, float *);
void raycast(ray3f ray, color4f *);
void createlocalimage(int);
void writeimage(int);
void writetofile(int, int);
void freeall(void);
//-----------------------------------------------------------------------------------------------------------------------//
// initialization functions
//-----------------------------------------------------------------------------------------------------------------------//
int init(CMDARGS, int);
int readvoldata(char *);
int  initvolbbox(int);
int initvolluts(void);
void initcamera(void);
void initcolbufs(int);
void initvolattrs(CMDARGS, int);
void updatevolbbox();
void readfromfile (char * , float *, int *, int *, int *);
CMDARGS parseargs(int, char **);

#ifdef LOCAL
// for drawing a bounding box around the volumetric data
GLfloat vertices[][3] = {
    {-1.0,-1.0,-1.0},{1.0,-1.0,-1.0}, {1.0,1.0,-1.0}, {-1.0,1.0,-1.0},
    {-1.0,-1.0,1.0}, {1.0,-1.0,1.0}, {1.0,1.0,1.0}, {-1.0,1.0,1.0}
};
#endif

//-----------------------------------------------------------------------------------------------------------------------//
// main  
//-----------------------------------------------------------------------------------------------------------------------//
int main(int argc,  char * argv[])
{
    int rc=0;
    int rank=0;
#ifdef MPI
    int comsize;

    // init MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &comsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    fprintf(stderr,"rank=%d\n", rank);
#ifdef BGQ
	install_signal_handler();
#endif
#endif
	
	args = parseargs(argc, argv);

	// init, initcamera, initvolattrs
	if ((rc=init(args, rank) != 0)) {
        fprintf(stderr, "initialization failed: %d\n", rc);
        return 0;
    };

    //---------------------------- Platform Specific Display and Interaction ----------------------------------------//
#if LOCAL

	resettransforms();
	mystate.alpha=0.25f;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(WINDOWWIDTH, WINDOWHEIGHT);
    glutCreateWindow("iVR");
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mousebutton);
    glutMotionFunc(mousemotion);
//    glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
    glDisable(GL_DEPTH_TEST);
    glutMainLoop();
#elif MPI		// BEGIN MPI CODE
#ifdef REMOTE 	// START OF REMOTE RENDERING HANDSHAKE

	// connect to relay server
	if (rank == 0) {
		sockfd = initconnection(HOSTNAME, PORT);
		if (sockfd == -1) {
			fprintf(stderr,"remote connection failed, sockfd=%d\n",sockfd);
			freeall();
			exit(0);
		}
		fprintf(stderr,"relay connection succeeded\n");
	}
	PRINTDEBUG("%3d: \n", rank);
	boolean_t connected = TRUE;
	int ctr = 0;
	while (connected) {  // main loop when steering
		fprintf(stderr,"%3d: before steering control\n", rank);
		if (rank == 0) {
			rc = recvstate(sockfd, &mystate);
			fprintf(stderr,"bytes recvd=%d\n", rc);		
			if (mystate.finish) connected = 0;;
			winwidth = mystate.winwidth; winheight=mystate.winheight;
		} // end if rank == 0 for remote visualization 
		// wait until we've completed the handshake with steering client before proceeding;
		MPI_Barrier(MPI_COMM_WORLD);
		MPI_Bcast(&connected, 1, MPI_INT, 0, MPI_COMM_WORLD);
		if (!connected) break;
		MPI_Bcast((float *)&winwidth, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
		MPI_Bcast((float *)&winheight, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&mystate.alpha, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
		MPI_Bcast(mystate.newconst, 6, MPI_FLOAT, 0, MPI_COMM_WORLD);
		MPI_Bcast(mystate.mvm, 16, MPI_FLOAT, 0, MPI_COMM_WORLD);
		MPI_Bcast(mystate.pm, 16, MPI_FLOAT, 0, MPI_COMM_WORLD);
		MPI_Bcast(mystate.vv, 4, MPI_FLOAT, 0, MPI_COMM_WORLD);
#endif
		float planeconst[6] = {pzero[0], pzero[1], pzero[2], pzero[3], pzero[4], pzero[5]};
		setviewport(&viewport, 0, 0, winwidth, winheight);
		setplaneconst(planeconst, mystate.newconst);
		setmvm(mvm, mystate.mvm);
		setpm(pm, mystate.pm);
		setfrustum(&viewvolume, mystate.vv);
		updatevolbbox();
		initcolbufs(rank);

    	// create and display the image
		unsigned long long lstart, lstop;
    	for (int i=0; i<1; i++) {
       		TIME(RENDER, lstart);
			createlocalimage(rank);
			// can't composite until all ranks have completed local image generation
			MPI_Barrier(MPI_COMM_WORLD);
			compositelocalimage();
       		TIME(RENDER, lstop);
       		PERFORMANCE(lstart, lstop, framenum, elapsed, avg);
    	}
		// wait until all ranks exit composite step before writing image
		MPI_Barrier(MPI_COMM_WORLD);
    	writeimage(ctr++);
#ifdef REMOTE
	} // end while remote loop
	if (rank == 0) disconnect(sockfd);
#endif // END OF REMOTE CLIENT  HANDSHAKE
    
     
	fprintf(stderr,"%3d: finished\n", rank);
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
#endif
	// clean up
	freeall();
	exit(0);
}

//-----------------------------------------------------------------------------------------------------------------------//
// cleanup
//-----------------------------------------------------------------------------------------------------------------------//
void freeall()
{
    if (volume.data.f != NULL) free (volume.data.f);
    if (volume.gradient != NULL) free (volume.gradient);
    if (lut != NULL) free (lut);
    if (colorbuf != NULL) free (colorbuf);
    if (initbuffer != NULL) free (initbuffer);
#ifdef REMOTE
	if (tiffimage != NULL) free(tiffimage);
#endif

}
//-----------------------------------------------------------------------------------------------------------------------//
// initialization
//-----------------------------------------------------------------------------------------------------------------------//
int init (CMDARGS args, int rank)
{
    int rc=0;
//	int coords[3] = {0, 0, 0};
//	int procdims[3] = {1, 1, 1};

	// initialize view
	angle = args.angle;
	axis[0] = args.axis.x;
	axis[1] = args.axis.y;
	axis[2] = args.axis.z;

	// inititalize window
	viewport.tilewid = args.tilewid;
	viewport.tilehgt = args.tilehgt;
fprintf(stderr, "rank:%d, viewport.tilewid=%d, viewport.tilehgt=%d\n", rank,viewport.tilewid, viewport.tilehgt);

	viewport.tilex = WINDOWWIDTH/viewport.tilewid;
	viewport.tiley = WINDOWHEIGHT/viewport.tilehgt;
fprintf(stderr, "rank:%d ,viewport.tilex=%d, viewport.tiley=%d\n", rank,viewport.tilex, viewport.tiley);
	viewport.w = viewport.tilex  * viewport.tilewid;
	viewport.h = viewport.tiley * viewport.tilehgt;
fprintf(stderr, "rank:%d,  viewport.w=%f, viewport.h=%f\n", rank,viewport.w, viewport.h);

	// init camera
	initcamera();

	// set window, i.e. portion of window we will render to. Need to fix this whole view mapping mess
	setviewport(&viewport, 0, 0, viewport.w, viewport.h);

	//  initialize attributes of volume both global and local
	initvolattrs(args, rank);

	// each rank reads the part of the volume it is responsible for processing
    if ((rc=readvoldata(args.filename)) < 0) {
        fprintf(stderr, "reading volume data input file failed %d\n",rc);
        freeall();
        exit(0);
    };
    

#ifdef LOCAL
    for (int i=0; i<8; i++) {
        vertices[i][0] *= scalefactor.x;
        vertices[i][1] *= scalefactor.y;
        vertices[i][2] *= scalefactor.z;
    }
#endif
    
    // initialize colormap function
	initcolormap(args.colormap);

	// initialize volume color tables
	if (args.colormap != -1) initvolluts();

	// init init color buffers
	initcolbufs(rank);

    // initialize xfer function
	inittransferfunc(args.xfer);

    
    // initialize xfer function // set to LINEAR  or NEAREST, but this should be a client menu option
	initsamplefunc(LINEAR);
    
    // initialize timers
    RESET_TIME();
    
    return 0;
}

//-----------------------------------------------------------------------------------------------------------------------//
// initcolbufs
//-----------------------------------------------------------------------------------------------------------------------//

void initcolbufs(int rank)
{
	// vpsize * 4 because RGBA framebuffer
	int vpsize = viewport.w*viewport.h;
fprintf(stderr, "rank:%d,  vpsize=%d\n", rank, vpsize);
	if (initbuffer != NULL) free(initbuffer);
	initbuffer = (float *) malloc(sizeof(float) * vpsize * ELEMENTSPERPIXEL);
	assert(initbuffer != NULL);
	memset((char *) initbuffer, 0, vpsize*sizeof(float)*ELEMENTSPERPIXEL);

	// color buffer NEED TO DO SUBARRAY TYPE FOR MPI
	if (colorbuf != NULL) free(colorbuf);
	colorbuf = (color4f *) malloc(vpsize*sizeof(color4f));
	assert(colorbuf != NULL);
	memset((char *) colorbuf, 0, vpsize*sizeof(color4f));

#ifdef REMOTE
	if (rank == 0) {
		if (tiffimage != NULL) free(tiffimage);
	    tiffimage = (unsigned char *) malloc((sizeof(char) * vpsize * ELEMENTSPERPIXEL));
    	assert (tiffimage != NULL);
	}
#endif

}


//-----------------------------------------------------------------------------------------------------------------------//
// initvolattrs
//-----------------------------------------------------------------------------------------------------------------------//
void initvolattrs(CMDARGS args, int rank) 
{
    
    int machendian;

    // determine endianess of machine
    // 1 if little endian, 0 if big endian
    machendian = endianness();

    // get the volume, allocate memory, calculate brick size,number, calculate tile size,number
    if (machendian != args.dataendian) volume.byteswap = 1;
    
    // get/set information about volume
    volume.globaldims.w = args.width;
    volume.globaldims.h = args.height;
    volume.globaldims.d = args.depth;

	// mapping
	volume.procdims.y = args.procdims[0];
	volume.procdims.x = args.procdims[1];
	volume.procdims.z = args.procdims[2];

	// compute geometry for decmposition based on global dimentions and mapping to ranks
	initvolbbox(rank);

    volume.celldims.x = 1.0f; // THIS WILL EVENTUALLY BE CONTAINED IN ARGS OR IN VOLUME DATA HEADER
    volume.celldims.y = 1.0f; // THIS WILL EVENTUALLY BE CONTAINED IN ARGS OR IN VOLUME DATA HEADER
    volume.celldims.z = 1.0f; // THIS WILL EVENTUALLY BE CONTAINED IN ARGS OR IN VOLUME DATA HEADER
    volume.fvoldims.x = (float) volume.localdims.w;
    volume.fvoldims.y = (float) volume.localdims.h;
    volume.fvoldims.z = (float) volume.localdims.d;
    volume.rfvoldims.x = 1.0f / volume.localdims.w;
    volume.rfvoldims.y = 1.0f / volume.localdims.h;
    volume.rfvoldims.z = 1.0f / volume.localdims.d;

	return;
}
//-----------------------------------------------------------------------------------------------------------------------//
// initcamera
//-----------------------------------------------------------------------------------------------------------------------//

void initcamera()
{
    
    // initialize view (u,v,n) in world coordinates
    vrp.x = vrp.y = vrp.z = 0.0f;
    dir.x = 0.0f; dir.y = 0.0f; dir.z = -1.0f;
    up.x = 0.0f; up.y = 1.0f; up.z = 0.0f;

    // initialize viewvolume in image plane (assumes that imageplane reference point is 0,0,0)
    // adjust so we have same aspect ratio as window
    float fwidth = FRUSTUMWIDTH;
	float fheight = FRUSTUMHEIGHT;
	float aspect;
    if (viewport.w > viewport.h) {
		aspect = (float) viewport.w/viewport.h;
		fwidth = fwidth * aspect;
	}
	else {
		aspect = (float) viewport.h/viewport.w;
		fheight = fheight * aspect;
	}
	frustum.bottom = -fheight/2.0f;
	frustum.top = -frustum.bottom;
	frustum.left = -fwidth/2.0f;
	frustum.right = -frustum.left;
    
    // initialize eyepos origin
#ifdef ORTHO
    e.u = 0.0f; e.v = 0.0f; e.n = -10000.0f; 	//ORTHO
	frustum.near = -2.0f;
	frustum.far = 2.0f;
#else
    e.u = 0.0f; e.v = 0.0f; e.n = -5.0f;		// PERSPECTIVE
	frustum.near = 0.1f;
	frustum.far = 100.0f;
#endif

    // initialize viewvolume in image plane (assumes that imageplane reference point is 0,0,0)
    viewvolume.bottom = frustum.bottom; viewvolume.left = frustum.left;
    viewvolume.top = frustum.top; viewvolume.right = frustum.right;
    viewvolume.far = frustum.far; viewvolume.near = frustum.near;


	// set projection matrix
#ifdef ORTHO
	setortho(scale, viewvolume);
#else
	float fovy = 2.0f * atan(frustum.top/-e.n);
	fovy = fovy * DEGREES;

	setperspective(fovy, 1.0f, 0.1f, 100.0f);
#endif

    // initialize view (u,v,n) in world coordinates
    camera.r.x = vrp.x; camera.r.y = vrp.y; camera.r.z = vrp.z;
    camera.n.x = dir.x; camera.n.y = dir.y; camera.n.z = dir.z;
    camera.v.x = up.x; camera.v.y = up.y; camera.v.z = up.z;
    camera.v = normalize3f(camera.v);
    camera.u = cross3f(camera.n, camera.v);
	camera.e.u = e.u; camera.e.v = e.v; camera.e.n = e.n;

    // set view to world transform matrix
    setvwm(vwm, camera);

    // set world to view transform matrix
    setwvm(wvm, camera);

    // initialize eyepos origin
    eyepos.u = e.u; eyepos.v = e.v; eyepos.n = e.n;


    return;
}

//-------------------------------------------------------------------------------------------------------------------//
//initvolbbox
//-------------------------------------------------------------------------------------------------------------------//
int initvolbbox(int rank)
{
	int maxcol, maxrow, maxslab, maxbox;
	int height, width, depth;
    float maxdim;
	float minx, miny, minz, maxx, maxy, maxz;
	vector3f  xyz;

	width = volume.globaldims.w;
	height = volume.globaldims.h;
	depth = volume.globaldims.d;

    // adjust the aspect ratio of bounding cube based on volume dimensions
    maxdim =  (float) (imax3(width, height, depth));
	// compute scale factor for global bounding box. Will use to draw a global bounding box
	float invdim  = 1.0f/maxdim;
    scalefactor.x = (float)width * invdim;
    scalefactor.y = (float)height * invdim;
    scalefactor.z = (float)depth * invdim;


   	maxrow = volume.procdims.y;
	maxcol = volume.procdims.x;
	maxslab = volume.procdims.z;
	maxbox = maxrow * maxcol * maxslab;

	// compute the mapping of ranks to row, columns, and slabs
	// replace this with CartCoord call
	vector3i *offsets = (vector3i *) malloc(sizeof(vector3i) * maxbox);
	assert(offsets != NULL);

	int r, c, s, indx=0;	
	for (s=0; s<maxslab; s++) {
		for(r=0; r<maxrow; r++){
			for (c=0; c<maxcol; c++) {
				offsets[indx].i=r;
				offsets[indx].j=c;
				offsets[indx].k=s;
				indx++;
			}
		}
	}
	// compute subvolume (brick) dimensions for file IO and for aabb 
	BRICK  brick;
	getlocalbrick(offsets, rank, &volume, scalefactor, OVERLAP, &brick);

	// compute the bounding box in  world coords so we have a bbox for the subvolume 
	for (int b=0; b<8; b++) {
		switch(b) {
			 case 0:
				 xyz.y = brick.y;
				 xyz.x = brick.x;
				 xyz.z = brick.z;
				 break;
			 case 1:
				 xyz.x += brick.w;
				 break;
			 case 2:
				 xyz.z += brick.d;
				 break;
			 case 3:
				 xyz.y = brick.y;
				 xyz.x = brick.x;
				 break;
			 case 4:
				 xyz.y += brick.h;
				 break;
			 case 5:
				 xyz.z = brick.z;
				 break;
			 case 6:
				 xyz.x += brick.w;
				 break;
			 case 7:
				 xyz.z += brick.d;
				 break;
		}

		aabb.verts[b].x = xyz.x;
		aabb.verts[b].y = xyz.y;
		aabb.verts[b].z = xyz.z;
		aabb.verts[b].w = 1.0f;
		aabb.rank = rank;
	}

	// compute  bbox min, max, mvmin, mvmax for this rank-> We may need later and we can avoid any interproc 
	// communications if we just do it during init
	minx = miny = minz = MAXFLOAT;
	maxx = maxy = maxz = -MAXFLOAT;
	for (int v=0; v<8; v++)  minx  = fmin(minx, aabb.verts[v].x);
	aabb.min.x = aabb.mvmin.x = minx;
	for (int v=0; v<8; v++)  miny  = fmin(miny, aabb.verts[v].y);
	aabb.min.y = aabb.mvmin.y = miny;
	for (int v=0; v<8; v++)  minz  = fmin(minz, aabb.verts[v].z);
	aabb.min.z = aabb.mvmin.z = minz;

	for (int v=0; v<8; v++)  maxx  = fmax(maxx, aabb.verts[v].x);
	aabb.max.x = aabb.mvmax.x = maxx;
	for (int v=0; v<8; v++)  maxy  = fmax(maxy, aabb.verts[v].y);
	aabb.max.y = aabb.mvmax.y = maxy;
	for (int v=0; v<8; v++)  maxz  = fmax(maxz, aabb.verts[v].z);
	aabb.max.z = aabb.mvmax.z = maxz;

	aabb.w = aabb.max.x - aabb.min.x; aabb.h = aabb.max.y - aabb.min.y; aabb.d = aabb.max.z - aabb.min.z;

	// set facenormals
	aabb.facenormals[0].x = -1.0f; aabb.facenormals[1].x = 1.0f;
	aabb.facenormals[2].y = -1.0f; aabb.facenormals[3].y = 1.0f;
	aabb.facenormals[4].z = -1.0f; aabb.facenormals[5].z = 1.0f;


    pzero[0] = aabb.min.x * aabb.facenormals[0].x; pzero[1] = aabb.max.x * aabb.facenormals[1].x; 
	pzero[2] = aabb.min.y * aabb.facenormals[2].y; pzero[3] = aabb.max.y * aabb.facenormals[3].y; 
	pzero[4] = aabb.min.z * aabb.facenormals[4].z; pzero[5] = aabb.max.z * aabb.facenormals[5].z;

	setplaneconst(pzero, pzero);

	free(offsets);
	return 0;
}

//-------------------------------------------------------------------------------------------------------------------//
//readvoldata
//-------------------------------------------------------------------------------------------------------------------//
int readvoldata(char * filename)
{
    long long localsize;

#ifndef MPI
	FILE *fd;
    
    // get volume
    if ((fd=fopen(filename, "r")) == NULL) {
        fprintf(stderr, "file open failed: %p\n", fd);
        return -1;
    }
    localsize = (long long) (volume.localdims.w * volume.localdims.h * volume.localdims.d);
    
    // allocate memory and read data
    volume.data.f = (float *) malloc(sizeof(float) * localsize);
	assert(volume.data.f != NULL);

    size_t num = fread( (float *) volume.data.f, localsize, sizeof(float), fd);
	assert(num > 0);
    fclose(fd);

#else // MPIIO distributed parallel volume rendering case
    int rank,  comsize;
    int localbytes;
    int periods[3] = {1, 1, 1};
    int startindex[3];
	int procdims[3] = {volume.procdims.y, volume.procdims.x, volume.procdims.z};
    //MPI_Status status;
	MPI_Comm comm;

	// get rank and comm size
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comsize);
	fprintf(stderr,"rank:%d, comsize:%d\n", rank, comsize);

	// in MPIIO terms dims represent row, column, slab which maps to h, w, d
    int globaldims[3] = {volume.globaldims.h, volume.globaldims.w, volume.globaldims.d};
    int localdims[3] = {volume.localdims.h, volume.localdims.w, volume.localdims.d};

	// start part where we create a cartesian coordinate communicator
	int coords[3];
    MPI_Cart_create(MPI_COMM_WORLD, 3, procdims, periods, 0, &comm);
    MPI_Comm_rank(comm, &rank);
    MPI_Cart_coords(comm, rank, 3, coords);

	// WE WILL USE THE VOLUME BEGIN AND ORIGIN WE COMPUTED IN INITVOLDIMS
	// start of subarray definition
    startindex[0]  =  volume.startindex.y;
    startindex[1]  =  volume.startindex.x;
    startindex[2]  =  volume.startindex.z;

    localsize = localdims[0] * localdims[1] * localdims[2];
    localbytes = localsize * sizeof(float);
	fprintf(stderr,"rank: %d, localbytes=%d\n", rank, localbytes);
    volume.data.f = (float *) malloc(localbytes);
	assert(volume.data.f != NULL);

	readfromfile(filename, volume.data.f, globaldims, localdims, startindex);
	fprintf(stderr,"rank: %d, startindex[0], startindex[1], startindex[2]=%d, %d, %d\n", rank, startindex[0], startindex[1], startindex[2]);
    //MPI_Get_count(&status, MPI_FLOAT, &count);
	MPI_Comm_free(&comm);

#endif

	// if there is byteswapping we'll do it here
    if (volume.byteswap) {
        for (int i=0; i<localsize; i++) {
            float in,out;
            in = volume.data.f[i];
            
            out = BYTESWAP_FP(in);
            volume.data.f[i] = out;
        }
    }

    return 0;
}

int compositelocalimage()
{

#ifdef MPI
	int rank, comsize;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &comsize);


	// create aabb array to store bboxes for all ranks
	AABB *abboxes=NULL, *bboxesorg=NULL;
	RECT *tiles=NULL;
	int hit=0;
	int numtiles = viewport.tilex * viewport.tiley;
	int x, y, w, h;

	bboxesorg = (AABB *) malloc(sizeof(AABB) * comsize);
	tiles = (RECT *) malloc(sizeof(RECT) * numtiles);
	abboxes = (AABB *) malloc(sizeof(AABB) * comsize);
	assert (bboxesorg != NULL && tiles != NULL && abboxes != NULL);

    // allgather parameters
    float *sbboxes=NULL;
    float *rbboxes=NULL;
	sbboxes =  (float *) malloc(sizeof(float) * MAXPLANE);
	rbboxes =  (float *) malloc(sizeof(float) * comsize * MAXPLANE);
	assert (sbboxes != NULL && rbboxes != NULL);

	sbboxes[0] = aabb.wincoord.x;
	sbboxes[1] = aabb.wincoord.y;
	sbboxes[2] = aabb.wincoord.z;
	sbboxes[3] = aabb.wincoord.w ;
	sbboxes[4] = aabb.wincoord.h ;
	sbboxes[5] = aabb.wincoord.d ;

	// Should we avoid doing this and just compute all teh bboxes for all ranks on each rank
    MPI_Allgather(sbboxes, MAXPLANE, MPI_FLOAT, rbboxes, MAXPLANE, MPI_FLOAT, MPI_COMM_WORLD);

	// could avoid this copy by splitting bboxes into two structures - rbboxes and bboxes
	for (int i=0, j=0; i<comsize; i++) {
		abboxes[i].wincoord.x = rbboxes[j++];
		abboxes[i].wincoord.y = rbboxes[j++];
		abboxes[i].wincoord.z = rbboxes[j++];
		abboxes[i].wincoord.w = rbboxes[j++];
		abboxes[i].wincoord.h = rbboxes[j++];
		abboxes[i].wincoord.d = rbboxes[j++];
        abboxes[i].wincoord.cx = abboxes[i].wincoord.x +  abboxes[i].wincoord.w/2.0f;
        abboxes[i].wincoord.cy = abboxes[i].wincoord.y +  abboxes[i].wincoord.h/2.0f;
		abboxes[i].rank = i;
	}
	memcpy(bboxesorg, abboxes, sizeof(aabb) * comsize);

	// sort by depth from near to far
	sortboxlist(0, comsize-1, abboxes);

	// compute overlap for each tile and subtile
	int (*rankspertile)[numtiles] = malloc(comsize * sizeof(*rankspertile));
	assert(*rankspertile != NULL);

	//memset((void **) rankspertile, -1, sizeof(int) * comsize * numtiles);
	for (int m=0; m<comsize; m++)  {
		for (int n=0; n<numtiles; n++) {
			rankspertile[m][n] = -1;
		}
	}

	// create tile parameters based on number of tiles and size of display
	createtiles(tiles, viewport);

	for (int j=0; j<numtiles; j++) {
		for (int i=0; i<comsize; i++) {
			// if the bbox overlaps the tile you compute the overlap region, 
			// and place the rank # in the tiles[i] list array
			hit=intersectrect(abboxes[i].wincoord, tiles[j]);
			if (hit) { 
				rankspertile[i][j] = abboxes[i].rank;
				PRINTDEBUG("%3d: TILE: %d -BOXRANK: %d -  BBOXDEPTH: %f\n", rank, j, abboxes[i].rank, abboxes[i].wincoord.d);
			}
		}
	}	

#if 0
	// there are h rows each representing a block
	// each row is the w of the tile times 4 floats (RGBA)
	// the next block is WINDOWWIDTH*4 (RGBA) floats  from the current block position
	int w = (int) (tiles[rank].w/* + 0.5f*/);
	int h = (int) (tiles[rank].h/* + 0.5f*/);
	int x = (int) (tiles[rank].x/* + 0.5f*/);
	int y = (int) (tiles[rank].y/* + 0.5f*/);
	// init initbuffer
	initbuffer = (float *) malloc(sizeof(float) * w * h * ELEMENTSPERPIXEL);
	assert(initbuffer != NULL);
	for (int i=0,j=0; i<w*h; i++) {
		initbuffer[j] = 0.0f;
		initbuffer[j+1] = 0.0f;
		initbuffer[j+2] = 0.0f;
		initbuffer[j+3] = 1.0f;
		j += 4;
	}
#endif	

	int *r = (int *) malloc(sizeof(int) * comsize);
	int *s = (int *) malloc(sizeof(int) * comsize);
	assert (r != NULL && s != NULL);

	int rcv;
	int snd;
	MPI_Request rrequests[comsize];
	MPI_Request srequests[comsize];
#ifdef IVR_DEBUG
	MPI_Status  statuses[comsize];
#endif

	rcv = 0;
	snd = 0;
	for (int j=0; j<numtiles; j++) {
		w = (int) (tiles[j].w/* + 0.5f*/);
		h = (int) (tiles[j].h/* + 0.5f*/);
		x = (int) (tiles[j].x/* + 0.5f*/);
		y = (int) (tiles[j].y/* + 0.5f*/);

		fprintf(stderr,"tile=%d w,h=%d,%d\n", j,w,h);

		if (j == rank) owneroftile = j;
		for (int i=0; i<comsize; i++) {
			if (rankspertile[i][j] != -1) {
				if (owneroftile == j) {
					r[rcv] = rankspertile[i][j];
					rcv++;
				}
				if (rankspertile[i][j] == rank) {
					s[snd] = j;
					snd++;
				}
			}
		}
	}
	
	int ns = 0;
	int nr = 0;
	float *rcolorbuf = NULL; 
	if (rcv > 0) {
    	rcolorbuf = (float *) malloc(sizeof(float) * w * h * rcv * ELEMENTSPERPIXEL);
		assert(rcolorbuf != NULL);

#ifdef IVR_DEBUG
    	for (int i=0; i<rcv; i++) {
			PRINTDEBUG("%3d: rcv=%d, i=%d, fromrnk %d\n", rank, rcv, i, r[i]);
		}
#endif
		if (owneroftile == rank) {
               for (int i = 0; i < rcv; ++i) {
               		float *rbuf = (rcolorbuf + w*h*ELEMENTSPERPIXEL*i);
   	            	MPI_Irecv(rbuf, w*h*4, MPI_FLOAT, r[i], 0, MPI_COMM_WORLD, &rrequests[nr++]);
           	}
		}
	}
	float *scolorbuf = NULL;
	if (snd > 0) {
#ifdef IVR_DEBUG
    	for (int i=0; i<snd; i++) {
			PRINTDEBUG("%3d: snd=%d, i=%d, torank %d\n", rank, snd, i, s[i]);
		}
#endif
		scolorbuf = (float*) malloc(sizeof(float) * w *h * ELEMENTSPERPIXEL * snd);
		assert(scolorbuf != NULL);

		for (int i=0; i<snd; i++) {
			x = (int) (tiles[s[i]].x/* + 0.5f*/);
			y = (int) (tiles[s[i]].y/* + 0.5f*/);
			w = (int) (tiles[s[i]].w/* + 0.5f*/);
			h = (int) (tiles[s[i]].h/* + 0.5f*/);
	  		start = y*viewport.w + x;

			//PRINTDEBUG("%3d: START-XY =%d, %d, %f, %f\n", s[i], x, y);

            float *sbuf = (scolorbuf + w*h*ELEMENTSPERPIXEL*i);
			float *scolors = (float *) &(colorbuf[start]);
			
			stridememcpy(sbuf, scolors, h, w*ELEMENTSPERPIXEL, viewport.w*ELEMENTSPERPIXEL, sizeof(float));
		 	MPI_Isend(sbuf, w*h*ELEMENTSPERPIXEL, MPI_FLOAT, s[i], 0, MPI_COMM_WORLD, &srequests[ns++]);
		}
	    free(scolorbuf);
	}
				
	 MPI_Waitall(nr, rrequests, MPI_STATUSES_IGNORE);
	 MPI_Waitall(ns, srequests, MPI_STATUSES_IGNORE);

	// free(scolorbuf);
  	//All data is in rcolorbuf on rank==r[0]
  	if (owneroftile == rank) { //rank owns tile and there is something to composite 
		int elements = w*h*ELEMENTSPERPIXEL;
		tilebuffer = (float *) malloc(elements*sizeof(float));
		assert(tilebuffer != NULL);

		memcpy(tilebuffer, initbuffer, elements*sizeof(float));
		if (rcv > 0) {
			PRINTDEBUGTILES(rcv, rank, elements, rcolorbuf);
			compositetiles(tilebuffer, rcolorbuf,  rcv, w, h);
  			free(rcolorbuf);
		}
#ifdef IVR_DEBUG2
		PRINTDEBUG("%3d: rcv=%d\n", rank, rcv);
		char fn[32];
		sprintf(fn, "tile%d.raw", rank);
		FILE *fout = fopen(fn,"w+b");
		fwrite(tilebuffer, elements,sizeof(float), fout);
		fclose(fout);
#endif
  	//	free(rcolorbuf);
	}
                
	free(r);
	free(s);
#if 0
	free(initbuffer);
#endif
	free((*rankspertile));
	free(tiles);
	free(abboxes);
	free(sbboxes);
	free(rbboxes);
	free(bboxesorg);
#endif
	return 0;
}

// create and display volume rendered image
void createlocalimage(int rank)
{
	int i=0, j=0;
    float u, v;
    float du, dv;
    ray3f ray, tray;
	vector3f incri, incrj;
    color4f color;

	// initialize framebuffer
	memset(colorbuf, 0, sizeof(color4f)*viewport.w*viewport.h);
    
    // initiliaze deltas for ray stepping
    du = (viewvolume.right - viewvolume.left)/viewport.w;
    dv = (viewvolume.top - viewvolume.bottom)/viewport.h;

    deltasample = fmin(du, dv);
    
    // buffers index
    int fbindex = 0;
    
    // cast rays for each pixel in window
	int istrt = (int) (aabb.wincoord.y - 0.5f);
	int istop = (int) (aabb.wincoord.y + aabb.wincoord.h + 1.0f);
	int jstrt = (int) (aabb.wincoord.x - 0.5f);
	int jstop = (int) (aabb.wincoord.x + aabb.wincoord.w + 1.0f);
#ifdef IVR_DEBUG
	fprintf(stderr, "rank=%d, istrt=%d, istop=%d, jstrt=%d, jstop=%d\n", rank, istrt, istop, jstrt, jstop);
#endif
	
	// DEBUG 
	/*
	istrt = (int) (0);
	istop = (int) (viewport.h);
	jstrt = (int) (0);
	jstop = (int) (viewport.w);
	*/
	// END DEBUG

	// initial ray direction
   	v = viewvolume.bottom + istrt * dv + dv/2.0f;
   	u = viewvolume.left + jstrt * du - du/2.0f;

	ray.dir.u = u; ray.dir.v = v; ray.dir.n = -camera.e.n;
    buildray(du, dv, &incri, &incrj, &ray);
					
#pragma omp parallel for private(j, color, fbindex, tray) firstprivate(ray)
    for (i=istrt; i < istop; i++)
    {
		tray.orig.x = ray.orig.x; tray.orig.y = ray.orig.y; tray.orig.z= ray.orig.z;
		tray.dir.x = ray.dir.x + incri.x*(i-istrt); 
		tray.dir.y = ray.dir.y + incri.y*(i-istrt); 
		tray.dir.z = ray.dir.z + incri.z*(i-istrt);

        for (j=jstrt; j < jstop; j++)
        {
            // default color is black
            color.r = color.g = color.b = color.a = 0.0f;

			// increment for next ray; SIMD this
			tray.dir.x += incrj.x;
			tray.dir.y += incrj.y;
			tray.dir.z += incrj.z;
            raycast(tray, &color);

			// increment color buffer pointer 
            fbindex = i * viewport.w  + j;
            colorbuf[fbindex].r = color.r;
            colorbuf[fbindex].g = color.g;
            colorbuf[fbindex].b = color.b;
            colorbuf[fbindex].a = color.a;
        }
    }
    
    
#ifdef LOCAL
    // Load array to 0 0 0 to set the raster origition to (0, 0, 0)
    GLfloat rp[3];
    rp[0] =  0.0f;
    rp[1] =  0.0f;
    rp[2] =  0.0f;
    // Set the raster origition and pass the array of colours to drawPixels
    // use colors directly, don't need to copy.
    glRasterPos3fv(rp);
    glDrawPixels(winwidth, winheight, GL_RGBA, GL_FLOAT, (GLvoid *)colorbuf);
	fprintf(stderr,"w=%d, h=%d\n", winwidth, winheight);
#endif
    
    return;
}

void writeimage(int a)
{
	int rank = 0;

#ifdef MPI
	// write out files for each rank for debug, will remove this later
   	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif

	DEBUGSUBVOLUMES(colorbuf, rank);

	writetofile(rank, a);

	writetomemory(rank,a);

	return;

}

CMDARGS parseargs(int argc, char * argv[])
{
   CMDARGS args = {{'\0'}, 0, 0, 0, 0, -1, -1, {-1, -1, -1}, -1, -1, -1, {{0.0f}, {0.0f}, {0.0f}}};
   if (argc == 17) {
        strcpy (args.filename, argv[1]);
        args.width = atoi(argv[2]);
        args.height = atoi(argv[3]);
        args.depth = atoi(argv[4]);
        args.dataendian = atoi(argv[5]);
        args.colormap = atoi(argv[6]);
        args.xfer = atoi(argv[7]);
		args.procdims[0] = atoi(argv[8]);
		args.procdims[1] = atoi(argv[9]);
		args.procdims[2]= atoi(argv[10]);
		args.tilewid = atoi(argv[11]);
		args.tilehgt = atoi(argv[12]);
		args.angle = atof(argv[13]);
		args.axis.x = atof(argv[14]);
		args.axis.y = atof(argv[15]);
		args.axis.z = atof(argv[16]);
      }
    else {
        fprintf(stderr, "command: raycast <volume filename> <volume width> <volume height> <volume depth> <endianess = 1 (big), 0 (little) <colormap = 0,1,...n>, <transferfunc = 0,1,...n>, processes x, processes y, processes z, tiles x, tiles y, angle, axis x, axis y, axisz\n");
        return(args);
    }
	return args;
}

int initvolluts()
{
#ifdef MPI
	int comsize, rank;
	int indx;
	int *rhsizes;
	int *globalhist;
	int *roffsets;
	int *rhists = NULL;
	float *rminvalues;
	float *rmaxvalues;
#endif
    int *hist = NULL;
	int hsize;
	float minvalue, maxvalue;

	/////////////////////////////// compute global color table ///////////////////////////////////////
    // compute local histogram and create a colormap from all local histograms
    hsize = histogram(&hist, volume.data.f, &minvalue, &maxvalue, volume.localdims.w, volume.localdims.h, volume.localdims.d);

	// broadcast hist, minvalue, maxvalue
#ifdef MPI
  //  MPI_Comm_size(comm, &comsize);
   // MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	rminvalues = (float *) malloc(sizeof(float) * comsize);
	rmaxvalues = (float *) malloc(sizeof(float) * comsize);
	assert (rminvalues != NULL && rmaxvalues != NULL); 

	rhsizes = (int *) malloc(sizeof(int) * comsize);
	assert (rhsizes != NULL); 

	// allgatherv parameters
	roffsets = (int *) malloc(sizeof(int) * comsize);
	assert (roffsets != NULL);

	//MPI_Allgather( &minvalue, 1, MPI_FLOAT, rminvalues, 1, MPI_FLOAT, comm);
	MPI_Allgather( &minvalue, 1, MPI_FLOAT, rminvalues, 1, MPI_FLOAT, MPI_COMM_WORLD);
	//MPI_Allgather( &maxvalue, 1, MPI_FLOAT, rmaxvalues, 1, MPI_FLOAT, comm);
	MPI_Allgather( &maxvalue, 1, MPI_FLOAT, rmaxvalues, 1, MPI_FLOAT, MPI_COMM_WORLD);
	//MPI_Allgather( &hsize, 1, MPI_INT, rhsizes, 1, MPI_INT, comm);
	MPI_Allgather( &hsize, 1, MPI_INT, rhsizes, 1, MPI_INT, MPI_COMM_WORLD);

	int d = 0;
	for (int i=0; i<comsize; i++) {
		roffsets[i] = d;
		d += rhsizes[i];
	}
	rhists = (int *) malloc(sizeof(int) * d);
	assert (rhists != NULL);

	//MPI_Allgatherv(hist, hsize,  MPI_INT, rhists, rhsizes, roffsets, MPI_INT, comm);
	MPI_Allgatherv(hist, hsize,  MPI_INT, rhists, rhsizes, roffsets, MPI_INT, MPI_COMM_WORLD);
	
	float tmpmin = MAXFLOAT;
	float tmpmax =  -MAXFLOAT;
	for (int i=0; i<comsize; i++) {
		minvalue = fmin(tmpmin, rminvalues[i]);
		maxvalue = fmax(tmpmax, rmaxvalues[i]);
		tmpmin = minvalue;
		tmpmax = maxvalue;
	}

	PRINTDEBUG("%3d:  MINMAX- %f, %f\n", rank, minvalue, maxvalue);

	hsize = (int) ((maxvalue - minvalue) + 1);
	globalhist =  (int *) malloc(sizeof(int) * hsize);
	assert(globalhist != NULL);

	for (int j=0; j<hsize; j++) globalhist[j] = 0;
	int rindx;


	for (int i=0; i<comsize; i++) {
		rindx = roffsets[i];	// get offset into individual histogram arrays
		indx = rminvalues[i] - minvalue;
		for (int j=0; j<rhsizes[i]; j++) {
			globalhist[indx+j] += rhists[rindx+j];
		}
	}
	
    lut = (color4f *) malloc(hsize * sizeof(color4f));
	maxlutindx = hsize - 1;
	assert (lut != NULL);

    colormap(lut, globalhist, hsize, minvalue, maxvalue);
	
	// free all the temp stuff
	free(rminvalues);
	free(rmaxvalues);
	free(rhsizes);
	free(rhists);
	free(roffsets);
	free(globalhist);

#else
    lut = (color4f *) malloc(hsize * sizeof(color4f));
	assert (lut != NULL); 

    colormap(lut, hist, hsize, minvalue, maxvalue);
#endif

	lutoffset = (int) (minvalue + 0.5);
    
#ifdef GRAD
	/////////////////////////////// compute subvolume gradient	//////////////////////////////////////
	// local gradients are sufficient, i.e. a subvol does not need gradient values from another subvol
    // compute gradient and store with rest of volume data
    int n = volume.localdims.w;
    int m = volume.localdims.h;
    int p = volume.localdims.d;
    
	long long localsize = n * m * p;
    // allocate gradient storage
    volume.gradient = (vector4f *) malloc(localsize * sizeof(vector4f));
	assert (volume.gradient != NULL);

	// local gradients are sufficient, i.e. a subvol does not need gradient values from another subvol
    gradient3d(volume.gradient, volume.data.f, n, m,  p);
#endif

	free(hist);
	return 0;
}    

// stitch the tiles together
int compositetiles(float *tilebuf, float *rcolorbuf, int num, int w, int h)
{

    for (int k = num-1; k >= 0; k--) {
        color4f *d = (color4f *) (tilebuf);
        color4f *s = (color4f *) (rcolorbuf+k*w*h*4);
        for (int i=0; i<w*h; i++, d+=1, s+=1) {
            d->b = s->b + (1.0f-s->a) * d->b;
            d->g = s->g + (1.0f-s->a) * d->g;
            d->r = s->r + (1.0f-s->a) * d->r;
            d->a = s->a + (1.0f-s->a) * d->a;
        }
    }
    
    return 0;
}

void readfromfile(char * filename, float * values, int *gdims, int *ldims, int *sindex)
{
	int gw, gh, gd, lw, lh, ld, sx, sy, sz;
	int count, offset;
	float *buf = (float *) values;


	fprintf(stderr,"gdims=[%d, %d, %d], ldims=[%d, %d, %d], startindex=[%d, %d, %d]\n", gdims[0],gdims[1], gdims[2], ldims[0], ldims[1], ldims[2], sindex[0], sindex[1], sindex[2]);

	gh = gdims[0];
	gw = gdims[1];
	gd = gdims[2];
	lh = ldims[0];
	lw = ldims[1];
	ld = ldims[2];
	sy = sindex[0];
	sx = sindex[1];
	sz = sindex[2];

	count = ldims[1]; 

	FILE *fin = fopen(filename, "r+b");
#if ORGFILEIO
    for (int z=0; z<ldims[2]; z++) {
        for (int y=0; y<ldims[0]; y++) {
			offset = maptoffset3d(sy+y , sx, sz+z, gw, gh, gd);
			fseek(fin, offset*sizeof(float), SEEK_SET);
            fread(buf, sizeof(float), count, fin);
			buf += count;
		}
	}
#else
	count = gdims[0] *gdims[1];
	float *tmp = (float *)malloc(count * sizeof(float));
    for (int z=0; z<ldims[2]; z++) {
		offset = maptoffset3d(0 , 0, sz+z, gw, gh, gd);
		fseek(fin, offset*sizeof(float), SEEK_SET);
        fread(tmp, sizeof(float), count, fin);
		for (int y=0; y<ldims[0]; y++) {
			for (int x=0; x<ldims[1]; x++) {
				int indxsrc = maptoffset3d(sy+y, sx+x, 0, gw, gh, gd);
				int indxdst = maptoffset3d(y, x, z, ldims[1], ldims[0], ldims[2]);
				buf[indxdst ] = tmp[indxsrc];
			}
		}
	}
#endif
		
	return;
}

void writetofile(int rank, int a)
{
#if defined(MPI) && defined(FILEIMAGE)
	// dump all the tiles to 1 file using MPI-IO
	int color = 0;
	MPI_Comm tilecomm;
	MPI_File  fhtout;
	if (owneroftile == rank) color = 1; 
	PRINTDEBUG("%3d: tileowner=%d, color=%d\n", rank, owneroftile, color);
	MPI_Comm_split(MPI_COMM_WORLD, color, rank, &tilecomm);
	if (color) {
		int cartrank;
		MPI_Comm cartcomm;
		const MPI_Status status;
		// init parameters for Cart_create
		int gsizes[2] = {viewport.h, viewport.w*ELEMENTSPERPIXEL};
		int psizes[2] = {viewport.tiley, viewport.tilex};
		int lsizes[2] = {gsizes[0]/psizes[0], (gsizes[1])/psizes[1]};
		int dims[2] = {psizes[0], psizes[1]};
		int periods[2] = {1,1};
		int coords[2];
		PRINTDEBUG("%3d: gsizes=%d,%d, psizes=%d,%d, lsizes=%d,%d\n", rank, gsizes[0], gsizes[1], psizes[0], psizes[1], 
					lsizes[0], lsizes[1]);
		// Create cartesian coord
		MPI_Cart_create(tilecomm, 2, dims, periods, 0, &cartcomm);
		MPI_Comm_rank(cartcomm, &cartrank);
		MPI_Cart_coords(cartcomm, cartrank, 2, coords);
		// check coords	
		PRINTDEBUG("%3d: coords=%d,%d\n", cartrank, coords[0], coords[1]);

		/* global indices of first element of local array */
		int startindex[2];
		startindex[0] = coords[0] * lsizes[0];
		startindex[1] = coords[1] * lsizes[1];

		PRINTDEBUG("%3d: start=%d,%d\n", cartrank, startindex[0], startindex[1]);
		int filetype;
		filetype=0;
		MPI_Type_create_subarray(2, gsizes, lsizes, startindex, MPI_ORDER_C, MPI_FLOAT, &filetype);
		MPI_Type_commit(&filetype);

		char imagefull[32];
		sprintf(imagefull,"imagefull%d.raw", a);
		PRINTDEBUG("%s\n", imagefull);
		MPI_File_open(cartcomm, imagefull, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fhtout);
		MPI_File_set_view(fhtout, 0, MPI_FLOAT, filetype, "native", MPI_INFO_NULL);
		int localarraysize = lsizes[0] * lsizes[1];
		PRINTDEBUG("%3d: size=%d\n", cartrank, localarraysize);

		MPI_File_write(fhtout, tilebuffer, localarraysize, MPI_FLOAT, &status);
		int count;
    	MPI_Get_count(&status, MPI_FLOAT, &count);
		PRINTDEBUG("%3d: cnt=%d\n", cartrank, count);
		MPI_File_close(&fhtout);
		MPI_Comm_free(&cartcomm);
		MPI_Type_free(&filetype);
//		free(tilebuffer);
	}
	MPI_Comm_free(&tilecomm);
#endif
}

int writetomemory(int rank, int a)
{

#if defined(MPI) && defined(MEMIMAGE)

	int color = 0;
	int rc=0;
	MPI_Comm tilecomm;
	if (owneroftile == rank) color = 1; 
	MPI_Comm_split(MPI_COMM_WORLD, color, rank, &tilecomm);
	if (color) {
		int cartsize, cartrank;
		MPI_Comm cartcomm;
		MPI_Status status;

		// init parameters for Cart_create
		int gsizes[2] = {viewport.h, viewport.w*4};
		int psizes[2] = {viewport.tilex, viewport.tiley};
		int lsizes[2] = {gsizes[0]/psizes[0], (gsizes[1])/psizes[1]};
		int dims[2] = {psizes[0], psizes[1]};
		int periods[2] = {1,1};
		int coords[2];
		int *indices = NULL;
		float *tmprecvbuffer = NULL;

		// Create cartesian coord
		MPI_Cart_create(tilecomm, 2, dims, periods, 0, &cartcomm);
		MPI_Comm_rank(cartcomm, &cartrank);
		MPI_Comm_size(cartcomm, &cartsize);
		MPI_Cart_coords(cartcomm, cartrank, 2, coords);

		/* global indices of first element of local array */
		int startindex[2];
		startindex[0] = coords[0] * lsizes[0];
		startindex[1] = coords[1] * lsizes[1];

		int localarraysize = lsizes[0] * lsizes[1];
		int globalarraysize = gsizes[0] * gsizes[1];
		PRINTDEBUG("%3d: start=%d,%d\n", cartrank, startindex[0], startindex[1]);

		if (cartrank == 0) {
			recvbuffer = (float *) malloc(sizeof(float) * globalarraysize);
			tmprecvbuffer = (float *) malloc(sizeof(float) * globalarraysize);
			indices = (int *) malloc(sizeof(int) * 2 * cartsize);
			assert (recvbuffer != NULL && tmprecvbuffer != NULL && indices != NULL); 
		}

		MPI_Gather(tilebuffer, localarraysize, MPI_FLOAT, tmprecvbuffer, localarraysize, MPI_FLOAT, 0, cartcomm);
		MPI_Gather(startindex, 2, MPI_INT, indices, 2, MPI_INT, 0, cartcomm);
		if (cartrank == 0) {
			int e=0,s=0;
			int i=0;
			int rindx, cindx; // row and column indices into temprecvbuffer, recvbuffer
			int numtiles = viewport.tilex * viewport.tiley;
			for (int n=0,indx=0; n<numtiles; n++) {
				rindx = indices[indx];
				cindx = indices[indx+1];
				s = (rindx * gsizes[1]) + cindx;; 
				e = s;
				for (int r=0;r<lsizes[0]; r++) {
					for (int c=0; c<lsizes[1]; c++) {
						recvbuffer[e] = tmprecvbuffer[i];
						e++;
						i++;
					}
					s +=  gsizes[1];
					e = s;
				}
				indx += 2;
			}
#ifdef REMOTE
			for (int i=0; i<globalarraysize; i++)  tiffimage[i] = (unsigned char) (recvbuffer[i] * 255.0 + 0.5f);
			rc = sendimage(sockfd, (unsigned char *)tiffimage, globalarraysize);
#endif
			free(recvbuffer);
			free(tmprecvbuffer);
			free(indices);
		}
		free(tilebuffer);
		MPI_Comm_free(&cartcomm);
	}
	MPI_Comm_free(&tilecomm);
#endif
    return 0;
}
