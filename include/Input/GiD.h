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

#ifndef GID_H_INCLUDED
#define GID_H_INCLUDED

// ----------------------------------------------------------------------------
// Include Prerequisites
// ----------------------------------------------------------------------------
#include <sphPrerequisites.h>

// ----------------------------------------------------------------------------
// Include standar libraries
// ----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// ----------------------------------------------------------------------------
// Include the h5part library
// ----------------------------------------------------------------------------
#ifdef HAVE_H5PART
	#include <H5Part.h>
	#include <hdf5.h>
#endif // HAVE_H5PART

// ----------------------------------------------------------------------------
// Include the Host-Server fluid transfer layer
// ----------------------------------------------------------------------------
#include <Fluid.h>

// ----------------------------------------------------------------------------
// Include the equation reader
// ----------------------------------------------------------------------------
#include <Tokenizer/Tokenizer.h>

namespace Aqua{ namespace InputOutput{ namespace Input{

/** Reads GiD file to set the fluid.
 * @param path Input file path.
 * @param ifluid Index of the fluid.
 * @param i0 Starting index of the particle.
 * @param n number of particles to read.
 * @param refd Reference density.
 * @param h Reference kernel height.
 * @param F Fluid host instance.
 * @return false if all gone right. \n true otherwise.
 */
int loadGiD(const char* path, int ifluid, unsigned int i0, unsigned int n, float refd, float h, Fluid *F);

}}} //namespaces

#endif // GID_H_INCLUDED