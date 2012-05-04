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

    //gl_FragColor
    gl_FragData[0] = tex * vec4(v_texCoord.x, v_texCoord.y, -v_texCoord.x, 1.0);
}
