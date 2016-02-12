#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <memory.h>
#include <string.h>
#include <sys/file.h>
#include <sys/types.h>

#include "types.h"
#include "utils.h"
#include "tiffio.h"

//void readtiff(unsigned int *, unsigned int **);
extern int winwidth, winheight;

void writetotiff(char *img, long int xs, long int ys, long int pl, float *image)
{
	writetiffheader(img, xs, ys, pl);
	writetiffdata(&img[512], xs, ys, pl, image);
}

void writetiffheader (char *img, long int xs, long int ys, long int pl ) 
{
	unsigned char *b;
  	unsigned char  buf[512]; 
  	char           cnt;
 
	memset(buf, 0, 512);

	//  TIFF file header.
  	buf[0] = 0x4d;
  	buf[1] = 0x4d;
  	buf[3] = 42;
  	buf[7] = 10;

  	cnt = 0;

  	b = buf + 12;

	// New subfile type.
  	b = make_tag ( 254, LONG, 1, 0, b, &cnt );

  	// Image width.
  	b = make_tag ( 256, SHORT, 1, (xs<<16), b, &cnt );

	// Image length.
  	b = make_tag ( 257, SHORT, 1, (ys<<16), b, &cnt );

  	// Bits per sample.
  	if ( pl == 3 ) {
    	b = make_tag ( 258, SHORT, 3, 256, b, &cnt );
  	}
  	else {
   	 	b = make_tag ( 258, SHORT, 1, (8<<16), b, &cnt );
  	}

  	// Compression.
  	b = make_tag ( 259, SHORT, 1, (1<<16), b, &cnt );

  	// Photo interp.
  	if ( pl == 3 ) {
    	b = make_tag ( 262, SHORT, 1, (2<<16), b, &cnt );
  	}
  	else {
    	b = make_tag ( 262, SHORT, 1, (1<<16), b, &cnt );
  	}

  	// Strip offset.
  	b = make_tag ( 273, LONG, 1, 512, b, &cnt );

  	// Samples per pixel.
  	b = make_tag ( 277, SHORT, 1, (pl<<16), b, &cnt );

  	// Rows per strip.
  	b = make_tag ( 278, SHORT, 1, (ys<<16), b, &cnt);

  	// Strip byte count.
  	b = make_tag ( 279, LONG, 1, (ys*xs*pl), b, &cnt );

  	// Planar configuration.
  	b = make_tag ( 284, SHORT, 1, (1<<16), b, &cnt );

  	// Next IFD.
  	b = make_tag ( 0, 0, 0, 0, b, &cnt );

  	cnt--;
  	buf[11] = cnt;

  	if ( pl == 3 ) {
    	buf[257] = 8;
    	buf[259] = 8;
    	buf[261] = 8;
  	}

  	memcpy( img, buf, 512);

  	return;
}

void writetiffdata ( char *img, long int xs, long int ys, long int pl, float *image ) 
{

  float  		*db;
  float  		*dg;
  float  		*dr;
  long int       x;
  long int       y;
  unsigned char *l;
  unsigned char *line;

  line = ( unsigned char * ) malloc ( xs*pl );
  assert(line != NULL);

  for ( y = 0; y < ys; y++ ) {
    dr = image + (y * xs*4);
    dg = (image+1) + (y * xs*4);
    db = (image+2) + (y * xs*4);
    l = line;
    for ( x = 0; x < xs; x++ ) {
      *l++ = *dr*255; dr+=4;
      if ( pl > 1 ) {    
        *l++ = *dg*255; dg+=4;
        *l++ = *db*255; db+=4;
      }
    }
    memcpy ( &img[y*xs*pl], line, (xs*pl));
  }

  free ( line );
  return;
}

