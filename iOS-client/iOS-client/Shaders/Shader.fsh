//
//  Shader.fsh
//  iOS-client
//
//  Created by Bruce D'Amora on 6/18/14.
//  Copyright (c) 2014 Bruce D'Amora. All rights reserved.
//

varying lowp vec4 colorVarying;

void main()
{
    gl_FragColor = colorVarying;
}
