/*=============================================================================
  Copyright (C) 2012-2022 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        RawFeature.cpp

  Description: Implementation of class VmbCPP::RawFeature.
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

#include "RawFeature.h"

#include <VmbC/VmbC.h>

#include <VmbCPP/FeatureContainer.h>

namespace VmbCPP {

RawFeature::RawFeature(const VmbFeatureInfo& featureInfo, FeatureContainer& featureContainer)
    : BaseFeature(featureInfo, featureContainer)
{
}

VmbErrorType RawFeature::GetValue(VmbUchar_t *pValue, VmbUint32_t &rnSize, VmbUint32_t &rnSizeFilled) const noexcept
{
    VmbError_t res;
    VmbUint32_t nSize;

    if ( nullptr == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }
    
    res = VmbFeatureRawLengthQuery( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), &nSize );

    if ( nullptr != pValue )
    {
        if ( rnSize < nSize )
        {
            return VmbErrorMoreData;
        }

        if ( VmbErrorSuccess == res )
        {
            res = VmbFeatureRawGet( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), (char*)pValue, rnSize, &rnSizeFilled );
        }
    }
    else
    {
        rnSize = nSize;
    }

    return static_cast<VmbErrorType>(res);
}

VmbErrorType RawFeature::SetValue(const VmbUchar_t *pValue, VmbUint32_t nSize) noexcept
{
    if ( nullptr == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    if ( nullptr == pValue )
    {
        return VmbErrorBadParameter;
    }

    return static_cast<VmbErrorType>(VmbFeatureRawSet(m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), reinterpret_cast<const char*>(pValue), nSize));
}

}  // namespace VmbCPP
