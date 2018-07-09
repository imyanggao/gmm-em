#include "EMGaussianMixtures.h"
#include <iostream>
#include "Timer.h"
#include "Debug.h"


EMGaussianMixtures::EMGaussianMixtures(double **x, int dataSize, int dataDim, int numOfClass)
  :m_x(x), m_numOfData(dataSize), m_dimOfGaussian(dataDim),
    m_numOfGaussian(numOfClass), m_setPriorFlag(0), m_numOfIteration(0),
    m_fail(false), m_converged(false), m_reachMaxIteration(false)
{
  m_latent = new double*[dataSize];
  m_probs = new double[dataSize*numOfClass];
  for (int i = 0; i < dataSize; i++)
    {
    m_latent[i] = &m_probs[i*numOfClass];
    }
  m_log_pdf = new double*[dataSize];
  m_probs2 = new double[dataSize*numOfClass];
  for (int i = 0; i < dataSize; i++)
    {
    m_log_pdf[i] = &m_probs2[i*numOfClass];
    }
  m_tmp1 = new double[numOfClass];
  m_tmp2 = new double[dataDim];
  m_tmp3 = new double[dataDim*dataDim];
  m_sum = new double[numOfClass];
  m_weight = new double[numOfClass];
  m_log_weight = new double[numOfClass];
  m_gmm = new GaussianMixtureModel(dataDim, numOfClass);
  m_maxIteration = 30;
  m_precision = pow(10,-7);
  m_logLikelihood = -std::numeric_limits<double>::infinity();
}

EMGaussianMixtures::~EMGaussianMixtures()
{
  delete m_probs;
  delete m_probs2;
  delete m_latent;
  delete m_log_pdf;
  delete m_tmp1;
  delete m_tmp2;
  delete m_tmp3;
  delete m_sum;
  delete m_gmm;
  delete m_weight;
  delete m_log_weight;
  if (m_setPriorFlag == 1)
    {
      delete m_probs3;
      delete m_log_prior;
    }
}

void EMGaussianMixtures::Reset(void)
{
  m_numOfIteration = 0;
  m_fail = false;
  m_converged = false;
  m_reachMaxIteration = false;
  m_logLikelihood = -std::numeric_limits<double>::infinity();
  for (int i = 0; i < m_numOfData*m_numOfGaussian; i++)
    {
    m_probs[i] = 0;
    m_probs2[i] = 0;
    }
  // if m_prior flag == 1 deal with
}

void EMGaussianMixtures::SetMaxIteration(int maxIteration)
{
  m_maxIteration = maxIteration;
}

void EMGaussianMixtures::SetPrecision(double precision)
{
  m_precision = precision;
}

void EMGaussianMixtures::SetParameters(int index, double *mean, double *covariance, double weight)
{
  m_gmm->SetGaussian(index, mean, covariance);
  m_gmm->SetWeight(index, weight);
}

void EMGaussianMixtures::SetGaussianMixtureModel(GaussianMixtureModel *gmm)
{
  for (int i = 0; i < m_numOfGaussian; i++)
    {
    m_gmm->SetGaussian(i, gmm->GetMean(i), gmm->GetCovariance(i));
    m_gmm->SetWeight(i, gmm->GetWeight(i));
    }
}

void EMGaussianMixtures::SetPrior(double **prior, bool onlyInitial)
{
  m_prior = prior;
  if (m_setPriorFlag == 0)
    {
      m_setPriorFlag = 1;
      m_log_prior = new double*[m_numOfData];
      m_probs3 = new double[m_numOfData*m_numOfGaussian];
      for (int i = 0; i < m_numOfData; i++)
        {
          m_log_prior[i] = &m_probs3[i*m_numOfGaussian];
        }
    }
  for (int i = 0; i < m_numOfGaussian; i++)
    {
      m_sum[i] = 0;
    }
  for (int i = 0; i < m_numOfData; i++)
    {
      for (int j = 0; j < m_numOfGaussian; j++)
        {
          m_log_prior[i][j] = log(m_prior[i][j]);
          m_latent[i][j] = m_prior[i][j];
          m_sum[j] += m_latent[i][j];
        }
    }
  UpdateMean();
  UpdateCovariance();
  if (onlyInitial == true)
    {
      UpdateWeight();
      m_setPriorFlag = 0;
    }
}

