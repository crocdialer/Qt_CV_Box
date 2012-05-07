//
//  Shader.fsh
//  Bollombub
//
//  Created by Fabian Schmidt on 4/24/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#version 150 core

uniform sampler2D   u_textureMap;
in vec4        v_texCoord;

out vec4 fragData;

void main()
{
    vec4 tex = texture(u_textureMap, v_texCoord.xy);

    //gl_FragColor, gl_FragData[0]
    fragData = tex * vec4(v_texCoord.x, v_texCoord.y, -v_texCoord.x, 1.0);
}
