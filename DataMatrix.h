#ifndef DATA_MATRIX_H
#define DATA_MATRIX_H

template<typename T>
T** NewMatrix(int N, int D)
{
  T** data = new T*[N];
  T* block = new T[N*D];
  for (int i = 0; i < N; i++)
    {
      data[i] = &block[i*D];
    }
  return data;
}

template<typename T>
void DeleteMatrix(T** data)
{
  delete *data;
  delete data;
}

#endif // DATA_MATRIX_H
