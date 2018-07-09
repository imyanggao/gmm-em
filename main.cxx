#include "ImageSeriesReader.h"
#include "ImageSeriesWriter.h"
#include "EMGaussianMixtures.h"
#include "KMeansPlusPlus.h"
#include "Timer.h"
#include "Debug.h"
#include "Sampling.h"
#include "DataMatrix.h"
#include <bitset>

// 3 bit mode: help | interactive | atlas
typedef std::bitset<3> ModeType;

void Usage(void)
{
  std::cout << "Usage:" << std::endl
            << "./gmmEM [-I] [--help] [--only-initial] [-n classNum] [-m maxNumIteration] [-s samplesNum] [-w atlasWeight] [-i input ...] [-a atlas ...] [-o output ...]"
            << std::endl << std::endl << "Description:" << std::endl
            << "The gmmEM utility use Gaussian mixture model and Expectation Maximization algorithm " << std::endl
            << "to segment multimodalities images with or without atlas. If no atlas, the [-n classNum]" << std::endl
            << "must be specified, and K-Mean++ will be used to give a random initialization for EM." << std::endl << std::endl
            << "Specify input images by [-i input ...]." << std::endl
            << "Specify output names by [-o ouput ...], otherwise, gmmEM will give a reasonable name." << std::endl
            << "In interactive mode, the segmentation results will output after each EM iteration." << std::endl
            << "Specify maximum number of iteration steps in EM by [-m maxNumIteration] (optional)." << std::endl
            << "Specify the number of samples by [-s samplesNum] (optional)." << std::endl
            << "Specify interactive mode by [-I], the utility gmmEM will pause after each EM iteration:" << std::endl
            << "- input 'Enter' key will continue next EM iteration, " << std::endl
            << "- input 'k' and 'Enter' keys will kill (stop) the utility gmmEM, " << std::endl
            << "- input 'c' and 'Enter' keys will leave interactive mode and give the final output," << std::endl
            << "- input 'r' and 'Enter' keys will remove input atlas, ([-a atlas ..] must be specified)" << std::endl
            << "- input 'a' and 'Enter' keys will add input atlas back. ([-a atlas ..] must be specified)" << std::endl
            << std::endl;
}