void EMGaussianMixtures::RemovePrior(void)
{
  if (m_setPriorFlag == 1)
    {
      delete m_probs3;
      delete m_log_prior;
      m_log_prior = 0;
      m_prior = 0;
      m_setPriorFlag = 0;
      UpdateWeight();
    }
}

int EMGaussianMixtures::GetMaxIteration(void)
{
  return m_maxIteration;
}

int EMGaussianMixtures::GetIterationNum(void)
{
  return m_numOfIteration;
}

bool EMGaussianMixtures::StopCriteria(void)
{
  if (m_fail || m_converged || m_reachMaxIteration)
    {
      return true;
    }
  else
    {
      return false;
    }
}

double ** EMGaussianMixtures::Update(void)
{
  double currentLogLikelihood = 0;
  m_numOfIteration = 0;
  m_fail = false;
  m_converged = false;
  m_reachMaxIteration = false;
  int converged_num = 0;
  int fail_num = 0;
  while (1)
  {
    ++m_numOfIteration;

//      //trick 2:
//      if (m_numOfIteration % 3 == 0)
//        {
//      for (int i = 0; i < m_numOfData; i++)
//        {
//          for (int j = 0; j < m_numOfGaussian; j++)
//            {
//              m_prior[i][j] = 0.95*(m_prior[i][j]-0.25) + 0.25;
//              m_log_prior[i][j] = log(m_prior[i][j]);
//            }
//        }
//        }

    PRINT("\n%s\n", "==============================")
    PRINT("%d Iteration Begin:\n", m_numOfIteration)

    FCN_TIMER( EvaluatePDF(); )
    FCN_TIMER( currentLogLikelihood = EvaluateLogLikelihood(); )
    if (m_logLikelihood > currentLogLikelihood)
    {
      fail_num++;
      PRINT("!!! Log Likelihood decrease %d time: old = %f, new = %f\n", fail_num, m_logLikelihood, currentLogLikelihood)
      if (fail_num > 3) {
      m_fail = true;
      break;
    }
    }
    else {
      fail_num = 0;
    }

    FCN_TIMER( UpdateLatent(); )
    FCN_TIMER( UpdateMean(); )
    FCN_TIMER( UpdateCovariance(); )
    FCN_TIMER( UpdateWeight(); )

    PRINT("\n%d Iteration End:\n", m_numOfIteration)
    PRINT("Log likelihood:\n%f\n", m_logLikelihood)
    PrintParameters();

    if (fabs(m_logLikelihood - currentLogLikelihood) <= m_precision)
    {
      converged_num++;
      PRINT("!!! Log Likelihood converged %d time: old = %f, new = %f\n", converged_num, m_logLikelihood, currentLogLikelihood)
      if (converged_num > 3) {
      m_converged = true;
      break;
    }
    }
    else {
      converged_num = 0;
    }

    m_logLikelihood = currentLogLikelihood;

    if (m_numOfIteration >= m_maxIteration)
    {
      m_reachMaxIteration = true;
      PRINT("%s\n", "!!! Reach the maximum iteration number")
      break;
    }
  }
  return m_latent;
}

double ** EMGaussianMixtures::UpdateOnce(void)
{
  double currentLogLikelihood;
  ++m_numOfIteration;

  PRINT("\n%s\n", "==============================")
  PRINT("%d Iteration Begin:\n", m_numOfIteration)

  FCN_TIMER( EvaluatePDF(); )
  FCN_TIMER( currentLogLikelihood = EvaluateLogLikelihood(); )

  FCN_TIMER( UpdateLatent(); )
  FCN_TIMER( UpdateMean(); )
  FCN_TIMER( UpdateCovariance(); )
  FCN_TIMER( UpdateWeight(); )

  PRINT("\n%d Iteration End:\n", m_numOfIteration)
  PRINT("Log likelihood:\n%f\n", m_logLikelihood)
  PrintParameters();

  if (m_logLikelihood > currentLogLikelihood)
  {
    m_fail = true;
    PRINT("!!! Log Likelihood decrease: old = %f, new = %f\n", m_logLikelihood, currentLogLikelihood)
  }
  if (fabs(m_logLikelihood - currentLogLikelihood) <= m_precision)
  {
    m_converged = true;
    PRINT("%s\n", "!!! Log Likelihood converged")
  }
  if (m_numOfIteration >= m_maxIteration)
  {
    m_reachMaxIteration = true;
    PRINT("%s\n", "!!! Reach the maximum iteration number")
  }

  m_logLikelihood = currentLogLikelihood;

  return m_latent;
}

