/*=============================================================================
  Copyright (C) 2012-2022 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        BoolFeature.cpp

  Description: Implementation of class VmbCPP::BoolFeature.
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

#include "BoolFeature.h"

#include <VmbC/VmbC.h>

#include <VmbCPP/FeatureContainer.h>

namespace VmbCPP {

BoolFeature::BoolFeature( const VmbFeatureInfo_t& featureInfo, FeatureContainer& featureContainer )
    :   BaseFeature( featureInfo, featureContainer )
{}

VmbErrorType BoolFeature::GetValue( bool &rbValue ) const noexcept
{
    if (nullptr == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    return (VmbErrorType)VmbFeatureBoolGet( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), &rbValue );
}

VmbErrorType BoolFeature::SetValue( bool bValue ) noexcept
{
    if (nullptr == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    return (VmbErrorType)VmbFeatureBoolSet( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), bValue );
}

}  // namespace VmbCPP
