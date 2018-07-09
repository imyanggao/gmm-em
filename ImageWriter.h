#ifndef IMAGE_WRITER_H
#define IMAGE_WRITER_H

#include "itkImageFileWriter.h"

template<typename TImageType>
void ImageWriter(std::string filename, typename TImageType::Pointer image)
{
  typedef itk::ImageFileWriter<TImageType> WriterType;
  typename WriterType::Pointer writer = WriterType::New();
  try
  {
    writer->SetFileName(filename);
    writer->SetInput(image);
    writer->Update();
  }
  catch( itk::ExceptionObject &err )
  {
    std::cerr << "ImageWriter Exception Caught !" << std::endl;
    std::cerr << err << std::endl;
    exit(1);
  }
}

#endif
