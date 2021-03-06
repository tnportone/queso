//-----------------------------------------------------------------------bl-
//--------------------------------------------------------------------------
//
// QUESO - a library to support the Quantification of Uncertainty
// for Estimation, Simulation and Optimization
//
// Copyright (C) 2008-2017 The PECOS Development Team
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

#include <limits>
#include <queso/WignerVectorRealizer.h>
#include <queso/GslVector.h>
#include <queso/GslMatrix.h>

namespace QUESO {

// Constructor -------------------------------------
template<class V, class M>
WignerVectorRealizer<V,M>::WignerVectorRealizer(
  const char*                  prefix,
  const VectorSet<V,M>& unifiedImageSet,
  const V&                     centerPos,
  double                       radius)
  :
  BaseVectorRealizer<V,M>(((std::string)(prefix)+"gen").c_str(),unifiedImageSet,std::numeric_limits<unsigned int>::max()),
  m_centerPos(new V(centerPos)),
  m_radius   (radius)
{
  if ((m_env.subDisplayFile()) && (m_env.displayVerbosity() >= 5)) {
    *m_env.subDisplayFile() << "Entering WignerVectorRealizer<V,M>::constructor()"
                            << ": prefix = " << m_prefix
                            << std::endl;
  }

  queso_require_greater_msg(m_radius, 0., "invalid radius");

  if ((m_env.subDisplayFile()) && (m_env.displayVerbosity() >= 5)) {
    *m_env.subDisplayFile() << "Leaving WignerVectorRealizer<V,M>::constructor()"
                            << ": prefix = " << m_prefix
                            << std::endl;
  }
}
// Destructor --------------------------------------
template<class V, class M>
WignerVectorRealizer<V,M>::~WignerVectorRealizer()
{
  delete m_centerPos;
}
// -------------------------------------------------
// TODO: implement me, please!!!!
template<class V, class M>
void
WignerVectorRealizer<V,M>::realization(V& nextValues) const
{
  queso_not_implemented();

  nextValues.cwSet(0.);
  return;
}

}  // End namespace QUESO

template class QUESO::WignerVectorRealizer<QUESO::GslVector, QUESO::GslMatrix>;
