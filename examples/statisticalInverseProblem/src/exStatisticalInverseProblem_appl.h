/*--------------------------------------------------------------------------
 *--------------------------------------------------------------------------
 *
 * Copyright (C) 2008 The PECOS Development Team
 *
 * Please see http://pecos.ices.utexas.edu for more information.
 *
 * This file is part of the QUESO Library (Quantification of Uncertainty
 * for Estimation, Simulation and Optimization).
 *
 * QUESO is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * QUESO is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QUESO. If not, see <http://www.gnu.org/licenses/>.
 *
 *--------------------------------------------------------------------------
 *-------------------------------------------------------------------------- */

#ifndef EX_STATISTICAL_INVERSE_PROBLEM_APPL_H
#define EX_STATISTICAL_INVERSE_PROBLEM_APPL_H

#include <exStatisticalInverseProblem_likelihood.h>
#include <queso/StatisticalInverseProblem.h>
#include <queso/CovCond.h>

//********************************************************
// The driving routine: called by main()
//********************************************************
template<class P_V,class P_M>
void 
uqAppl(const QUESO::BaseEnvironment& env)
{
  if (env.fullRank() == 0) {
    std::cout << "Beginning run of 'exStatisticalInverseProblem_example'\n"
              << std::endl;
  }

  //******************************************************
  // Step 1 of 5: Instantiate the parameter space
  // It has dimension equal to 4
  //******************************************************
  if (env.fullRank() == 0) {
    std::cout << "Executing step 1 of 5: instantiation of parameter space ...\n"
              << std::endl;
  }

  QUESO::VectorSpace<P_V,P_M> paramSpace(env,"param_",4,NULL);

  //******************************************************
  // Step 2 of 5: Instantiate the parameter domain
  //******************************************************
  if (env.fullRank() == 0) {
    std::cout << "Executing step 2 of 5: instantiation of parameter domain ...\n"
              << std::endl;
  }

  P_V paramMins(paramSpace.zeroVector());
  paramMins.cwSet(-INFINITY);
  P_V paramMaxs(paramSpace.zeroVector());
  paramMaxs.cwSet( INFINITY);
  QUESO::BoxSubset<P_V,P_M> paramDomain("param_",paramSpace,paramMins,paramMaxs);

  //******************************************************
  // Step 3 of 5: Instantiate the likelihood function object (data + routine), to be used by QUESO.
  //******************************************************
  if (env.fullRank() == 0) {
    std::cout << "Executing step 3 of 5: instantiation of likelihood function object ...\n"
              << std::endl;
  }

  P_V paramMeans(paramSpace.zeroVector());

  double condNumber = 100.0;
  P_V direction(paramSpace.zeroVector());
  direction.cwSet(1.);
  P_M* covMatrixInverse = paramSpace.newMatrix();
  P_M* covMatrix        = paramSpace.newMatrix();
  QUESO::CovCond(condNumber,direction,*covMatrix,*covMatrixInverse);

  Likelihood likelihood("like_", paramDomain);

  likelihood.paramMeans = &paramMeans;
  likelihood.matrix = covMatrixInverse;
  likelihood.applyMatrixInvert = false;

  //******************************************************
  // Step 4 of 5: Instantiate the inverse problem
  //******************************************************
  if (env.fullRank() == 0) {
    std::cout << "Executing step 4 of 5: instantiation of inverse problem ...\n"
              << std::endl;
  }

  QUESO::UniformVectorRV<P_V,P_M> priorRv("prior_", // Extra prefix before the default "rv_" prefix
                                          paramDomain);

  QUESO::GenericVectorRV<P_V,P_M> postRv("post_", // Extra prefix before the default "rv_" prefix
                                         paramSpace);

  QUESO::StatisticalInverseProblem<P_V,P_M> ip("", // No extra prefix before the default "ip_" prefix
                                               NULL,
                                               priorRv,
                                               likelihoodFunctionObj,
                                               postRv);

  //******************************************************
  // Step 5 of 5: Solve the inverse problem
  //******************************************************
  if (env.fullRank() == 0) {
    std::cout << "Executing step 5 of 5: solution of inverse problem ...\n"
              << std::endl;
  }

  //******************************************************
  // According to options in the input file 'sip.inp', the following output files
  // will be created during the solution of the inverse problem:
  // --> ...
  //******************************************************
  P_V paramInitials(paramSpace.zeroVector());
  P_M* proposalCovMatrix = postRv.imageSet().vectorSpace().newProposalMatrix(NULL,&paramInitials);
  ip.solveWithBayesMetropolisHastings(NULL,
                                      paramInitials,
                                      proposalCovMatrix);
  delete proposalCovMatrix;

  //******************************************************
  // Write data to disk, to be used by 'sip_plot.m' afterwards
  //******************************************************
  if (env.fullRank() == 0) {
    std::cout << "Inverse problem solved. Writing data to disk now ...\n"
              << std::endl;
  }

  char varPrefixName[64+1];
  std::set<unsigned int> auxSet;
  auxSet.insert(0);

  sprintf(varPrefixName,"sip_appl_paramMeans");
  paramMeans.subWriteContents(varPrefixName,
                              "outputData/appl_output",
                              UQ_FILE_EXTENSION_FOR_MATLAB_FORMAT,
                              auxSet);
  sprintf(varPrefixName,"sip_appl_covMatrix");
  covMatrix->subWriteContents(varPrefixName,
                              "outputData/appl_output",
                              UQ_FILE_EXTENSION_FOR_MATLAB_FORMAT,
                              auxSet);
  sprintf(varPrefixName,"sip_appl_covMatrixInverse");
  covMatrixInverse->subWriteContents(varPrefixName,
                                     "outputData/appl_output",
                                     UQ_FILE_EXTENSION_FOR_MATLAB_FORMAT,
                                     auxSet);
  //std::cout << "covMatrix = [" << *covMatrix
  //          << "];"
  //          << "\n"
  //          << "covMatrixInverse = [" << *covMatrixInverse
  //          << "];"
  //          << std::endl;

  //******************************************************
  // Write weighted squared norm to disk, to be used by 'sip_plot.m' afterwards
  //******************************************************
  // Define auxVec
  const QUESO::BaseVectorRealizer<P_V,P_M>& postRealizer = postRv.realizer();
  QUESO::VectorSpace<P_V,P_M> auxSpace(env,"",postRealizer.subPeriod(),NULL);
  P_V auxVec(auxSpace.zeroVector());

  // Populate auxVec
  P_V tmpVec (paramSpace.zeroVector());
  P_V diffVec(paramSpace.zeroVector());
  for (unsigned int i = 0; i < auxSpace.dimLocal(); ++i) {
    postRealizer.realization(tmpVec);
    diffVec = tmpVec - paramMeans;
    auxVec[i] = scalarProduct(diffVec, *covMatrixInverse * diffVec);
  }

  // Write auxVec to disk
  sprintf(varPrefixName,"sip_appl_d");
  auxVec.subWriteContents(varPrefixName,
                          "outputData/appl_output",
                          UQ_FILE_EXTENSION_FOR_MATLAB_FORMAT,
                          auxSet);

  //******************************************************
  // Release memory before leaving routine.
  //******************************************************
  delete covMatrixInverse;
  delete covMatrix;

  if (env.fullRank() == 0) {
    std::cout << "Finishing run of 'exStatisticalInverseProblem_example'"
              << std::endl;
  }

  return;
}
#endif // EX_STATISTICAL_INVERSE_PROBLEM_APPL_H
