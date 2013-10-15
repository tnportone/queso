//-----------------------------------------------------------------------bl-
//--------------------------------------------------------------------------
//
// QUESO - a library to support the Quantification of Uncertainty
// for Estimation, Simulation and Optimization
//
// Copyright (C) 2008,2009,2010,2011,2012,2013 The PECOS Development Team
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the Version 2.1 GNU Lesser General
// Public License as published by the Free Software Foundation.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc. 51 Franklin Street, Fifth Floor,
// Boston, MA  02110-1301  USA
//
//-----------------------------------------------------------------------el-
// 
// $Id$
//
//--------------------------------------------------------------------------

#include <queso/BetaJointPdf.h>

namespace QUESO {

// Constructor -------------------------------------
template<class V,class M>
BetaJointPdf<V,M>::BetaJointPdf(
  const char*                  prefix,
  const VectorSet<V,M>& domainSet,
  const V&                     alpha,
  const V&                     beta)
  :
  BaseJointPdf<V,M>(((std::string)(prefix)+"uni").c_str(),domainSet),
  m_alpha(alpha),
  m_beta (beta)
{
  if ((m_env.subDisplayFile()) && (m_env.displayVerbosity() >= 54)) {
    *m_env.subDisplayFile() << "Entering BetaJointPdf<V,M>::constructor()"
                            << ": prefix = " << m_prefix
                            << std::endl;
  }

  if ((m_env.subDisplayFile()) && (m_env.displayVerbosity() >= 54)) {
    *m_env.subDisplayFile() << "Leaving BetaJointPdf<V,M>::constructor()"
                            << ": prefix = " << m_prefix
                            << std::endl;
  }
}
// Destructor --------------------------------------
template<class V,class M>
BetaJointPdf<V,M>::~BetaJointPdf()
{
}
// Math methods-------------------------------------
template<class V, class M>
double
BetaJointPdf<V,M>::actualValue(
  const V& domainVector,
  const V* domainDirection,
        V* gradVector,
        M* hessianMatrix,
        V* hessianEffect) const
{
  UQ_FATAL_TEST_MACRO(domainVector.sizeLocal() != this->m_domainSet.vectorSpace().dimLocal(),
                      m_env.worldRank(),
                      "BetaJointPdf<V,M>::actualValue()",
                      "invalid input");

  UQ_FATAL_TEST_MACRO((domainDirection || gradVector || hessianMatrix || hessianEffect),
                      m_env.worldRank(),
                      "BetaJointPdf<V,M>::actualValue()",
                      "incomplete code for gradVector, hessianMatrix and hessianEffect calculations");

  // No need to multiply by exp(m_logOfNormalizationFactor) because 'lnValue()' is called [PDF-05]
  return exp(this->lnValue(domainVector,domainDirection,gradVector,hessianMatrix,hessianEffect));
}
//--------------------------------------------------
template<class V, class M>
double
BetaJointPdf<V,M>::lnValue(
  const V& domainVector,
  const V* domainDirection,
        V* gradVector,
        M* hessianMatrix,
        V* hessianEffect) const
{
  UQ_FATAL_TEST_MACRO((domainDirection || gradVector || hessianMatrix || hessianEffect),
                      m_env.worldRank(),
                      "BetaJointPdf<V,M>::lnValue()",
                      "incomplete code for gradVector, hessianMatrix and hessianEffect calculations");

  double aux = 0.;
  double result = 0.;
  for (unsigned int i = 0; i < domainVector.sizeLocal(); ++i) {
    if (m_normalizationStyle == 0) {
      //aux = log(gsl_ran_beta_pdf(domainVector[i],m_alpha[i],m_beta[i]));
      aux = log(m_env.basicPdfs()->betaPdfActualValue(domainVector[i],m_alpha[i],m_beta[i]));
    }
    else {
      aux = (m_alpha[i]-1.)*log(domainVector[i]) + (m_beta[i]-1.)*log(1.-domainVector[i]);
    }
    if ((m_env.subDisplayFile()) && (m_env.displayVerbosity() >= 99)) {
      *m_env.subDisplayFile() << "In BetaJointPdf<V,M>::lnValue()"
                              << ", m_normalizationStyle = "      << m_normalizationStyle
                              << ": domainVector[" << i << "] = " << domainVector[i]
                              << ", m_alpha[" << i << "] = "      << m_alpha[i]
                              << ", m_beta[" << i << "] = "       << m_beta[i]
                              << ", log(pdf)= "                   << aux
                              << std::endl;
    }
    result += aux;
  }
  result += m_logOfNormalizationFactor; // [PDF-05]

  return result;
}
//--------------------------------------------------
template<class V, class M>
double
BetaJointPdf<V,M>::computeLogOfNormalizationFactor(unsigned int numSamples, bool updateFactorInternally) const
{
  double value = 0.;

  if ((m_env.subDisplayFile()) && (m_env.displayVerbosity() >= 2)) {
    *m_env.subDisplayFile() << "Entering BetaJointPdf<V,M>::computeLogOfNormalizationFactor()"
                            << std::endl;
  }
  value = BaseJointPdf<V,M>::commonComputeLogOfNormalizationFactor(numSamples, updateFactorInternally);
  if ((m_env.subDisplayFile()) && (m_env.displayVerbosity() >= 2)) {
    *m_env.subDisplayFile() << "Leaving BetaJointPdf<V,M>::computeLogOfNormalizationFactor()"
                            << ", m_logOfNormalizationFactor = " << m_logOfNormalizationFactor
                            << std::endl;
  }

  return value;
}

}  // End namespace QUESO