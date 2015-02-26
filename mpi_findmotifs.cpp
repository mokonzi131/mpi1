// Implement your solutions in this file
#include "findmotifs.h"
#include "hamming.h"
#include <mpi.h>

const MPI_Comm COM = MPI_COMM_WORLD;
const int MASTER = 0;
const int TAG_PARAMETER = 0x01;
const int TAG_TASK = 0x02;

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

    // TODO: implement your solution here

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
    std::cerr << "[worker] knows n=" << n << ", l=" << l << ", d=" << d << ", depth=" << master_depth << std::endl;
    
    // 2.) while the master is sending work:
    //      a) receive subproblems
    //      b) locally solve (using findmotifs_worker(...))
    //      c) send results to master

    // 3.) you have to figure out a communication protocoll:
    //     - how does the master tell the workers that no more work will come?
    //       (i.e., when to break loop 2)
    // 4.) clean up: e.g. free allocated space
}



std::vector<bits_t> findmotifs_master(const unsigned int n,
                                      const unsigned int l,
                                      const unsigned int d,
                                      const bits_t* input,
                                      const unsigned int till_depth)
{
    std::vector<bits_t> results;

    // TODO: implement your solution here

    return results;
}

std::vector<bits_t> master_main(unsigned int n, unsigned int l, unsigned int d,
                                const bits_t* input, unsigned int master_depth)
{
    // figure out how many slaves there are
    int processors;
    MPI_Comm_size(COM, &processors);

    // 1.) send input to all workers (including n, l, d, input, depth)
    for (int i = 1; i < processors; ++i)
    {
        std::cerr << "[MASTER] communicating with slave #" << i << std::endl;
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
        std::cerr << "[MASTER] done communicating" << std::endl;
    }

    // 2.) solve problem till depth `master_depth` and then send subproblems
    //     to the workers and receive solutions in each communication
    //     Use your implementation of `findmotifs_master(...)` here.
    std::vector<bits_t> results;
    bits_t test = 0x15;
    results.push_back(test);


    // 3.) receive last round of solutions
    // 4.) terminate (and let the workers know)

    return results;
}