int cmdParser(int argc, char* argv[],
              ModeType& mode, int& numOfClass,
              int& maxIteration, int& sampleNum,
              double& atlasWeight, bool& onlyInitial,
              StringVector& inputNames,
              StringVector& atlasNames,
              StringVector& outputNames)
{
  bool in_input = false;
  bool in_atlas = false;
  bool in_output = false;
  bool in_classNum = false;
  bool in_maxIteration = false;
  bool in_sampleNum = false;
  bool in_atlasWeight = false;

  if (argc == 1)
    {
      mode[0] = true;
      return 0;
    }
  else
    {
      for (int i = 1; i < argc; ++i)
        {
          std::string str(argv[i]);
          if (str == "--help")
            {
              mode[0] = true;
              in_input = false;
              in_atlas = false;
              in_output = false;
              in_classNum = false;
              in_maxIteration = false;
              in_sampleNum = false;
              in_atlasWeight = false;
              return 0;
            }
          if (str == "-I")
            {
              mode[1] = true;
              in_input = false;
              in_atlas = false;
              in_output = false;
              in_classNum = false;
              in_maxIteration = false;
              in_sampleNum = false;
              in_atlasWeight = false;
              continue;
            }
          if (str == "--only-initial")
            {
              onlyInitial = true;
              in_input = false;
              in_atlas = false;
              in_output = false;
              in_classNum = false;
              in_maxIteration = false;
              in_sampleNum = false;
              in_atlasWeight = false;
              continue;
            }
          if (str == "-i")
            {
              in_input = true;
              in_atlas = false;
              in_output = false;
              in_classNum = false;
              in_maxIteration = false;
              in_sampleNum = false;
              in_atlasWeight = false;
              continue;
            }
          if (str == "-a")
            {
              mode[2] = true;
              in_atlas = true;
              in_input = false;
              in_output = false;
              in_classNum = false;
              in_maxIteration = false;
              in_sampleNum = false;
              in_atlasWeight = false;
              continue;
            }
          if (str == "-o")
            {
              in_output = true;
              in_input = false;
              in_atlas = false;
              in_classNum = false;
              in_maxIteration = false;
              in_sampleNum = false;
              in_atlasWeight = false;
              continue;
            }
          if (str == "-n")
            {
              in_classNum = true;
              in_input = false;
              in_atlas = false;
              in_output = false;
              in_maxIteration = false;
              in_sampleNum = false;
              in_atlasWeight = false;
              continue;
            }
          if (str == "-m")
            {
              in_maxIteration = true;
              in_classNum = false;
              in_input = false;
              in_atlas = false;
              in_output = false;
              in_sampleNum = false;
              in_atlasWeight = false;
              continue;
            }
          if (str == "-s")
            {
              in_sampleNum = true;
              in_classNum = false;
              in_input = false;
              in_atlas = false;
              in_output = false;
              in_maxIteration = false;
              in_atlasWeight = false;
              continue;
            }
          if (str == "-w")
            {
              in_atlasWeight = true;
              in_sampleNum = false;
              in_classNum = false;
              in_input = false;
              in_atlas = false;
              in_output = false;
              in_maxIteration = false;
              continue;
            }
          if (in_input == true)
            {
              inputNames.push_back(str);
              continue;
            }
          if (in_atlas == true)
            {
              atlasNames.push_back(str);
              continue;
            }
          if (in_output == true)
            {
              outputNames.push_back(str);
              continue;
            }
          if (in_classNum == true)
            {
              numOfClass = atoi(str.c_str());
              continue;
            }
          if (in_maxIteration == true)
            {
              maxIteration = atoi(str.c_str());
              continue;
            }
          if (in_sampleNum == true)
            {
               sampleNum = atoi(str.c_str());
               continue;
            }
          if (in_atlasWeight == true)
            {
              atlasWeight = atof(str.c_str());
              continue;
            }
        }

      // error handle here
      int inputSize = inputNames.size();
      int atlasSize = atlasNames.size();
      int outputSize = outputNames.size();
      if (inputSize == 0)
        {
          std::cerr << "No input images ! Use [-i input ...] to specify !" << std::endl;
          return 1;
        }
      if (mode[2] == false)
        {
          if (numOfClass <= 0)
            {
              std::cerr << "If no atlas specified, desired number of class must be specify by using [-n classNum] !" << std::endl;
              return 1;
            }
        }
      else
        {
          if (atlasSize == 0)
            {
              std::cerr << "Missing atlas ! Use [-a atlas ...] to specify !" << std::endl;
              return 1;
            }
          if (numOfClass == 0)
            {
              numOfClass = atlasSize;
            }
          else
            {
              if (atlasSize != numOfClass)
                {
                  std::cerr << "Desired number of class conflict with given number of atlas !" << std::endl;
                  return 1;
                }
            }
        }

      if (outputSize == 0)
        {
          std::string extension = ".nii";
          std::stringstream filename;
          for (int i = 0; i < numOfClass; i++)
            {
              filename.str("");
              filename << i << extension;
              outputNames.push_back(filename.str());
            }
        }
      else
        {
          if (outputSize != numOfClass)
            {
              std::cerr << "Desired number of class conflict with given number of output names !" << std::endl;
              return 1;
            }
        }
      if (maxIteration == 0)
        {
          maxIteration = 50;
        }
      if (mode[2] == false)
        {
          if (atlasWeight != 1.0)
            {
              std::cerr << "[-w atlasWeight] could only be accecpted when specifying atlas by [-a atlas ...] !" << std::endl;
              return 1;
            }
        }
      else
        {
          if ((atlasWeight > 1) || (atlasWeight < 0))
            {
              std::cerr << "atlasWeight could only between 0 and 1 !" << std::endl;
              return 1;
            }
        }
      if ((onlyInitial == true) && (mode[2] == false))
        {
          std::cerr << "[--only-initial] could only be used with [-a atlas ...] !" << std::endl;
          return 1;
        }
      return 0;
    }
}

