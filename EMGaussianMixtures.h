#ifndef EM_GAUSSIAN_MIXTURES_H
#define EM_GAUSSIAN_MIXTURES_H

#include "GaussianMixtureModel.h"

class EMGaussianMixtures
{
public:
  EMGaussianMixtures(double **x, int dataSize, int dataDim, int numOfClass);
  ~EMGaussianMixtures();

  void Reset(void);
  void SetMaxIteration(int maxIteration);
  void SetPrecision(double precision);
  void SetParameters(int index, double *mean, double *covariance, double weight);
  void SetGaussianMixtureModel(GaussianMixtureModel *gmm);
  void SetPrior(double **prior, bool onlyInitial);
  void RemovePrior(void);

  GaussianMixtureModel *GetGaussianMixtureModel() const { return m_gmm; }
  
  int GetMaxIteration(void);
  int GetIterationNum(void);

  bool StopCriteria(void);

  double ** Update(void);
  double ** UpdateOnce(void);
  double ** ComputeLatent(void);
  double EvaluateLogLikelihood(void);
  void PrintParameters(void);

  static double ComputePosterior(int nGauss, double *log_pdf, double *w, double *log_w, int j);

private:
  void EvaluatePDF(void);
  void UpdateLatent(void);
  void UpdateMean(void);
  void UpdateCovariance(void);
  void UpdateWeight(void);
  
  double **m_latent;
  double **m_log_pdf;
  double **m_prior;
  double **m_log_prior;
  double **m_x;
  double *m_probs;
  double *m_probs2;
  double *m_probs3;
  double *m_tmp1;
  double *m_tmp2;
  double *m_tmp3;
  double *m_sum;
  double *m_weight;
  double *m_log_weight;
  double m_logLikelihood;
  int m_numOfGaussian;
  int m_dimOfGaussian;
  int m_maxIteration;
  int m_numOfIteration;
  int m_numOfData;
  int m_setPriorFlag;
  double m_precision;
  GaussianMixtureModel *m_gmm;
  bool m_fail;
  bool m_converged;
  bool m_reachMaxIteration;
};

#endif
