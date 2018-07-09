#ifndef IMAGE_SERIES_WRITER_H
#define IMAGE_SERIES_WRITER_H

#include "itkImageRegionIterator.h"
#include "ImageWriter.h"

typedef std::vector<std::string> StringVector;

template<typename TImageType>
void ImageSeriesWriter(StringVector filenames,
                       double** data,
                       typename TImageType::SizeType size)
{
  typename TImageType::IndexType index;
  index.Fill(0);
  typename TImageType::RegionType region(index, size);
  typename TImageType::Pointer image = TImageType::New();
  image->SetRegions(region);
  image->Allocate();

  typedef itk::ImageRegionIterator<TImageType> IteratorType;

  int dimIdx = 0;
  for (StringVector::const_iterator filename = filenames.begin();
       filename != filenames.end(); ++filename, ++dimIdx)
    {
      IteratorType iter(image, image->GetLargestPossibleRegion());
      long dataIdx = 0;
      for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter, ++dataIdx)
        {
          iter.Set(data[dataIdx][dimIdx]);
        }

      ImageWriter<TImageType>(*filename, image);
    }
}


template<typename TImageType>
void ImageSeriesWriter(double** data,
                       typename TImageType::SizeType size,
                       long dataDim)
{
  typename TImageType::IndexType index;
  index.Fill(0);
  typename TImageType::RegionType region(index, size);
  typename TImageType::Pointer image = TImageType::New();
  image->SetRegions(region);
  image->Allocate();

  typedef itk::ImageRegionIterator<TImageType> IteratorType;

  std::string extension = ".nii";
  std::stringstream filename;
  int dimIdx = 0;
  for (int i = 0; i < dataDim; ++i, ++dimIdx)
    {
      IteratorType iter(image, image->GetLargestPossibleRegion());
      long dataIdx = 0;
      for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter, ++dataIdx)
        {
          iter.Set(data[dataIdx][dimIdx]);
        }

      filename.str("");
      filename << i << extension;
      ImageWriter<TImageType>(filename.str(), image);
    }
}

template<typename TImageType>
void ImageSeriesWriter(StringVector filenames,
                       std::vector<typename TImageType::Pointer> imageVector)
{
  int imageIdx = 0;
  for (StringVector::const_iterator filename = filenames.begin();
       filename != filenames.end(); ++filename, ++imageIdx)
    {
      ImageWriter<TImageType>(*filename, imageVector[imageIdx]);
    }
}

template<typename TImageType>
void ImageSeriesWriter(std::vector<typename TImageType::Pointer> imageVector)
{
  std::string extension = ".nii";
  std::stringstream filename;
  int imageNum = imageVector.size();
  for (int imageIdx = 0; imageIdx < imageNum; ++imageIdx)
    {
      filename.str("");
      filename << imageIdx << extension;
      ImageWriter<TImageType>(filename.str(), imageVector[imageIdx]);
    }
}

#endif // IMAGE_SERIES_WRITER_H
