//
//  Shader.fsh
//  Bollombub
//
//  Created by Fabian Schmidt on 4/24/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

//#version 150 core

uniform sampler2D   u_textureMap;
varying vec4        v_texCoord;

void main()
{
    vec4 tex = texture2D(u_textureMap, v_texCoord.xy);
    gl_FragColor = tex;//vec4(1.0, 0.0, 0.0, 0);
}