double ** EMGaussianMixtures::ComputeLatent(void)
{
  EvaluatePDF();
  UpdateLatent();
  return m_latent;
}

void EMGaussianMixtures::EvaluatePDF(void)
{
  for (int i = 0; i < m_numOfData; i++)
    {
    for (int j = 0; j < m_numOfGaussian; j++)
      {
        if(!m_gmm->GetGaussian(j)->isDeltaFunction())
      m_log_pdf[i][j] = m_gmm->EvaluateLogPDF(j, m_x[i]);
      }
    }

    for (int j = 0; j < m_numOfGaussian; j++)
      {
      m_weight[j] = m_gmm->GetWeight(j);
      m_log_weight[j] = log(m_weight[j]);
      }
}

#include <vnl/vnl_math.h>

double EMGaussianMixtures::ComputePosterior(int nGauss, double *log_pdf, double *w, double *log_w, int j)
{
  // Instead of directly computing the expression
  //   latent[i][j] = w[j] * N(x_i; m_j, Sigma_j) / Sum_k[w[k] * N(x_i; m_k, Sigma_k)]
  // which is equivalently
  //   latent[i][j] = exp(a_j) / Sum_k[ exp(a_k) ]
  // where
  //   a_j = log( w[j] * N(x_i; m_j, Sigma_j) )
  // we compute
  //   latent[i][j] = 1 / (1 + Sum_(k!=j)[ exp(a_k - a_j) ])
  // which is numerically stable

  // If the weight of the class is zero, the posterior is automatically zero
  if(w[j] == 0)
    return 0;

  // We are computing m_latent[i][j]
  double denom = 1.0;

  // The log of w[j] * pdf[j];
  double exp_j = (log_w[j] + log_pdf[j]);
  for (int k = 0; k < nGauss; k++)
    {
    if(j != k && w[k] > 0)
      {
      // The log of (w[k] * pdf[k]) / (w[j] * pdf[j])
      double exponent = (log_w[k] + log_pdf[k]) - exp_j;

      if(exponent < -20)
        {
        // (w[k] * pdf[k]) / (w[j] * pdf[j]) is effectively zero
        continue;
        }
      else if(exponent > 20)
        {
        // latent[i][j] is effectively zero
        denom = vnl_huge_val(1.0);
        break;
        }
      else
        {
        denom += exp(exponent);
        }
      }
    }

  // Now compute 1/denom
  double post = 1.0 / denom;
  return post;
}

void EMGaussianMixtures::UpdateLatent(void)
{
  double sum = 0;
  for (int i = 0; i < m_numOfGaussian; i++)
    {
    m_sum[i] = 0;
    }

//  if (m_setPriorFlag == 0)
//    {
    for (int i = 0; i < m_numOfData; i++)
      {
      for (int j = 0; j < m_numOfGaussian; j++)
        {
          if (m_setPriorFlag == 0)
            {
              m_latent[i][j] = ComputePosterior(m_numOfGaussian, m_log_pdf[i], m_weight, m_log_weight, j);
            }
          else
            {
              m_latent[i][j] = ComputePosterior(m_numOfGaussian, m_log_pdf[i], m_prior[i], m_log_prior[i], j);
            }
        if (m_latent[i][j] != m_latent[i][j])
        {
            std::cout << "latent numerical issue: m_latent[i][j] = " << m_latent[i][j] << std::endl;
        }
        m_sum[j] += m_latent[i][j];
        }


      /*
      sum = 0;
      for (int j = 0; j < m_numOfGaussian; j++)
        {
        m_tmp1[j] = m_weight[j] * m_pdf[i][j];
        sum += m_tmp1[j];
        }
      if(sum == 0)
        {
        for (int j = 0; j < m_numOfGaussian; j++)
          {
          m_latent[i][j] = 1.0 / m_numOfGaussian;
          m_sum[j] += m_latent[i][j];
          }
        }
      else
        {
        for (int j = 0; j < m_numOfGaussian; j++)
          {
          m_latent[i][j] = m_tmp1[j] / sum;
          m_sum[j] += m_latent[i][j];
          }
        }
        */
      }
//    }
//  else
//    {
//    /**= THIS IS NOT USED
//    for (int i = 0; i < m_numOfData; i++)
//      {
//      sum = 0;
//      for (int j = 0; j < m_numOfGaussian; j++)
//        {
//        m_tmp1[j] = m_prior[i][j] * m_pdf[i][j];
//        sum += m_tmp1[j];
//        }
//      for (int j = 0; j < m_numOfGaussian; j++)
//        {
//        m_latent[i][j] = m_tmp1[j] / sum;
//        m_sum[j] += m_latent[i][j];
//        }
//      }
//      */
//    }
}

