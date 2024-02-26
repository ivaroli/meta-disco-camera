/*=============================================================================
  Copyright (C) 2012-2022 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        FloatFeature.cpp

  Description: Implementation of class VmbCPP::FloatFeature.
               Intended for use in the implementation of VmbCPP.

-------------------------------------------------------------------------------

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
  NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
  DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, 
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED  
  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

#include "FloatFeature.h"

#include <VmbC/VmbC.h>

#include <VmbCPP/FeatureContainer.h>

namespace VmbCPP {

FloatFeature::FloatFeature( const VmbFeatureInfo& featureInfo, FeatureContainer& featureContainer )
    :   BaseFeature( featureInfo, featureContainer )
{
}

VmbErrorType FloatFeature::GetValue( double &rfValue ) const noexcept
{
    if ( nullptr == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    return static_cast<VmbErrorType>(VmbFeatureFloatGet(m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), &rfValue));
}

VmbErrorType FloatFeature::SetValue( const double rfValue ) noexcept
{
    if ( nullptr == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    return static_cast<VmbErrorType>(VmbFeatureFloatSet(m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), rfValue));
}

VmbErrorType FloatFeature::GetRange( double &rfMinimum, double &rfMaximum ) const noexcept
{
    if ( nullptr == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    return static_cast<VmbErrorType>(VmbFeatureFloatRangeQuery(m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), &rfMinimum, &rfMaximum));
}

VmbErrorType FloatFeature::HasIncrement( VmbBool_t &incrementSupported ) const noexcept
{
    if ( nullptr == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }
    VmbBool_t hasIncrement;
    VmbError_t result = VmbFeatureFloatIncrementQuery(m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(),&hasIncrement, nullptr);
    if( VmbErrorSuccess == result)
    {
        incrementSupported = hasIncrement;
        return VmbErrorSuccess;
    }
    return static_cast<VmbErrorType>(result);
}

VmbErrorType FloatFeature::GetIncrement( double &rnIncrement ) const noexcept
{
    if (nullptr == m_pFeatureContainer)
    {
        return VmbErrorDeviceNotOpen;
    }
    VmbBool_t hasIncrement;
    VmbError_t result = VmbFeatureFloatIncrementQuery(m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(),&hasIncrement, &rnIncrement);
    if( VmbErrorSuccess == result && !hasIncrement)
    {
        return VmbErrorNotAvailable;
    }
    return static_cast<VmbErrorType>(result);
}

}  // namespace VmbCPP
