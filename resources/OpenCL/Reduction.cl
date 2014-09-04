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

/** @file
 * @brief Reductions, like scans, prefix sums, maximum or minimum, etc...
 * (See Aqua::CalcServer::Reduction for details)
 */

#ifndef HAVE_3D
    #include "types/2D.h"
#else
    #include "types/3D.h"
#endif

/** @brief Reduction step.
 *
 * Each step is aimed to compute just one reduced value from each work group.
 * Hence you can call this kernel recursively until only one work group needs
 * to be computed, resulting in just one reduced value.
 * @param input Input array to be reduced.
 * @param output Output array where reduced values should be stored.
 * @param N Number of input elements.
 * @param lmem local memory to store the output data during the work.
 * It is used to acclerate the process.
 */
__kernel void Reduction(__global T *input,
                        __global T *output,
                        unsigned int N,
                        __local T* lmem )
{
	unsigned int i;
	// Get the global index (to ensure not out of bounds reading operations)
	unsigned int gid = get_global_id(0);
	// Get id into the work group
	unsigned int tid = get_local_id(0);

	if(gid >= N)
		lmem[tid] = IDENTITY;	
	else
		lmem[tid] = input[gid];
	barrier(CLK_LOCAL_MEM_FENCE);

	// Start reducing the variables.
    // The point is working in the first half of the work group (that left to
    // reduce), reducing it with the other half of data.
	for(i=get_local_size(0)/2;i>0;i>>=1){
		// Ensure that we are not reading out of bounds
		if(tid < i)
			lmem[tid] = reduce(lmem[tid], lmem[tid+i]);
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	if(tid == 0){
		output[get_group_id(0)] = lmem[0];
	}
}