unsigned char *make_tag ( short int tag, short int type, long int lng, long int fld, unsigned char *b, char *cnt ) 
{

  *b++ = ( unsigned char ) ( (tag >> 8)         &  0xff );
  *b++ = ( unsigned char ) (  tag               &  0xff );
  *b++ = ( unsigned char ) ( (type    >> 8)     &  0xff );
  *b++ = ( unsigned char ) (  type              &  0xff );
  *b++ = ( unsigned char ) ( (lng     >> 24)    &  0xff );
  *b++ = ( unsigned char ) ( (lng     >> 16)    &  0xff );
  *b++ = ( unsigned char ) ( (lng     >> 8)     &  0xff );
  *b++ = ( unsigned char ) (  lng               &  0xff );
  *b++ = ( unsigned char ) ( (fld     >> 24)    &  0xff );
  *b++ = ( unsigned char ) ( (fld     >> 16)    &  0xff );
  *b++ = ( unsigned char ) ( (fld     >> 8)     &  0xff );
  *b++ = ( unsigned char ) (  fld               &  0xff );

  *cnt = *cnt + 1;

  return ( b );
}
/*
void readtiff(unsigned int *tif, unsigned int **raster)
{
    if (tif) {
        TIFFRGBAImage img;
        char emsg[1024];

        if (TIFFRGBAImageBegin(&img, tif, 0, emsg)) {
            size_t npixels;
            npixels = img.width * img.height;
            *raster = (unsignd int *) _TIFFmalloc(npixels * sizeof (unsignd int));
            if (*raster != NULL) {
                TIFFRGBAImageGet(&img, *raster, img.width, img.height);
            }
            TIFFRGBAImageEnd(&img);
        } else
            TIFFError(tif, emsg);
    }
}
*/
void sortboxlist(int l, int r, AABB *boxlist)
{
  int   i, j;
  AABB tmp, mid;

  i = l;
  j = r;
  mid = boxlist[(l+r)/2];

  do {
    while (boxlist[i].wincoord.d < mid.wincoord.d) i++;
    while (boxlist[j].wincoord.d > mid.wincoord.d) j--;

    if ( i <= j ) {
      tmp = boxlist[i];
      boxlist[i] = boxlist[j];
      boxlist[j] = tmp;
      i++;
      j--;
    }
  } while ( i <= j );

  if ( l < j ) sortboxlist( l, j, boxlist);
  if ( i < r ) sortboxlist( i, r, boxlist);
}

void createtiles(RECT *t, WINDOW vp)
{
	int cnt, tnrow, tncol;
	float tw, th;

	tnrow =  vp.tiley;
	tncol =  vp.tilex;
	tw =  vp.tilewid;
	th =  vp.tilehgt;
	
	cnt = 0;
	for (int i=0; i<tnrow; i++) {
		for (int j=0; j<tncol; j++) {
			t[cnt].x = j * tw;
			t[cnt].w = tw;	
			t[cnt].y = i * th; 
			t[cnt].h = th;
			t[cnt].cx = t[cnt].x +  t[cnt].w/2.0f;
			t[cnt].cy = t[cnt].y +  t[cnt].h/2.0f;
			cnt++;
		}
		
	}
	return;
}

int intersectrect(RECT r0, RECT r1)
{
	float a,b,c,d;
	int intsect;

	a = fabs(r0.cx-r1.cx);
	b = fabs(r0.w+r1.w)/2.0f; 
	c = fabs(r0.cy-r1.cy);
	d = fabs(r0.h+r1.h)/2.0f; 
	intsect = (a < b) && (c<d);
	if (intsect) {
		return 1;
	}
	else {
		return 0;
	}
}

void stridememcpy(float *dest, float *src, int blocknum, int blocklen, int stride, int elemsz)
{

	float *dtemp = dest;
	float *stemp = src;
#ifdef IVR_DEBUG
	fprintf(stderr,"blocknum=%d, blocklen=%d, stride=%d\n",blocknum, blocklen, stride);
#endif

	for (int i=0; i<blocknum; i++) {
		memcpy(dtemp,  stemp, blocklen*elemsz);
		dtemp += blocklen;
		stemp +=  stride;
	}

}

