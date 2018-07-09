# gmm-em
  Image segmentation by Gaussian mixture model and expectation maximization method. gmmEM segmentation utility supports mulitmodality 3D images with or without image atlas. K-Mean++ would be used for initialization without image atlas.

## Prerequisites
  * CMake (for building)
  * ITK (for image IO)

## Usage
```
$./gmmEM --help
Usage:
./gmmEM [-I] [--help] [--only-initial] [-n classNum] [-m maxNumIteration] [-s samplesNum] [-w atlasWeight] [-i input ...] [-a atlas ...] [-o output ...]

Description:
The gmmEM utility use Gaussian mixture model and Expectation Maximization algorithm 
to segment multimodalities images with or without atlas. If no atlas, the [-n classNum]
must be specified, and K-Mean++ will be used to give a random initialization for EM.

Specify input images by [-i input ...].
Specify output names by [-o ouput ...], otherwise, gmmEM will give a reasonable name.
In interactive mode, the segmentation results will output after each EM iteration.
Specify maximum number of iteration steps in EM by [-m maxNumIteration] (optional).
Specify the number of samples by [-s samplesNum] (optional).
Specify interactive mode by [-I], the utility gmmEM will pause after each EM iteration:
- input 'Enter' key will continue next EM iteration, 
- input 'k' and 'Enter' keys will kill (stop) the utility gmmEM, 
- input 'c' and 'Enter' keys will leave interactive mode and give the final output,
- input 'r' and 'Enter' keys will remove input atlas, ([-a atlas ..] must be specified)
- input 'a' and 'Enter' keys will add input atlas back. ([-a atlas ..] must be specified)
```
