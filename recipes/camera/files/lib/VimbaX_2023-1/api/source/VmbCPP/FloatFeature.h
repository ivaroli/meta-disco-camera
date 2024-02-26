/*=============================================================================
  Copyright (C) 2012-2022 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        FloatFeature.h

  Description: Definition of class VmbCPP::FloatFeature.
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

#ifndef VMBCPP_FLOATFEATURE_H
#define VMBCPP_FLOATFEATURE_H

/**
* \file        FloatFeature.h
*
* \brief       Definition of class VmbCPP::FloatFeature.
*              Intended for use in the implementation of VmbCPP.
*/

#include "BaseFeature.h"

struct VmbFeatureInfo;

namespace VmbCPP {

class FeatureContainer;

class FloatFeature final : public BaseFeature 
{
public:
    FloatFeature( const VmbFeatureInfo& featureInfo, FeatureContainer& featureContainer );

    virtual VmbErrorType GetValue( double &value ) const noexcept override;

    virtual VmbErrorType SetValue(double rfValue) noexcept override;

    virtual VmbErrorType GetRange( double &minimum, double &maximum ) const noexcept override;
    
    virtual VmbErrorType HasIncrement( VmbBool_t &incrementSupported) const noexcept override;

    virtual VmbErrorType GetIncrement( double &increment ) const noexcept override;
};

}  // namespace VmbCPP

#endif