int slicingcalc(int totsize, int minslicesize, int nslices, int log2align, int overlap, int rank, int *slicesize, int *sliceorigin)
{
    // Given the total number of points in one dimension, the minimum number
    // of points required for an MPI transfer, the number of bricks
    // desired, the alignment required (0=no alignment, 1=multiple of 2
    // elements, 2=multiple of 4, 3=multiple of 8, etc.), and the rank,
    // calculate the size and origin of the rank'th bricks. Exit if alignment
    // requested is not valid or if problem size is not a multiple of the
    // requested alignment.
    // N.B. "rank" runs from 0 to nslices-1.

    int mask, minsize, ns, bottom, top;

   	if (nslices == 1) {
        *sliceorigin = 0;
        *slicesize = totsize;
        return(0);
    }

    // Alignment required must be none, 2, 4, 8, 16, or 32 elements; the
    // code will work for more, but a larger request is likely to be an
    // error, so this test is provided to make sure the programmer realizes
    // what he/she has requested.
    if (log2align<0 || log2align>5) {
        printf("slicingCalc error: log2align = %d, must be in range 0 to 5\n",log2align);
        return(-1);
    }

    // For now, require each dimension to be an exact multiple of the required
    // alignment. This could be relaxed by padding the input.
    mask = (1<<log2align)-1;
    if ((totsize&mask)!=0) {
        printf("slicingCalc error: totsize = %d, must be a multiple of required alignment %d\n",totsize,1<<log2align);
        return(-1);
    }
    // Minimum bricks size is stencil size rounded up to alignment boundary

    minsize = ((minslicesize+mask)>>log2align)<<log2align;
    if ((minsize*nslices)>=totsize) {
        // Must use bricks of the minimum size; use as many as needed, and
        // do not use the remaining bricks. Note that the last bricks may be
        // slightly larger.
        ns=totsize/minsize;  // number of bricks which can be made; note rounding down
        if (rank<ns) {
            *sliceorigin = rank*minsize;
            if (rank<(ns-1)) {
                *slicesize = minsize;
            } else {
                *slicesize=totsize-(rank*minsize);
            }
        } else {
            *sliceorigin = -1;
            *slicesize = -1;
        }
    } else {
        // We have enough points to give some to all bricks; take the ideal
        // boundaries and round them to the nearest aligned boundaries.

        bottom = rank*totsize/nslices;         // ideal boundaries
        top = (rank+1)*totsize/nslices;
        bottom += (1<<log2align)>>1;           // add half of alignment
        top += (1<<log2align)>>1;
        top = (top>>log2align)<<log2align;     // round down to aligned boundary
        bottom = (bottom>>log2align)<<log2align;
		bottom = (rank != 0) ? bottom-overlap : bottom;
        *sliceorigin = bottom;
        *slicesize = top-bottom;
    }
	return 0;
}
void getlocalbrick(vector3i *offsets, int rank, VOLUME *volume, vector3f scalefactor, int overlap, BRICK *brick)
{
	vector3i remainder;

	
	volume->localdims.w =(volume->globaldims.w)/(volume->procdims.x);
	volume->localdims.h =(volume->globaldims.h)/(volume->procdims.y);
	volume->localdims.d =(volume->globaldims.d)/(volume->procdims.z);

	remainder.w = volume->globaldims.w % (volume->localdims.w * volume->procdims.x);
	remainder.h = volume->globaldims.h % (volume->localdims.h * volume->procdims.y);
	remainder.d = volume->globaldims.d % (volume->localdims.d * volume->procdims.z);

	volume->startindex.x = offsets[rank].j * volume->localdims.w;
	volume->startindex.y = offsets[rank].i * volume->localdims.h;
	volume->startindex.z = offsets[rank].k * volume->localdims.d;

	// adjust for remainders
	if (offsets[rank].j == volume->procdims.x-1)
		volume->localdims.w += remainder.w;
	if (offsets[rank].i == volume->procdims.y-1)
		volume->localdims.h += remainder.h;
	if (offsets[rank].k == volume->procdims.z-1)
		volume->localdims.d += remainder.d;

	// compute brick dimensions for aabb
	vector3f ratio;
	vector3f range;

	range.x = scalefactor.x * 2.0f;
	range.y = scalefactor.y * 2.0f;
	range.z = scalefactor.z * 2.0f;
	ratio.w = (float) volume->localdims.w/(float) volume->globaldims.w;
	ratio.h = (float) volume->localdims.h/(float) volume->globaldims.h;
	ratio.d = (float) volume->localdims.d/(float) volume->globaldims.d;
	brick->w = 2.0f * scalefactor.x * ratio.w;
	brick->h = 2.0f * scalefactor.y * ratio.h;
	brick->d = 2.0f * scalefactor.z * ratio.d;

	ratio.x = (float) volume->startindex.x/(float) volume->globaldims.w;
	ratio.y = (float) volume->startindex.y/(float) volume->globaldims.h;
	ratio.z = (float) volume->startindex.z/(float) volume->globaldims.d;
	brick->x = -scalefactor.x + range.x * ratio.x;
	brick->y = -scalefactor.y + range.y * ratio.y;
	brick->z = -scalefactor.z + range.z * ratio.z;

	// adjust for overlaps
	volume->localdims.w = 
	(offsets[rank].j == volume->procdims.x-1) ? volume->localdims.w : volume->localdims.w + overlap;

	volume->localdims.h = 
	(offsets[rank].i == volume->procdims.y-1) ? volume->localdims.h : volume->localdims.h + overlap;

	volume->localdims.d = 
	(offsets[rank].k == volume->procdims.z-1) ? volume->localdims.d : volume->localdims.d + overlap;

}