int main(int argc, char* argv[])
{
  ModeType mode;
  int numOfClass = 0;
  int maxIteration = 0;
  int sampleNum = 0;
  double atlasWeight = 1.0;
  bool onlyInitial = false;
  StringVector inputNames, atlasNames, outputNames;
  int ret = 0;
  ret = cmdParser(argc, argv, mode, numOfClass, maxIteration, sampleNum,
                  atlasWeight, onlyInitial, inputNames, atlasNames, outputNames);

  if (ret != 0)
    {
      return 1;
    }
  else
    {
      if (mode[0] == true)
        {
          Usage();
          return 0;
        }

      typedef itk::Image<float, 3> ImageType;
      ImageType::SizeType imageSize;
      double** data;
      double** atlas;
      long dataNum, dataDim;
      long atlasDataNum, atlasDataDim;

      ImageSeriesReader<ImageType>(inputNames, data, dataNum, dataDim, imageSize);
      if ((sampleNum == 0) || (sampleNum > dataNum))
        {
          sampleNum = dataNum;
        }

      double** dataSample = new double*[sampleNum];
      double** atlasSample = new double*[sampleNum];
      int* sampleIdx = new int[sampleNum];
      ReservoirSampling<double**>(data, dataNum, dataSample, sampleNum, sampleIdx);

      if (mode[2] == true)
        {
          ImageSeriesReader<ImageType>(atlasNames, atlas, atlasDataNum, atlasDataDim, imageSize);
          const double avgWeight = 1.0 / (double) atlasDataDim;
          PRINT("\n%s", "Weight atlas Begin")
          for (int i = 0; i < atlasDataNum; i++)
            {
              for (int j = 0; j < atlasDataDim; j++)
                {
                  atlas[i][j] = atlasWeight * (atlas[i][j]-avgWeight) + avgWeight;
                }
            }
          PRINT("\n%s\n", "Weight atlas End")
          for (int i = 0; i < sampleNum; i++)
            {
              atlasSample[i] = atlas[sampleIdx[i]];
            }
        }

      EMGaussianMixtures *emSample = new EMGaussianMixtures(dataSample, sampleNum, dataDim, numOfClass);
      emSample->SetMaxIteration(maxIteration);

      EMGaussianMixtures *emData = new EMGaussianMixtures(data, dataNum, dataDim, numOfClass);

      if (mode[2] == true)
        {
          emSample->SetPrior(atlasSample, onlyInitial);
          emData->SetPrior(atlas, onlyInitial);
        }
      else
        {
          // K-mean++ initialization
          KMeansPlusPlus *kmeanspp = new KMeansPlusPlus(dataSample, sampleNum, dataDim, numOfClass);
          kmeanspp->Initialize();
          emSample->SetGaussianMixtureModel(kmeanspp->GetGaussianMixtureModel());
        }

      double** probs;

//      // trick 1: seperate rest atlas to 3 equal pieces
//      emSample->GetGaussianMixtureModel()->SetMean(3, emSample->GetGaussianMixtureModel()->GetMean(0));
//      emSample->GetGaussianMixtureModel()->SetMean(4, emSample->GetGaussianMixtureModel()->GetMean(1));
////      emSample->GetGaussianMixtureModel()->GetMean(5)[0] = 0;
////      emSample->GetGaussianMixtureModel()->GetMean(5)[1] = 0;
//      emSample->GetGaussianMixtureModel()->PrintParameters();
//      getchar();

      if (mode[1] == true)
        {
          std::string str;
          while(! emSample->StopCriteria())
            {
              std::getline(std::cin, str);
              if (str == "k")
                {
                  break;
                }
              if (str == "c")
                {
                  while(! emSample->StopCriteria())
                    {
                      emSample->UpdateOnce();
                    }
                  PRINT("\n%s\n","Write result images Begin")
                  emData->SetGaussianMixtureModel(emSample->GetGaussianMixtureModel());
                  probs = emData->ComputeLatent();
                  ImageSeriesWriter<ImageType>(outputNames, probs, imageSize);
                  PRINT("%s\n","Write result images End")
                  break;
                }
              if ((str == "r") && (mode[2] == true))
                {
                  emSample->RemovePrior();
                  std::cout << "Atlas removed !!!" << std::endl;
                  continue;
                }
              if ((str == "a") && (mode[2] == true))
                {
                  emSample->SetPrior(atlasSample, onlyInitial);
                  std::cout << "Atlas add back !!!" << std::endl;
                  continue;
                }
              emSample->UpdateOnce();
              PRINT("\n%s\n","Write result images Begin")
              emData->SetGaussianMixtureModel(emSample->GetGaussianMixtureModel());
              probs = emData->ComputeLatent();
              ImageSeriesWriter<ImageType>(outputNames, probs, imageSize);
              PRINT("%s\n","Write result images End")
            }
        }
      else
        {
          emSample->Update();
          PRINT("\n%s\n","Write result images Begin")
          emData->SetGaussianMixtureModel(emSample->GetGaussianMixtureModel());
          probs = emData->ComputeLatent();
          ImageSeriesWriter<ImageType>(outputNames, probs, imageSize);

          for (int i = 0; i < dataNum; i++) {
            int idx = -1;
            double val = -1;
            for (int j = 0; j < numOfClass; j++) {
              if (probs[i][j] > val) {
                val = probs[i][j];
                idx = j;
              }
            }
            for (int j = 0; j < numOfClass; j++) {
              if (j == idx) {
                probs[i][j] = 1;
              }
              else {
                probs[i][j] = 0;
              }
            }
          }
          StringVector hardoutput(outputNames.size());
          for (int i = 0; i < outputNames.size(); i++) {
            hardoutput[i] = "label_" + outputNames[i];
            std::cout << hardoutput[i] << std::endl;
          }
          ImageSeriesWriter<ImageType>(hardoutput, probs, imageSize);

          PRINT("%s\n","Write result images End")
        }

//      std::cout << "Begin quick test:" << std::endl;
//      for (int i = 0; i < dataNum; ++i)
//        {
//          if ((data[i][0] < emSample->GetGaussianMixtureModel()->GetMean(1)[0])
//              && (data[i][1] < emSample->GetGaussianMixtureModel()->GetMean(1)[1]))
//            {
//              probs[i][2] = 0;
//            }
//        }
//      ImageSeriesWriter<ImageType>(outputNames, probs, imageSize);
//      std::cout << "End test" << std::endl;
    }


}
