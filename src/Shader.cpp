/*
 Copyright (c) 2010, The Barbarian Group
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#include "Shader.h"
#include <map>
#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace glm;

namespace gl {

//////////////////////////////////////////////////////////////////////////
// Shader::Obj
struct Shader::Obj 
{
    Obj() : mHandle( 0 ) {}
    ~Obj();
    
    GLuint						mHandle;
    std::map<std::string,int>	mUniformLocs;

};
    
Shader::Obj::~Obj()
{
	if( mHandle )
		glDeleteProgram( (GLuint)mHandle );
}

//////////////////////////////////////////////////////////////////////////
// Shader
Shader::Shader( const char *vertexShader, const char *fragmentShader, const char *geometryShader, GLint geometryInputType, GLint geometryOutputType, GLint geometryOutputVertices)
	: m_Obj( shared_ptr<Obj>( new Obj ) )
{
	m_Obj->mHandle = glCreateProgram();
	
	if ( vertexShader )
		loadShader( vertexShader, GL_VERTEX_SHADER_ARB );
    
	if( fragmentShader )
		loadShader( fragmentShader, GL_FRAGMENT_SHADER_ARB );
    
	if( geometryShader ) {
		loadShader( geometryShader, GL_GEOMETRY_SHADER_EXT );
        
        glProgramParameteriEXT(m_Obj->mHandle, GL_GEOMETRY_INPUT_TYPE_EXT, geometryInputType);
        glProgramParameteriEXT(m_Obj->mHandle, GL_GEOMETRY_OUTPUT_TYPE_EXT, geometryOutputType);
        glProgramParameteriEXT(m_Obj->mHandle, GL_GEOMETRY_VERTICES_OUT_EXT, geometryOutputVertices);
    }
    
	link();
}

void Shader::loadShader( const char *shaderSource, GLint shaderType )
{
	GLuint handle = glCreateShader( shaderType );
	glShaderSource( handle, 1, reinterpret_cast<const GLchar**>( &shaderSource ), NULL );
	glCompileShader( handle );
	
	GLint status;
	glGetShaderiv( (GLuint) handle, GL_COMPILE_STATUS, &status );
	if( status != GL_TRUE ) {
		std::string log = getShaderLog( (GLuint)handle );
		throw ShaderCompileExc( log, shaderType );
	}
	glAttachShader( m_Obj->mHandle, handle );
}

void Shader::link()
{
	glLinkProgram( m_Obj->mHandle );	
}

void Shader::bind() const
{
	glUseProgram( m_Obj->mHandle );
}

void Shader::unbind()
{
	glUseProgram( 0 );
}

GLuint Shader::getHandle() const
{
    return m_Obj->mHandle;
}
    
std::string Shader::getShaderLog( GLuint handle ) const
{
	std::string log;
	
	GLchar *debugLog;
	GLint debugLength = 0, charsWritten = 0;
	glGetShaderiv( handle, GL_INFO_LOG_LENGTH, &debugLength );

	if( debugLength > 0 ) {
		debugLog = new GLchar[debugLength];
		glGetShaderInfoLog( handle, debugLength, &charsWritten, debugLog );
		log.append( debugLog, 0, debugLength );
		delete [] debugLog;
	}
	
	return log;
}

void Shader::uniform( const std::string &name, int data )
{
	GLint loc = getUniformLocation( name );
	glUniform1i( loc, data );
}

void Shader::uniform( const std::string &name, const vec2 &data )
{
	GLint loc = getUniformLocation( name );
	glUniform2i( loc, data.x, data.y );
}

void Shader::uniform( const std::string &name, const int *data, int count )
{
	GLint loc = getUniformLocation( name );
	glUniform1iv( loc, count, data );
}

void Shader::uniform( const std::string &name, const ivec2 *data, int count )
{
	GLint loc = getUniformLocation( name );
	glUniform2iv( loc, count, &data[0].x );
}

void Shader::uniform( const std::string &name, float data )
{
	GLint loc = getUniformLocation( name );
	glUniform1f( loc, data );
}

void Shader::uniform( const std::string &name, const vec3 &data )
{
	GLint loc = getUniformLocation( name );
	glUniform3f( loc, data.x, data.y, data.z );
}

void Shader::uniform( const std::string &name, const vec4 &data )
{
	GLint loc = getUniformLocation( name );
	glUniform4f( loc, data.x, data.y, data.z, data.w );
}

void Shader::uniform( const std::string &name, const float *data, int count )
{
	GLint loc = getUniformLocation( name );
	glUniform1fv( loc, count, data );
}

void Shader::uniform( const std::string &name, const vec2 *data, int count )
{
	GLint loc = getUniformLocation( name );
	glUniform2fv( loc, count, &data[0].x );
}

void Shader::uniform( const std::string &name, const vec3 *data, int count )
{
	GLint loc = getUniformLocation( name );
	glUniform3fv( loc, count, &data[0].x );
}

void Shader::uniform( const std::string &name, const vec4 *theArray, int count )
{
	GLint loc = getUniformLocation( name );
	glUniform4fv( loc, count, &theArray[0].x );
}

void Shader::uniform( const std::string &name, const mat3 &theMat, bool transpose )
{
	GLint loc = getUniformLocation( name );
	glUniformMatrix3fv( loc, 1, ( transpose ) ? GL_TRUE : GL_FALSE, glm::value_ptr(theMat) );
}

void Shader::uniform( const std::string &name, const mat4 &theMat, bool transpose )
{
	GLint loc = getUniformLocation( name );
	glUniformMatrix4fv( loc, 1, ( transpose ) ? GL_TRUE : GL_FALSE, glm::value_ptr(theMat) );
}

GLint Shader::getUniformLocation( const std::string &name )
{
	map<string,int>::const_iterator uniformIt = m_Obj->mUniformLocs.find( name );
	if( uniformIt == m_Obj->mUniformLocs.end() ) {
		GLint loc = glGetUniformLocation( m_Obj->mHandle, name.c_str() );
		m_Obj->mUniformLocs[name] = loc;
		return loc;
	}
	else
		return uniformIt->second;
}

GLint Shader::getAttribLocation( const std::string &name )
{
	return glGetAttribLocation( m_Obj->mHandle, name.c_str() );
}

//////////////////////////////////////////////////////////////////////////
// ShaderCompileExc
ShaderCompileExc::ShaderCompileExc( const std::string &log, GLint aShaderType ) throw()
	: mShaderType( aShaderType )
{
	if( mShaderType == GL_VERTEX_SHADER_ARB )
		strncpy( mMessage, "VERTEX: ", 1000 );
	else if( mShaderType == GL_FRAGMENT_SHADER_ARB )
		strncpy( mMessage, "FRAGMENT: ", 1000 );
	else if( mShaderType == GL_GEOMETRY_SHADER_EXT )
		strncpy( mMessage, "GEOMETRY: ", 1000 );
	else
		strncpy( mMessage, "UNKNOWN: ", 1000 );
	strncat( mMessage, log.c_str(), 15000 );
}
	
}