#ifdef IVR_DEBUG
void PRINTDEBUG(char *fmt, ...)

{
   va_list arg_ptr;

   va_start(arg_ptr, fmt);
   vfprintf(stderr, fmt, arg_ptr);
   va_end(arg_ptr);
}
#else
void PRINTDEBUG(char *fmt, ...) {};
#endif

#ifdef IVR_DEBUG2
void PRINTDEBUG2(char *fmt, ...)

{
   va_list arg_ptr;

   va_start(arg_ptr, fmt);
   vfprintf(stderr, fmt, arg_ptr);
   va_end(arg_ptr);
}
#else
void PRINTDEBUG2(char *fmt, ...) {};
#endif

#ifdef IVR_DEBUGIMAGES
void PRINTDEBUGIMAGE(char *str, char *img, int size)
{
            FILE *fd=fopen(str, "w+b");
            fwrite(img, 1 ,size, fd);
            fclose(fd);
}
#else
void PRINTDEBUGIMAGE(char *str, char *img, int size) {};
#endif

#ifdef IVR_DEBUGTILES
void PRINTDEBUGTILES(int rcv, int rank, int elements, float *rcolorbuf)
{
   for (int i=0; i<rcv; i++) {
       char fn[32];
       int indx = i * elements;
       sprintf(fn, "tilerbuf%d-%d.raw", rank,i);
       FILE *fout = fopen(fn,"w+b");
       fwrite(&rcolorbuf[indx], elements, sizeof(float), fout);
       fclose(fout);
   }
}
#else
void PRINTDEBUGTILES(int rcv, int rank, int elements, float *rcolorbuf) {};
#endif

void DEBUGSUBVOLUMES(color4f *colorbuf, int rank)
{

    
#ifdef IVR_DEBUG // testing that each rank it creating the correct image for it's portion of volume
   	char output[16];
    FILE *fd;

    // print out raw 2D images per rank - if not using MPI you will always get image0.raw
    sprintf(output,"image%d.raw",rank);
    if ((fd = fopen(output, "w+b")) == NULL) {
        fprintf(stderr,"failed to open image.raw\n");
        return;
    }
    size_t num = fwrite((void *) colorbuf, sizeof(color4f), winwidth*winheight, fd);
    PRINTDEBUG("num bytes written=%ld\n", num);

    fclose(fd);
#endif
}

