/*--------------------------------------------------------------------------
 *--------------------------------------------------------------------------
 *
 * Copyright (C) 2008,2009 The PECOS Development Team
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
 *
 * $Id$
 *
 * Basic API: Class member functions.
 * 
 *--------------------------------------------------------------------------
 *-------------------------------------------------------------------------- */

using namespace std;

#include <basic_classes.h>
#include <basic_int.h>
#include <hpct.h>

namespace QUESO_Basic_API {

  void   QUESO_fatal        (const char *message);
  double Likelihood_Wrapper (const basicV &,const basicV *,const void *,basicV *,basicM *,basicV *);

  //------------------
  // Member Functions
  //------------------

  QUESO_Basic_Class::QUESO_Basic_Class()
  {
    m_initialized           = 0;		
    m_silent                = 0; 		
    m_paramSpace            = NULL;
    m_paramDomain           = NULL;
    m_user_likelihood_func  = NULL;
  }

  void QUESO_Basic_Class::Initialize(const char *inputfile)
  {

    // Define new QUESO environment

    m_env         = new uqFullEnvironmentClass(MPI_COMM_WORLD,inputfile,"");
    m_initialized = 1;

    m_inputfile = new string(inputfile);

    //    memcpy(m_inputfile,inputfile,strlen()
}

  void QUESO_Basic_Class:: DefineParameterSpace()
  {
    int     num_params;		// # of UQ parameters 
    double *param_min;		// min value of each parameter
    double *param_max;		// max value of each parameter
    double *param_ini;		// initial value of each parameter

    int     ierr = 1;

    VerifyInit();

    // Verify presence of required input file

    ierr *= hpct_input_fopen(m_inputfile->c_str());

    if(ierr == 0)
      QUESO_fatal("Unable to access QUESO input file");

    // Derive the UQ parameters from input file and create QUESO parameter vector

    ierr *= hpct_input_fread_int("queso/parameters/num_params",&m_num_params);

    if(ierr == 0)
      QUESO_fatal("Unable to read num_params from input file.");
    
    printf("--> Setup parameter space variables/ranges...\n");
    m_paramSpace  = new uqVectorSpaceClass <basicV,basicM> (*m_env,"queso_basic_",m_num_params,NULL);
    
    param_min = (double *) calloc(m_num_params,sizeof(double));
    param_max = (double *) calloc(m_num_params,sizeof(double));
    param_ini = (double *) calloc(m_num_params,sizeof(double));
  
    if(param_min == NULL || param_max == NULL || param_ini == NULL)
      QUESO_fatal("Unable to allocate memory for desired parameter space");
  
    ierr *= hpct_input_fread_double_vec("queso/parameters/param_mins",  param_min, m_num_params);
    ierr *= hpct_input_fread_double_vec("queso/parameters/param_maxs",  param_max, m_num_params);
    ierr *= hpct_input_fread_double_vec("queso/parameters/param_inits", param_ini, m_num_params);
  
    if(ierr == 0)
      QUESO_fatal("Unable to read parameter min/max/init values");

    m_queso_var_min = new basicV ( m_paramSpace->zeroVector() );
    m_queso_var_max = new basicV ( m_paramSpace->zeroVector() );
    m_queso_var_ini = new basicV ( m_paramSpace->zeroVector() );
    
    for(int i = 0;i<m_num_params;i++)
      {
	(*m_queso_var_min)[i] = param_min[i];
	(*m_queso_var_max)[i] = param_max[i];
	(*m_queso_var_ini)[i] = param_ini[i];
      }
    
    printf("--> Setup parameter search box...\n");
    m_paramDomain = new uqBoxSubsetClass<basicV, basicM> ("queso_basic_",*m_paramSpace,
							  *m_queso_var_min,*m_queso_var_max);
    // Clean up.

    free(param_min);
    free(param_max);
    free(param_ini);

    hpct_input_fclose();

  }

  void QUESO_Basic_Class:: VerifyInit()
  {
    if(!m_initialized)
      QUESO_fatal("QUESO not initialized prior to use");
  }

  void QUESO_Basic_Class::Likelihood_Register(double (*fp)(double *) )
  {

    VerifyInit();

    printf("--> Setting prior and post vectors...\n");

    m_priorRV = new uqUniformVectorRVClass <basicV, basicM> ("prior_",*m_paramDomain);
    m_postRV  = new uqGenericVectorRVClass <basicV, basicM> ("post_" ,*m_paramSpace );
    
    m_likelihoodObj = new uqGenericScalarFunctionClass 
      <basicV,basicM> ("like_",*m_paramDomain,Likelihood_Wrapper,NULL,true);
    
    m_user_likelihood_func = fp;
    
    // define the inverse (calibration) problem
    
    printf("--> Defining inverse problem...\n");
    
    m_ip = new uqStatisticalInverseProblemClass <basicV,basicM> ("",*m_priorRV,*m_likelihoodObj,*m_postRV);
    
    // Default covariance matrix for now - default assumption assumes
    // 6-sigma distribution range falls over 1/3 of the max parameter range.
    
    double cov_param = 1/(3.*6.);	// 6-sigma over 1/3 of the range
    double param_range = 0.0;
    
    printf("--> Defining default covariance matrix...\n");
    
    m_CovMatrix = m_postRV->imageSet().vectorSpace().newGaussianMatrix(m_priorRV->pdf().domainVarVector(),
								       *m_queso_var_ini);

    for(int i=0;i<m_num_params;i++)
      {
	param_range = (*m_queso_var_max)[i] - (*m_queso_var_min)[i];
	(*m_CovMatrix)(i,i) = (cov_param*param_range)*(cov_param*param_range);
      }
    
  }

  void QUESO_Basic_Class::SolveInverseProblem()
  {
    VerifyInit();  

    // Launch the Markov Chain 

    m_ip->solveWithBayesMarkovChain(*m_queso_var_ini,m_CovMatrix);
  }

  //----------------------------------------------
  // Fatal error(): example only - to be replaced.
  //----------------------------------------------

  void QUESO_fatal(const char *message)
  {
    printf("%s\n",message);
    exit(1);

    // koomie note: likely need mpi_abort and better exception handling.
  }

  //---------------------------------------------------------------------
  // Wrapper for user likelihood routine: culls data from QUESO uqVector
  // and passes to the user routine as an array of doubles.
  //---------------------------------------------------------------------

  double Likelihood_Wrapper(const basicV &paramValue,
			    const basicV *paramDirection,
			    const void *Data,basicV *gradV,basicM *hesianM,
			    basicV *hessianE)
  {
    static int first_entry = 1;
    double user_func_return;
    double likelihood;
    double *uqParams;
    int num_params;

    num_params = paramValue.sizeGlobal();

    if(first_entry)
      {
	if(num_params < 1)
	  QUESO_fatal("Invalid number of parameters");

	if(_QUESO_Basic->m_user_likelihood_func  == NULL )
	  QUESO_fatal("Invalid user-supplied likelihood function");

	uqParams = (double *)calloc(num_params,sizeof(double));
	if(uqParams == NULL)
	  QUESO_fatal("Unable to allocate emmory for uqParams");
      }
	
    for(int i=0;i<num_params;i++)
      uqParams[i] = paramValue[i];
    
      return( _QUESO_Basic->m_user_likelihood_func(uqParams) );
      
  }

}   //  QUESO_Basic_API namespace
