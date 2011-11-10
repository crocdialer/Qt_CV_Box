/*
 * Singleton.h
 *
 *  Created on: Aug 20, 2008
 *      Author: Fabian
 *
 *	The code in this file is heavily inspired by article 1.3 in the the book:
 *	Game Programming Gems from Charles River Media
 *
 */

#ifndef _SINGLETON_H
#define _SINGLETON_H

#include <cassert>


template <typename T> class Singleton
{

protected:

	static T* ms_Singleton;

public:
    Singleton( void )
	{
        assert( !ms_Singleton );
        ms_Singleton = static_cast<T*>(this);
    }
   ~Singleton( void )
	{
	   assert( ms_Singleton );
	   ms_Singleton = 0;
	};

	static void unload()
	{
		if(ms_Singleton)
			delete ms_Singleton;
	};

    static T& getSingleton( void )
	{

    	if(!ms_Singleton)
    		new T();

    	return ( *ms_Singleton );
    };

    static T* getSingletonPtr( void )
	{

    	if(!ms_Singleton)
    		new T();

    	return ( ms_Singleton );
    };
};


#endif