#if 0 // save for later
void createbboxes()
{
	maxbox = maxrow * maxcol * maxslab;

	bboxes = (AABB *) malloc(sizeof(AABB) * maxbox);
	int (*startindices)[3][2] = malloc(sizeof(*startindices) * maxbox);

	int cnt = 0;
	for (i=0;  i<maxslab; i++) {
		for (j=0; j<maxcol; j++) {
			for (k=0; k<maxrow; k++) {
				cnt = (((k * maxrow) + i) * maxcol) + j;
				slicingcalc(depth, 1, maxslab, 0, overlap, k, &slicesize, &sliceorigin);
				startindices[cnt][2][0] = sliceorigin;
				startindices[cnt][2][1] = slicesize;
				slicingcalc(width, 1, maxcol, 0, overlap, j, &slicesize, &sliceorigin);
				startindices[cnt][1][0] = sliceorigin;
				startindices[cnt][1][1] = slicesize;
				slicingcalc(height, 1, maxrow, 0, overlap, i, &slicesize, &sliceorigin);
				startindices[cnt][0][0] = sliceorigin;
				startindices[cnt][0][1] = slicesize;
			}
		}
	}

	// save local bbox size and start point
	volume.localdims.h = startindices[rank][0][1];	
	volume.localdims.w = startindices[rank][1][1];	
	volume.localdims.d = startindices[rank][2][1];	
	volume.startindex.y = startindices[rank][0][0];
	volume.startindex.x = startindices[rank][1][0];
	volume.startindex.z = startindices[rank][2][0];

	// compute all the bounding boxes in world coords for all the ranks

   	for (i=0; i<maxrow; i++) {
   	    for (j=0; j<maxcol; j++) {
   	        for (k=0; k<maxslab; k++) {
   	             cnt = (((k * maxrow) + i) * maxcol) + j;
   	             for (int b=0; b<8; b++) {
   	                 switch(b) {
   	                     case 0:
   	                         ijk.y = startindices[cnt][0][0];
   	                         ijk.x = startindices[cnt][1][0];
   	                         ijk.z = startindices[cnt][2][0];
   	                         break;
   	                     case 1:
   	                         ijk.y = startindices[cnt][0][0];
   	                         ijk.x = startindices[cnt][1][0] + startindices[cnt][1][1];
   	                         ijk.z = startindices[cnt][2][0];
   	                         break;
   	                     case 2:
   	                         ijk.y = startindices[cnt][0][0] + startindices[cnt][0][1];
   	                         ijk.x = startindices[cnt][1][0] + startindices[cnt][1][1];
   	                         ijk.z = startindices[cnt][2][0];
   	                         break;
   	                     case 3:
   	                         ijk.y = startindices[cnt][0][0] + startindices[cnt][0][1];
   	                         ijk.x = startindices[cnt][1][0];
   	                         ijk.z = startindices[cnt][2][0];
   	                         break;
   	                    case 4:
   	                         ijk.y = startindices[cnt][0][0];
   	                         ijk.x = startindices[cnt][1][0];
   	                         ijk.z = startindices[cnt][2][0] + startindices[cnt][2][1];
   	                         break;
   	                     case 5:
   	                         ijk.y = startindices[cnt][0][0];
   	                         ijk.x = startindices[cnt][1][0] + startindices[cnt][1][1];
   	                         ijk.z = startindices[cnt][2][0] + startindices[cnt][2][1];
   	                         break;
   	                     case 6:
   	                         ijk.y = startindices[cnt][0][0] + startindices[cnt][0][1];
   	                         ijk.x = startindices[cnt][1][0] + startindices[cnt][1][1];
   	                         ijk.z = startindices[cnt][2][0] + startindices[cnt][2][1];
   	                         break;
   	                     case 7:
   	                         ijk.y = startindices[cnt][0][0] + startindices[cnt][0][1];
   	                         ijk.x = startindices[cnt][1][0];
   	                         ijk.z = startindices[cnt][2][0] + startindices[cnt][2][1];
   	                         break;
   	             }
                 xyz.x = (ijk.x - width/2.0f) * 2.0/maxdim;
                 xyz.y = (ijk.y - height/2.0f) * 2.0/maxdim;
                 xyz.z = (ijk.z - depth/2.0f) * 2.0/maxdim;
                 bboxes[cnt].verts[b].x = xyz.x;
                 bboxes[cnt].verts[b].y = xyz.y;
                 bboxes[cnt].verts[b].z = xyz.z;
                 bboxes[cnt].verts[b].w = 1.0f;
				 bboxes[cnt].rank = cnt;
			}
          }
       }
    }

	// compute all the bbox min, max, mvmin, mvmax for all the processes-> We may need later and we can avoid any interproc 
	// communications if we just do it during init
	for (i = 0; i<maxbox; i++) {
		AABB *box = (AABB *)&bboxes[i];
		minx = miny = minz = MAXFLOAT;
		maxx = maxy = maxz = -MAXFLOAT;
		for (int v=0; v<8; v++)  minx  = fmin(minx, box->verts[v].x);
		box->min.x = box->mvmin.x = minx;
		for (int v=0; v<8; v++)  miny  = fmin(miny, box->verts[v].y);
		box->min.y = box->mvmin.y = miny;
		for (int v=0; v<8; v++)  minz  = fmin(minz, box->verts[v].z);
		box->min.z = box->mvmin.z = minz;

		for (int v=0; v<8; v++)  maxx  = fmax(maxx, box->verts[v].x);
		box->max.x = box->mvmax.x = maxx;
		for (int v=0; v<8; v++)  maxy  = fmax(maxy, box->verts[v].y);
		box->max.y = box->mvmax.y = maxy;
		for (int v=0; v<8; v++)  maxz  = fmax(maxz, box->verts[v].z);
		box->max.z = box->mvmax.z = maxz;

		box->w = box->max.x - box->min.x; box->h = box->max.y - box->min.y; box->d = box->max.z - box->min.z;

		// set facenormals
		box->facenormals[0].x = -1.0f; box->facenormals[1].x = 1.0f;
		box->facenormals[2].y = -1.0f; box->facenormals[3].y = 1.0f;
		box->facenormals[4].z = -1.0f; box->facenormals[5].z = 1.0f;
    
	}
	return;
}
#endif
