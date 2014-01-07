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

#include <TimeManager.h>

// ----------------------------------------------------------------------------
// Include the Problem setup manager header
// ----------------------------------------------------------------------------
#include <ProblemSetup.h>

// ----------------------------------------------------------------------------
// Include the Problem setup manager header
// ----------------------------------------------------------------------------
#include <ScreenManager.h>

// ----------------------------------------------------------------------------
// Include the main header
// ----------------------------------------------------------------------------
#include <CalcServer/Energy.h>

// ----------------------------------------------------------------------------
// Include the calculation server
// ----------------------------------------------------------------------------
#include <CalcServer.h>

namespace Aqua{ namespace CalcServer{

Energy::Energy()
	: Kernel("Energy")
	, mDevEnergy(NULL)
	, mTime(0.f)
	, _path(NULL)
	, _program(NULL)
	, _kernel(NULL)
	, _global_work_size(0)
	, _local_work_size(0)
	, mReduction(NULL)
{
	InputOutput::ScreenManager *S = InputOutput::ScreenManager::singleton();
	InputOutput::ProblemSetup *P  = InputOutput::ProblemSetup::singleton();
	unsigned int str_len = strlen(P->OpenCL_kernels.energy);
	if(str_len <= 0) {
		S->addMessage(3, "(Energy::Energy): Path of the kernels (2D) is empty.\n");
		exit(EXIT_FAILURE);
	}
	_path = new char[str_len+4];
	if(!_path) {
		S->addMessage(3, "(Energy::Energy): Memory cannot be allocated for the path.\n");
		exit(EXIT_FAILURE);
	}
	strcpy(_path, P->OpenCL_kernels.energy);
	strcat(_path, ".cl");

	_local_work_size  = localWorkSize();
	if(!_local_work_size){
	    S->addMessage(3, "(Energy::Energy): I cannot get a valid local work size for the required computation tool.\n");
	    exit(EXIT_FAILURE);
	}
	_global_work_size = globalWorkSize(_local_work_size);
	if(setupEnergy()) {
		exit(EXIT_FAILURE);
	}
	if(setupReduction()) {
		exit(EXIT_FAILURE);
	}
	mEnergy.x = 0.f;
	mEnergy.y = 0.f;
	mEnergy.z = 0.f;
	mEnergy.w = 0.f;
	S->addMessage(1, "(Energy::Energy): Energy ready to work!\n");
}

Energy::~Energy()
{
	InputOutput::ScreenManager *S = InputOutput::ScreenManager::singleton();
	S->addMessage(1, "(Energy::~Energy): Destroying reduction processor...\n");
	if(mReduction) delete mReduction;
	if(mDevEnergy)clReleaseMemObject(mDevEnergy); mDevEnergy=0;
	if(_kernel)clReleaseKernel(_kernel); _kernel=0;
	if(_program)clReleaseProgram(_program); _program=0;
	if(_path)delete[] _path; _path=0;
}

bool Energy::execute()
{
	InputOutput::ScreenManager *S = InputOutput::ScreenManager::singleton();
	InputOutput::TimeManager *T   = InputOutput::TimeManager::singleton();
	CalcServer *C = CalcServer::singleton();
	int err_code=0;
	err_code |= sendArgument(_kernel, 13, sizeof(cl_float), (void*)&(C->cs));
	err_code |= sendArgument(_kernel, 14, sizeof(vec     ), (void*)&(C->g));
	if(err_code != CL_SUCCESS) {
		S->addMessage(3, "(Energy::Execute): Fail senting the _kernel parameters.\n");
		S->printOpenCLError(err_code);
	}
	#ifdef HAVE_GPUPROFILE
		cl_event event;
		cl_ulong end, start;
		profileTime(0.f);
		err_code = clEnqueueNDRangeKernel(C->command_queue, _kernel, 1, NULL, &_global_work_size, NULL, 0, NULL, &event);
	#else
		err_code = clEnqueueNDRangeKernel(C->command_queue, _kernel, 1, NULL, &_global_work_size, NULL, 0, NULL, NULL);
	#endif
	if(err_code != CL_SUCCESS) {
		S->addMessage(3, "(Energy::Execute): I cannot execute the energy calculation kernel.\n");
		if(err_code == CL_INVALID_WORK_GROUP_SIZE) {
			S->addMessage(0, "\tInvalid local work group size.\n");
		}
		else if(err_code == CL_OUT_OF_RESOURCES) {
			S->addMessage(0, "\tDevice out of resources.\n");
		}
		else if(err_code == CL_MEM_OBJECT_ALLOCATION_FAILURE) {
			S->addMessage(0, "\tAllocation error at device.\n");
		}
		else if(err_code == CL_OUT_OF_HOST_MEMORY) {
			S->addMessage(0, "\tFailure allocating resources required by the OpenCL implementation on the host.\n");
		}
		return true;
	}
	#ifdef HAVE_GPUPROFILE
		err_code = clWaitForEvents(1, &event);
		if(err_code != CL_SUCCESS) {
			S->addMessage(3, "(Energy::Execute): Impossible to wait for the kernels end.\n");
			return true;
		}
		err_code |= clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, 0);
		err_code |= clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, 0);
		if(err_code != CL_SUCCESS) {
			S->addMessage(3, "(Energy::Execute): I cannot profile the kernel execution.\n");
			return true;
		}
		profileTime(profileTime() + (end - start)/1000.f);  // 10^-3 ms
	#endif
    cl_mem output;
    output = mReduction->execute();
    if(!output)
        return true;
    // Get the computed compnents
	if(C->getData((void *)&mDEnergyDT, output, sizeof(vec4)))
		return true;
    // Integrate the elastic energy and add it to the total one
    float dt = T->time() - mTime;
    mTime = T->time();
    mEnergy.x += mDEnergyDT.x * dt;
    mEnergy.y += mDEnergyDT.y * dt;
    mEnergy.z += mDEnergyDT.z;
    mEnergy.w += mDEnergyDT.w;
	return false;
}

