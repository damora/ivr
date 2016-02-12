
#define II       1
#define MM       2  
#define BYTE     1 
#define ASCII    2
#define SHORT    3
#define LONG     4
#define RATIONAL 5
#define TIFFHEADER 512
    
//  Header structure: 

typedef struct {
  short int  byte_order;
  short int  version;
  long int   ifd_offset;
} TIFF_HEADER;

// Pointer structure: 

typedef struct {
  short int  tag;
  short int  type;
  long int   length;
  long int   voff;
} TIFF_POINTER;

//  File information structure: 

typedef struct {
  long int   iimm;
  long int   subfile_flags;
  long int   photo_interp;
  long int   planar_config;
  short int  bits_sample[3];
  short int  sample_pixel;
  long int   rows_strip;
  long int   strip_cnt_length;
  long int   strip_cnt_offset;
  long int   strip_off_length;
  long int   strip_off_offset;
} TIFF_DATA;

//  Function prototypes.


unsigned char *make_tag ( short int tag, short int type, long int lng, 
                 long int fld, unsigned char *b, char *cnt );

void           writetotiff (char *outimg, long int xs, long int ys, long int pl, float *inpimage );
void           writetiffdata (char *outimg, long int xs, long int ys, long int pl, float *inpimage );
void           writetiffheader (char *outimg, long int xs, long int ys, long int pl );

