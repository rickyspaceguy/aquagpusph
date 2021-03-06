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
 * @brief Performance report.
 * (See Aqua::CalcServer::Reports::Performance for details)
 */

#ifndef PERFORMANCE_H_INCLUDED
#define PERFORMANCE_H_INCLUDED

#include <sys/time.h>
#include <sphPrerequisites.h>
#include <CalcServer/Reports/Report.h>
#include <AuxiliarMethods.h>

namespace Aqua{ namespace CalcServer{ namespace Reports{

/** @class Performance Performance.h CalcServer/Performance.h
 * @brief On screen performance output.
 *
 * Performance will print the following data:
 *    -# Allocated memory in the computational device
 *    -# The average CPU time consumend of each tool (GPU time can be taken
 *    with the profiling tools of each vendor)
 *
 * @see Aqua::InputOutput::ScreenManager
 */
class Performance : public Aqua::CalcServer::Reports::Report
{
public:
    /** @brief Constructor.
     * @param tool_name Tool name.
     * @param color Color to print the report. Valid colors are:
     *    - "white"
     *    - "green"
     *    - "blue"
     *    - "yellow"
     *    - "red"
     *    - "magenta"
     *    - "cyan"
     * @param bold false if the report should not be highlighted with bolded
     * font, true otherwise.
     * @param output_file Path of the output file.
     */
    Performance(const char* tool_name,
                const char* color="white",
                bool bold=false,
                const char* output_file="");

    /** @brief Destructor
     */
    ~Performance();

    /** @brief Initialize the tool.
     * @return false if all gone right, true otherwise.
     */
    bool setup();

    /** @brief Launch _execute() without measuring the elapsed time
     *
     * This tool is measuring the total time elapsed during a single time step
     * computation, and comparing it with the time consumed by the other tools.
     * Hence we should override the overloaded time measuring capabilities
     * @return false if all gone right, true otherwise.
     * @see Aqua::CalcServer::Tool
     */
    bool execute(){return _execute();}

protected:
    /** @brief Get the allocated memory.
     * @return Allocated memory in the computational device.
     */
    size_t computeAllocatedMemory();

    /** @brief Execute the tool.
     * @return false if all gone right, true otherwise.
     */
    bool _execute();

private:
    /// Output color
    char *_color;
    /// Output bold or normal flag
    bool _bold;
    /// Output file name
    char *_output_file;
    /// Output file handler
    FILE *_f;
    /** @brief Time mark at the end of the previous time step (to measure the
     * total elapsed time per time step)
     */
    timeval _tic;
    /** @brief First execution flag
     *
     * The first time step is including all the time required to build the tool,
     * which may invalidate the average and variance values. In that case is
     * therefore better using the elapsed time by the tools
     */
    bool _first_execution;
};

}}} // namespace

#endif // PERFORMANCE_H_INCLUDED