bool Energy::setupEnergy()
{
	InputOutput::ScreenManager *S = InputOutput::ScreenManager::singleton();
	CalcServer *C = CalcServer::singleton();
	cl_int err_code = 0;
	char msg[1024];
	if(!loadKernelFromFile(&_kernel, &_program, C->context, C->device, _path, "Energy", "-Dvec4=float4"))
		return true;
	mDevEnergy = C->allocMemory(C->n * sizeof( vec4 ));
	if(!mDevEnergy)
		return true;
	sprintf(msg, "\tAllocated memory = %u bytes\n", (unsigned int)C->allocated_mem);
	S->addMessage(0, msg);
	err_code  = sendArgument(_kernel,  0, sizeof(cl_mem  ), (void*)&mDevEnergy);
	err_code |= sendArgument(_kernel,  1, sizeof(cl_mem  ), (void*)&(C->imove));
	err_code |= sendArgument(_kernel,  2, sizeof(cl_mem  ), (void*)&(C->ifluid));
	err_code |= sendArgument(_kernel,  3, sizeof(cl_mem  ), (void*)&(C->pos));
	err_code |= sendArgument(_kernel,  4, sizeof(cl_mem  ), (void*)&(C->v));
	err_code |= sendArgument(_kernel,  5, sizeof(cl_mem  ), (void*)&(C->mass));
	err_code |= sendArgument(_kernel,  6, sizeof(cl_mem  ), (void*)&(C->dens));
	err_code |= sendArgument(_kernel,  7, sizeof(cl_mem  ), (void*)&(C->press));
	err_code |= sendArgument(_kernel,  8, sizeof(cl_mem  ), (void*)&(C->drdtin));
	err_code |= sendArgument(_kernel,  9, sizeof(cl_mem  ), (void*)&(C->drdt_F));
	err_code |= sendArgument(_kernel, 10, sizeof(cl_mem  ), (void*)&(C->fin));
	err_code |= sendArgument(_kernel, 11, sizeof(cl_mem  ), (void*)&(C->refd));
	err_code |= sendArgument(_kernel, 12, sizeof(cl_mem  ), (void*)&(C->gamma));
	err_code |= sendArgument(_kernel, 13, sizeof(cl_float), (void*)&(C->cs));
	err_code |= sendArgument(_kernel, 14, sizeof(vec     ), (void*)&(C->g));
	err_code |= sendArgument(_kernel, 15, sizeof(cl_uint ), (void*)&(C->n));
	if(err_code)
		return true;
	//! Test for right work group size
	cl_device_id device;
	size_t local_work_size=0;
	err_code |= clGetCommandQueueInfo(C->command_queue,CL_QUEUE_DEVICE,
	                                sizeof(cl_device_id),&device, NULL);
	if(err_code != CL_SUCCESS) {
		S->addMessage(3, "(Energy::setupEnergy): I Cannot get the device from the command queue.\n");
	    return true;
	}
	err_code |= clGetKernelWorkGroupInfo(_kernel,device,CL_KERNEL_WORK_GROUP_SIZE,
	                                   sizeof(size_t), &local_work_size, NULL);
	if(err_code != CL_SUCCESS) {
		S->addMessage(3, "(Energy::setupEnergy): Failure retrieving the maximum local work size.\n");
	    return true;
	}
	if(local_work_size < _local_work_size)
	    _local_work_size  = local_work_size;
	_global_work_size = globalWorkSize(_local_work_size);
	return false;
}

bool Energy::setupReduction()
{
	CalcServer *C = CalcServer::singleton();
    mReduction = new Reduction(mDevEnergy, C->n, "float4", "(float4)(0.f,0.f,0.f,0.f)", "c = a + b;");
	return false;
}

}}  // namespace
