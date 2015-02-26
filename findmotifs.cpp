// Implement your solutions in this file
#include "findmotifs.h"
#include "hamming.h"

struct Parameters 
{
    unsigned int NUM_ELEMENTS;
    unsigned int NUM_POSITIONS;
    unsigned int HAMMING_DISTANCE;
    const bits_t* pInput;
};

bits_t InvertBit(bits_t element, unsigned int position);
void Enumerate(const Parameters& parameters, bits_t element, unsigned int inversions, unsigned int position, std::vector<bits_t>& result);
void Test(const Parameters& parameters, bits_t element, std::vector<bits_t>& result);

// implements the sequential findmotifs function
std::vector<bits_t> findmotifs(unsigned int n, unsigned int l,
                               unsigned int d, const bits_t* input)
{
    // If you are not familiar with C++ (using std::vector):
    // For the output (return value) `result`:
    //                  The function asks you to return all values which are
    //                  of a hamming distance `d` from all input values. You
    //                  should return all these values in the return value
    //                  `result`, which is a std::vector.
    //                  For each valid value that you find (i.e., each output
    //                  value) you add it to the output by doing:
    //                      result.push_back(value);
    //                  Note: No other functionality of std::vector is needed.
    // You can get the size of a vector (number of elements) using:
    //                      result.size()

    // create an empty vector
    std::vector<bits_t> result;

    // start with base number and solution parameters
    bits_t base = input[0];
    Parameters parameters;
    parameters.NUM_ELEMENTS = n;
    parameters.NUM_POSITIONS = l;
    parameters.HAMMING_DISTANCE = d;
    parameters.pInput = input;

    // enumerate elements in the search space
    Enumerate(parameters, base, 0, 0, result);

    return result;
}

void Test(const Parameters& parameters, bits_t element, std::vector<bits_t>& result)
{
    bool valid = true;
    for (unsigned int i = 0; i < parameters.NUM_ELEMENTS; ++i)
    {
        if (hamming(element, parameters.pInput[i]) > parameters.HAMMING_DISTANCE)
            valid = false;
    }

    if (valid)
        result.push_back(element);
}

void Enumerate(const Parameters& parameters, bits_t element, unsigned int inversions, unsigned int position, std::vector<bits_t>& result)
{
    // check for a possible solution
    if (position == parameters.NUM_POSITIONS)
    {
        Test(parameters, element, result);
        return;
    }

    // enumerate candidate and its inversion candidate
    Enumerate(parameters, element, inversions, position + 1, result);
    if (inversions < parameters.HAMMING_DISTANCE)
    {
        bits_t element_ = InvertBit(element, position);
        Enumerate(parameters, element_, inversions + 1, position + 1, result);
    }
}

bits_t InvertBit(bits_t element, unsigned int position)
{
    bits_t MASK = 0x01;
    return (element ^ (MASK << position));
}

