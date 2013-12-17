/*
 *  This file is part of AQUAgpusph, a free CFD program based on SPH.
 *  Copyright (C) 2012  Jose Luis Cercos Pita <jl.cercos@upm.es>
 *
 *  AQUAgpusph is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  AQUAgpusph is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with AQUAgpusph.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SPHPREREQUISITES_H_INCLUDED
#define SPHPREREQUISITES_H_INCLUDED

#define XSTR(x) STR(x)
#define STR(x) #x

#include<string>

// ----------------------------------------------------------------------------
// Autotools configuration file
// ----------------------------------------------------------------------------
#include <config.h>

// ----------------------------------------------------------------------------
// OpenCL stuff
// ----------------------------------------------------------------------------
#include <CL/cl.h>

#ifndef vec2
    #define vec2 cl_float2
#endif
#ifndef vec3
    #define vec3 cl_float3
#endif
#ifndef vec4
    #define vec4 cl_float4
#endif

#ifndef ivec2
    #define ivec2 cl_int2
#endif
#ifndef ivec3
    #define ivec3 cl_int3
#endif
#ifndef ivec4
    #define ivec4 cl_int4
#endif

#ifndef uivec2
    #define uivec2 cl_uint2
#endif
#ifndef uivec3
    #define uivec3 cl_uint3
#endif
#ifndef uivec4
    #define uivec4 cl_uint4
#endif

#ifdef HAVE_3D
	#ifndef vec
	    #define vec vec4
	#endif
	#ifndef ivec
	    #define ivec ivec4
	#endif
	#ifndef uivec
	    #define uivec uivec4
	#endif
#else
	#ifndef vec
	    #define vec vec2
	#endif
	#ifndef ivec
	    #define ivec ivec2
	#endif
	#ifndef uivec
	    #define uivec uivec2
	#endif
#endif

#ifndef __CL_MIN_LOCALSIZE__
	#define __CL_MIN_LOCALSIZE__ 64
#endif
#ifndef __CL_MAX_LOCALSIZE__
	#define __CL_MAX_LOCALSIZE__ 1024
#endif

inline const char* methodAndClassName(const std::string& pretty_function)
{
    std::string all_name, method_name, class_name;
    size_t begin,end;

    // Filter the name removing the preceding and trailing types data
    end    = pretty_function.find("(");
    begin  = pretty_function.substr(0,end).rfind(" ") + 1;
    end   -= begin;
    all_name = pretty_function.substr(begin,end);

    // Get the method name
    begin = all_name.rfind("::");
    if (begin == std::string::npos){
        // There are no class name
        return all_name.c_str();
    }
    method_name = all_name.substr(begin+2, std::string::npos);
    end = begin;
    begin = all_name.substr(0,end).rfind("::");
    if(begin == std::string::npos)
        begin = 0;
    else
        begin += 2;
    end -= begin;
    class_name = all_name.substr(begin, end);
    return (class_name + "::" + method_name).c_str();
}

#define __METHOD_CLASS_NAME__ methodAndClassName(__PRETTY_FUNCTION__)

/** \class mat sphPrerequisites.h sphPrerequisites.h
 * Dimensions depending matrix.
 *   - 4x4 in 3D.
 *   - 2x2 in 2D.
 * @warning This matrix class have not errors control, so
 * a bad item request may result in a segmentation fault.
 */
class mat
{
public:
	/** Constructor. The matrix will be initialized as
	 * a zeroes matrix.
	 */
	mat(){
	    #ifdef HAVE_3D
	        row[0].x=0.f; row[0].y=0.f; row[0].z=0.f; row[0].w=0.f;
	        row[1].x=0.f; row[1].y=0.f; row[1].z=0.f; row[1].w=0.f;
	        row[2].x=0.f; row[2].y=0.f; row[2].z=0.f; row[2].w=0.f;
	        row[3].x=0.f; row[3].y=0.f; row[3].z=0.f; row[3].w=0.f;
	    #else
	        row[0].x=0.f; row[0].y=0.f;
	        row[1].x=0.f; row[1].y=0.f;
	    #endif
	}

	/** Subscript operator to return a matrix row.
	 * @param index Row index.
	 * @return Row vector.
	 */
	vec const& operator[] (unsigned index) const{
	    return row[index];
	}
	/** Subscript operator to return a matrix row.
	 * @param index Row index.
	 * @return Row vector.
	 */
	vec& operator[] (unsigned index){
	    return row[index];
	}

	/** Matrix-Vector multiply operator.
	 * @param V Vector to multiply.
	 * @return Resulting vector.
	 */
	vec operator* (const vec &V){
	    unsigned int i,j;
	    vec R;
	    #ifdef HAVE_3D
	        R.x = row[0].x*V.x + row[0].y*V.y + row[0].z*V.z + row[0].w*V.w;
	        R.y = row[1].x*V.x + row[1].y*V.y + row[1].z*V.z + row[1].w*V.w;
	        R.z = row[2].x*V.x + row[2].y*V.y + row[2].z*V.z + row[2].w*V.w;
	        R.w = row[3].x*V.x + row[3].y*V.y + row[3].z*V.z + row[3].w*V.w;
	    #else
	        R.x = row[0].x*V.x + row[0].y*V.y;
	        R.y = row[1].x*V.x + row[1].y*V.y;
	    #endif
	    return R;
	}

	/** Assigment operator.
	 * @param M Matrix to copy.
	 * @return Copied matrix.
	 */
	mat& operator= (const mat &M) {
	    if (this == &M) {   // Same object, simply return it
	        return *this;
	    }
	    #ifdef HAVE_3D
	        row[0].x=M[0].x; row[0].y=M[0].y; row[0].z=M[0].z; row[0].w=M[0].w;
	        row[1].x=M[1].x; row[1].y=M[1].y; row[1].z=M[1].z; row[1].w=M[1].w;
	        row[2].x=M[2].x; row[2].y=M[2].y; row[2].z=M[2].z; row[2].w=M[2].w;
	        row[3].x=M[3].x; row[3].y=M[3].y; row[3].z=M[3].z; row[3].w=M[3].w;
	    #else
	        row[0].x=M[0].x; row[0].y=M[0].y;
	        row[1].x=M[1].x; row[1].y=M[1].y;
	    #endif
	    return *this;
	}
private:
	/// Matrix rows
	#ifdef HAVE_3D
	    vec row[4];
	#else
	    vec row[2];
	#endif
};

#endif // SPHPREREQUISITES_H_INCLUDED