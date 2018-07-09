#ifndef SAMPLING_H
#define SAMPLING_H

#include "stdlib.h"
#include "time.h"

// To support both * and ** type, don't specify typename as pointer here
// but you need to put double* or double** to use it
template<typename T>
void ReservoirSampling(T& data, int dataSize, T& sample, int sampleSize, int*& sampleIdx)
{
  // fill the reservoir array
  for (int i = 0; i < sampleSize; i++)
    {
      sample[i] = data[i];
      sampleIdx[i] = i;
    }

  srand(time(0));

  // replace elements with gradually decreasing probability
  for (int i = sampleSize; i < dataSize; i++)
    {
      // important: inclusive range
      int idx = rand() % (i+1);
      if (idx < sampleSize)
        {
          sample[idx] = data[i];
          sampleIdx[idx] = i;
        }
    }
}


#endif // SAMPLING_H