void EMGaussianMixtures::UpdateMean(void)
{
  for (int i = 0; i < m_numOfGaussian; i++)
    {
    for (int j = 0; j < m_dimOfGaussian; j++)
      {
      m_tmp2[j] = 0;
      }
    
    for (int j = 0; j < m_numOfData; j++)
      {
      for (int k = 0; k < m_dimOfGaussian; k++)
        {
//          // Hack here
//          if ((m_x[j][1] <= 132.308) && (k == 2))
//            continue;
        m_tmp2[k] += m_latent[j][i] * m_x[j][k];
        }
      }

    for (int j = 0; j < m_dimOfGaussian; j++)
      {
      m_tmp2[j] = m_tmp2[j] / m_sum[i];
      }

    m_gmm->SetMean(i, m_tmp2);
    }
}

void EMGaussianMixtures::UpdateCovariance(void)
{
  double *current_mean;
  for (int i = 0; i < m_numOfGaussian; i++)
    {
    current_mean = m_gmm->GetMean(i);
    
    for (int j = 0; j < m_dimOfGaussian; j++)
      {
      for (int k = 0; k < m_dimOfGaussian; k++)
        {
        m_tmp3[j*m_dimOfGaussian+k] = 0;
        }
      }
    
    for (int j = 0; j < m_numOfData; j++)
      {
      for (int k = 0; k < m_dimOfGaussian; k++)
        {
        m_tmp2[k] = m_x[j][k] - current_mean[k];
        }
      for (int k = 0; k < m_dimOfGaussian; k++)
        {
        for (int l = 0; l < m_dimOfGaussian; l++)
          {
          m_tmp3[k*m_dimOfGaussian+l] += m_tmp2[k] * m_tmp2[l] * m_latent[j][i];
          }
        }
      }

    for (int j = 0; j < m_dimOfGaussian*m_dimOfGaussian; j++)
      {
      m_tmp3[j] = m_tmp3[j] / m_sum[i];
      }

    m_gmm->SetCovariance(i, m_tmp3);
    }
}

void EMGaussianMixtures::UpdateWeight(void)
{
  for (int i = 0; i < m_numOfGaussian; i++)
    {
    m_gmm->SetWeight(i, m_sum[i]/m_numOfData);
    }
}

double EMGaussianMixtures::EvaluateLogLikelihood(void)
{
  double tmp1 = 0;
  double tmp2 = 0;
  if (m_setPriorFlag == 0)
    {
    for (int i = 0; i < m_numOfData; i++)
      {
      tmp1 = 0;
      for (int j = 0; j < m_numOfGaussian; j++)
        {
        if(!m_gmm->GetGaussian(j)->isDeltaFunction())
          tmp1 += m_weight[j] * exp(m_log_pdf[i][j]);
        }
      tmp2 += log(tmp1);
      }
    }
  else
    {
    for (int i = 0; i < m_numOfData; i++)
      {
      tmp1 = 0;
      for (int j = 0; j < m_numOfGaussian; j++)
        {
        if(!m_gmm->GetGaussian(j)->isDeltaFunction())
          tmp1 += m_prior[i][j] * exp(m_log_pdf[i][j]);
        }
      tmp2 += log(tmp1);
      }
    }
  return tmp1;
}

void EMGaussianMixtures::PrintParameters(void)
{
  m_gmm->PrintParameters();
}
