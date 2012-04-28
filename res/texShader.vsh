//
//  Shader.vsh
//  Bollombub
//
//  Created by Fabian Schmidt on 4/24/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

//#version 150 core

uniform mat4 u_modelViewProjectionMatrix;
uniform mat4 u_textureMatrix;
//uniform mat3 u_normalMatrix;

attribute vec4 a_position;
attribute vec3 a_normal;
attribute vec4 a_texCoord;

varying vec4 v_texCoord;

void main()
{
    //vec3 eyeNormal = normalize(u_normalMatrix * a_normal);
    //float nDotVP = max(0.0, dot(eyeNormal, normalize(lightPosition)));
                 
    v_texCoord = a_texCoord * u_textureMatrix;
    v_texCoord += u_textureMatrix[3];
    
    gl_Position = u_modelViewProjectionMatrix * a_position;
}
