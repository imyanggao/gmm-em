#ifndef IMAGE_SERIES_READER_H
#define IMAGE_SERIES_READER_H

#include "ImageSizeReader.h"
#include "itkImageRegionConstIterator.h"

typedef std::vector<std::string> StringVector;

template<typename TImageType>
void ImageSeriesReader(StringVector filenames,
                       double**& data,
                       long& dataNum,
                       long& dataDim,
                       typename TImageType::SizeType& size)
{
  typedef itk::ImageRegionConstIterator<TImageType> ConstIteratorType;
  typename TImageType::Pointer image;
  typename TImageType::SizeType curSize;

  int imageDim = size.GetSizeDimension();
  dataDim = filenames.size();
  dataNum = 1;

  int dimIdx = 0;
  for (StringVector::const_iterator filename = filenames.begin();
       filename != filenames.end(); ++filename, ++dimIdx)
    {
      ImageSizeReader<TImageType>(*filename, curSize, image);

      if (filename == filenames.begin())
        {
          size = curSize;
          for (int i = 0; i < imageDim; i++)
            {
              dataNum *= size[i];
            }
          data = new double*[dataNum];
          double *block = new double[dataNum*dataDim];
          for (int i = 0; i < dataNum; i++)
            {
              data[i] = &block[i*dataDim];
            }
        }
      else if (curSize != size)
        {
          std::cerr << "ImageSeriesReader size different !" << std::endl;
          exit(1);
        }

      ConstIteratorType iter(image, image->GetLargestPossibleRegion());
      long dataIdx = 0;
      for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter, ++dataIdx)
        {
          data[dataIdx][dimIdx] = iter.Get();
        }
    }
}

template<typename TImageType>
void ImageSeriesReader(StringVector filenames,
                       std::vector<typename TImageType::Pointer>& imageVector,
                       long& dataDim,
                       typename TImageType::SizeType& size)
{
  typedef itk::ImageRegionConstIterator<TImageType> ConstIteratorType;
  typename TImageType::Pointer image;
  typename TImageType::SizeType curSize;

  dataDim = filenames.size();

  for (StringVector::const_iterator filename = filenames.begin();
       filename != filenames.end(); ++filename)
    {
      ImageSizeReader<TImageType>(*filename, curSize, image);

      if (filename == filenames.begin())
        {
          size = curSize;
        }
      else if (curSize != size)
        {
          std::cerr << "ImageSeriesReader size different !" << std::endl;
          exit(1);
        }
      imageVector.push_back(image);
    }
}

#endif // IMAGE_SERIES_READER_H
