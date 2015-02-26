// Implement your solutions in this file
#include "findmotifs.h"
#include "hamming.h"
#include <mpi.h>
#include <deque>
#include <stdlib.h>

const MPI_Comm COM = MPI_COMM_WORLD;
const int MASTER = 0;
const int TAG_PARAMETER = 0x01;
const int TAG_TASK = 0x02;
const int TAG_FINISHED = 0x03;
const int TAG_RESULT = 0x04;

bits_t InvertBit(bits_t element, size_t position)
{
    bits_t MASK = 0x01;
    return (element ^ (MASK << position));
}

void Test(bits_t element, const size_t n, const bits_t* input, const size_t d, std::vector<bits_t>& solutions)
{
    bool valid = true;
    for (size_t i = 0; i < n; ++i)
    {
        if (hamming(element, input[i]) > d)
            valid = false;
    }

    if (valid)
        solutions.push_back(element);
}

void EnumerateSlave(
        bits_t element,
        size_t inversions,
        size_t position,
        const size_t l,
        const size_t d,
        const size_t n,
        const bits_t* input,
        std::vector<bits_t>& solutions)
{
    // if we have reached our l-depth, check for a solution 
    if (position == l)
    {
        Test(element, n, input, d, solutions);
        return;
    }

    // enumerate candidate and inversion candidate
    EnumerateSlave(element, inversions, position + 1, l, d, n, input, solutions);
    if (inversions < d)
    {
        bits_t element_ = InvertBit(element, position);
        EnumerateSlave(element_, inversions + 1, position + 1, l, d, n, input, solutions);
    }
}

#include <iostream>
#include <bitset>
std::vector<bits_t> findmotifs_worker(const unsigned int n,
                       const unsigned int l,
                       const unsigned int d,
                       const bits_t* input,
                       const unsigned int startbitpos,
                       bits_t start_value)
{
    std::vector<bits_t> results;

    // figure out how many inversions have already been found, then continue enumerating
    size_t inversions = hamming(start_value, input[0]);
    EnumerateSlave(start_value, inversions, startbitpos, l, d, n, input, results);

    return results;
}

void worker_main()
{
    // 1.) receive input from master (including n, l, d, input, master-depth)
    unsigned int n, l, d, master_depth;
    std::vector<bits_t> input;

    MPI_Status status;
    MPI_Recv(&n, 1, MPI_UNSIGNED, MASTER, TAG_PARAMETER, COM, &status); 
    MPI_Recv(&l, 1, MPI_UNSIGNED, MASTER, TAG_PARAMETER, COM, &status);
    MPI_Recv(&d, 1, MPI_UNSIGNED, MASTER, TAG_PARAMETER, COM, &status);
    for (size_t j = 0; j < n; ++j)
    {
        bits_t input_value;
        MPI_Recv(&input_value, 1, MPI_UINT64_T, MASTER, TAG_PARAMETER, COM, &status);
        input.push_back(input_value);
    }
    MPI_Recv(&master_depth, 1, MPI_UNSIGNED, MASTER, TAG_PARAMETER, COM, &status);

    // 2.) while the master is sending work:
    //      a) receive subproblems
    //      b) locally solve (using findmotifs_worker(...))
    //      c) send results to master
    bool master_active = true;
    std::deque<bits_t> assignments;
    while(master_active)
    {
        // initiate receive
        MPI_Request request;
        MPI_Status status;
        bits_t element;
        MPI_Irecv(&element, 1, MPI_UINT64_T, MASTER, MPI_ANY_TAG, COM, &request);
        std::cerr << "calling irecv" << std::endl;

        // if we have any assignments, solve one and return results
        if (!assignments.empty())
        {
            bits_t task = assignments.front();
            assignments.pop_front();
            std::cerr << "solving problem: " << task << std::endl;
            std::vector<bits_t> solutions = findmotifs_worker(n, l, d, input.data(), master_depth, task);
            MPI_Send(solutions.data(), solutions.size(), MPI_UINT64_T, MASTER, TAG_RESULT, COM);
        }

        // finalize receive 
        std::cerr << "calling wait" << std::endl;
        MPI_Wait(&request, &status);
        if (status.MPI_TAG == TAG_TASK)
        {
            std::cerr << "pushing new task: " << std::bitset<64>(element) << std::endl;
            assignments.push_back(element);
        }
        else if (status.MPI_TAG == TAG_FINISHED)
        {
            std::cerr << "terminating" << std::endl;
            master_active = false;
        }

   }
    // 3.) you have to figure out a communication protocoll:
    //     - how does the master tell the workers that no more work will come?
    //       (i.e., when to break loop 2)
    // 4.) clean up: e.g. free allocated space
}

