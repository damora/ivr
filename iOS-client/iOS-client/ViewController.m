//
//  ViewController.m
//  iOS-client
//
//  Created by Bruce D'Amora on 6/18/14.
//  Copyright (c) 2014 Bruce D'Amora. All rights reserved.
//



#import "ViewController.h"
#import "mathutil.h"
#import "types.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

// Uniform index.
enum
{
    UNIFORM_MODELVIEWPROJECTION_MATRIX,
    NUM_UNIFORMS
};
GLint uniforms[NUM_UNIFORMS];

// Attribute index.
enum
{
    ATTRIB_VERTEX,
    NUM_ATTRIBUTES
};

GLfloat gCubeVertexData[90] =
{
    // Data layout for each line below is:
    // positionX, positionY, positionZ,     normalX, normalY, normalZ,
    // front
    -1.0f, -1.0f, 1.0f,
     1.0f, -1.0f, 1.0f,
     1.0f,  1.0f, 1.0f,
    -1.0f,  1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,
    
    // back
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    
    //left
    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    //right
    1.0f, -1.0f,  1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f, -1.0f,  1.0f,
    
    
    //bottom
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     // top
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
 };
char * ipaddr = "localhost" ;

int width = 501;
int height = 201;
int depth =  276 ;

@interface ViewController () {
    GLuint _program;
    
    GLKMatrix4 _modelViewProjectionMatrix;
    float _rotation;
    
    GLuint _vertexArray;
    GLuint _vertexBuffer;
    IBOutlet UISegmentedControl *ToolbarButtons;
    
    }
@property (strong, nonatomic) EAGLContext *context;
@property (strong, nonatomic) GLKBaseEffect *effect;

- (void)setupGL;
- (void)tearDownGL;
@end

@implementation ViewController
@synthesize ToolbarButtons;

-(IBAction)indexChanged:(UISegmentedControl *)sender
{
    switch (self.ToolbarButtons.selectedSegmentIndex)
    {
        case 0:     // Rotate
            
            break;
        case 1:     // Pan
            
            break;
        case 2:     // Zoom
            
            break;
        case 3:     // Clip X
            
            break;
        case 4:     // Clip Y
            
            break;
        case 5:     // Clip Z
            
            break;
        case 6:     // Adjust Alpha
            
            break;
        case 7:     // Home state
            
            break;
            
        default:
            break;
    } 
}
- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    
    if (!self.context) {
        NSLog(@"Failed to create ES context");
    }
    
    GLKView *view = (GLKView *)self.view;
    view.context = self.context;
    view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    
    [self setupGL];
}

- (void)dealloc
{
    [self tearDownGL];
    
    if ([EAGLContext currentContext] == self.context) {
        [EAGLContext setCurrentContext:nil];
    }
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    
    if ([self isViewLoaded] && ([[self view] window] == nil)) {
        self.view = nil;
        
        [self tearDownGL];
        
        if ([EAGLContext currentContext] == self.context) {
            [EAGLContext setCurrentContext:nil];
        }
        self.context = nil;
    }
    
}

- (void)setupGL
{
    float maxdim;
    vector3f scalefactor;
    
    [EAGLContext setCurrentContext:self.context];
    
    self.effect = [[GLKBaseEffect alloc] init];
    self.effect.light0.enabled = GL_TRUE;
    self.effect.light0.diffuseColor = GLKVector4Make(1.0f, 0.4f, 0.4f, 1.0f);

    // adjust the aspect ratio of bounding cube based on volume dimensions
    maxdim =  (float) (imax3(width, height, depth));
    
    // compute scale factor for global bounding box. Will use to draw a global bounding box
    float invdim  = 1.0f/maxdim;
    
    scalefactor.x = (float)width * invdim;
    scalefactor.y = (float)height * invdim;
    scalefactor.z = (float)depth * invdim;
    
    // initialize vertices for bounding volume box
    for (int i=0; i<90; i+=3) {
        gCubeVertexData[i]   *= scalefactor.x;
        gCubeVertexData[i+1] *= scalefactor.y;
        gCubeVertexData[i+2] *= scalefactor.z;
    }
 
    glEnable(GL_DEPTH_TEST);
    
    glGenVertexArraysOES(1, &_vertexArray);
    glBindVertexArrayOES(_vertexArray);
    
    glGenBuffers(1, &_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gCubeVertexData), gCubeVertexData, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(GLKVertexAttribPosition);
    glVertexAttribPointer(GLKVertexAttribPosition, 3, GL_FLOAT, GL_FALSE, 12, BUFFER_OFFSET(0));
    
//    glBindVertexArrayOES(0);
}

- (void)tearDownGL
{
    [EAGLContext setCurrentContext:self.context];
    
    glDeleteBuffers(1, &_vertexBuffer);
    glDeleteVertexArraysOES(1, &_vertexArray);
    
    self.effect = nil;

}

#pragma mark - GLKView and GLKViewController delegate methods

- (void)update
{
    
    //float aspect = fabs(self.view.window.bounds.size.width/self.view.window.bounds.size.height);;
    
    float aspect = fabsf(self.view.bounds.size.width / self.view.bounds.size.height);
    GLKMatrix4 projectionMatrix = GLKMatrix4MakePerspective(GLKMathDegreesToRadians(43.6028), aspect, 0.1f, 100.0f);
    
    self.effect.transform.projectionMatrix = projectionMatrix;
    
    GLKMatrix4 baseModelViewMatrix = GLKMatrix4MakeTranslation(0.0f, 0.0f, 0.0f);
    baseModelViewMatrix = GLKMatrix4Rotate(baseModelViewMatrix, _rotation, 0.0f, 0.0f, 1.0f);
    
    // Compute the model view matrix for the object rendered with GLKit

    GLKMatrix4 modelViewMatrix;
    modelViewMatrix = GLKMatrix4Rotate(modelViewMatrix, _rotation, 0.0f, 0.0f, 1.0f);
    modelViewMatrix = GLKMatrix4MakeTranslation(0.0f, 0.0f, -5.0f);
    modelViewMatrix = GLKMatrix4Multiply(baseModelViewMatrix, modelViewMatrix);
    
    self.effect.transform.modelviewMatrix = modelViewMatrix;
    
    
    _modelViewProjectionMatrix = GLKMatrix4Multiply(projectionMatrix, modelViewMatrix);
    
    _rotation += self.timeSinceLastUpdate * 0.5f;
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
    glClearColor(0.5f, 0.5f, 0.5, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glBindVertexArrayOES(_vertexArray);
    
    // Render the object with GLKit
    [self.effect prepareToDraw];
    
    glDrawArrays(GL_LINE_LOOP, 0, 30);
    
}


- (void) touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    UITouch *theTouch = [touches anyObject];
    CGPoint _startPoint = [theTouch locationInView:self.view];
    CGFloat x = _startPoint.x;
    CGFloat y = _startPoint.y;
    fprintf(stderr,"x,y=%f, %f\n", x,y);
    
}

- (void) touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    UITouch *theTouch = [touches anyObject];
    CGPoint touchLocation =
    [theTouch locationInView:self.view];
    CGFloat x = touchLocation.x;
    CGFloat y = touchLocation.y;
    fprintf(stderr,"x,y=%f, %f\n", x,y);
}

- (void) touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    UITouch *theTouch = [touches anyObject];
    CGPoint _endPoint = [theTouch locationInView:self.view];
    CGFloat x = _endPoint.x;
    CGFloat y = _endPoint.y;
    fprintf(stderr,"x,y=%f, %f\n", x,y);
}

@end

    
