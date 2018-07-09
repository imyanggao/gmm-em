#ifndef IMAGE_SIZE_READER_H
#define IMAGE_SIZE_READER_H

#include "itkImageFileReader.h"

template<typename TImageType>
void ImageSizeReader(std::string filename, typename TImageType::SizeType& size, typename TImageType::Pointer& image)
{
  typedef itk::ImageFileReader<TImageType> ReaderType;
  typename ReaderType::Pointer reader = ReaderType::New();
  typename TImageType::RegionType region;
  try
  {
    reader->SetFileName(filename);
    reader->Update();
    image = reader->GetOutput();
    region = image->GetLargestPossibleRegion();
    size = region.GetSize();
  }
  catch( itk::ExceptionObject &err )
  {
    std::cerr << "ImageSizeReader Exception Caught !" << std::endl;
    std::cerr << err << std::endl;
    exit(1);
  }
}

#endif // IMAGE_SIZE_READER_H