int processors;

void AssignTask(bits_t element, std::vector<bits_t>& solutions)
{
    static size_t slave_id = 0;

    // send the task to a worker
    std::cerr << "[master] sending element" << std::endl;
    MPI_Send(&element, 1, MPI_UINT64_T, slave_id + 1, TAG_TASK, COM);

    // receive the solutions (if any) from a worker
    std::cerr << "[master] waiting for solutions" << std::endl;
    MPI_Status status;
    MPI_Probe(slave_id + 1, TAG_RESULT, COM, &status);
    int buffer_size;
    std::cerr << "[master] there are " << buffer_size << " solutions" << std::endl;
    MPI_Get_count(&status, MPI_UINT64_T, &buffer_size);
    bits_t* buffer = (bits_t*)malloc(sizeof(bits_t) * buffer_size);
    MPI_Recv(buffer, buffer_size, MPI_UINT64_T, slave_id + 1, TAG_RESULT, COM, MPI_STATUS_IGNORE);
    for (int i = 0; i < buffer_size; ++i)
        solutions.push_back(buffer[i]);
    free(buffer);

    // find next slave, there are (p-1) slaves
    slave_id = (slave_id + 1) % (processors - 1);
}

void EnumerateMaster(bits_t element, size_t inversions, size_t position, const size_t k, const size_t d, std::vector<bits_t>& solutions)
{
    // if we have reached our k-depth for the master, send task to worker
    if (position == k)
    {
        AssignTask(element, solutions);
        return;
    }

    // enumerate candidate and inversion candidate
    EnumerateMaster(element, inversions, position + 1, k, d, solutions);
    if (inversions < d)
    {
        bits_t element_ = InvertBit(element, position);
        EnumerateMaster(element_, inversions + 1, position + 1, k, d, solutions);
    }
}

std::vector<bits_t> findmotifs_master(const unsigned int n,
                                      const unsigned int l,
                                      const unsigned int d,
                                      const bits_t* input,
                                      const unsigned int till_depth)
{
    std::vector<bits_t> results;

    // recursively search for solutions
    bits_t base = input[0];
    EnumerateMaster(base, 0, 0, till_depth, d, results);
    
    return results;
}

std::vector<bits_t> master_main(unsigned int n, unsigned int l, unsigned int d,
                                const bits_t* input, unsigned int master_depth)
{
    // figure out how many slaves there are
    MPI_Comm_size(COM, &processors);

    // 1.) send input to all workers (including n, l, d, input, depth)
    for (int i = 1; i < processors; ++i)
    {
        MPI_Request request;
        MPI_Isend(&n, 1, MPI_UNSIGNED, i, TAG_PARAMETER, COM, &request);
        MPI_Isend(&l, 1, MPI_UNSIGNED, i, TAG_PARAMETER, COM, &request);
        MPI_Isend(&d, 1, MPI_UNSIGNED, i, TAG_PARAMETER, COM, &request);
        for (size_t j = 0; j < n; ++j)
        {
            bits_t value = input[j];
            MPI_Isend(&value, 1, MPI_UINT64_T, i, TAG_PARAMETER, COM, &request);
        }
        MPI_Isend(&master_depth, 1, MPI_UNSIGNED, i, TAG_PARAMETER, COM, &request);
    }

    // 2.) solve problem till depth `master_depth` and then send subproblems
    //     to the workers and receive solutions in each communication
    //     Use your implementation of `findmotifs_master(...)` here.
    std::vector<bits_t> results = findmotifs_master(n, l, d, input, master_depth);

    // 3.) receive last round of solutions TODO
    
    // 4.) terminate (and let the workers know)
    for (int i = 1; i < processors; ++i)
    {
        MPI_Request request;
        uint64_t nothing = 0;
        MPI_Isend(&nothing, 1, MPI_UINT64_T, i, TAG_FINISHED, COM, &request);
    }

    return results;
}

