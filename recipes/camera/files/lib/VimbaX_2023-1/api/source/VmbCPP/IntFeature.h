/*=============================================================================
  Copyright (C) 2012-2022 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        IntFeature.h

  Description: Definition of class VmbCPP::IntFeature.
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

#ifndef VMBCPP_INTFEATURE_H
#define VMBCPP_INTFEATURE_H

/**
* \file        IntFeature.h
*
* \brief       Definition of class VmbCPP::IntFeature.
*              Intended for use in the implementation of VmbCPP.
*/

#include "BaseFeature.h"

struct VmbFeatureInfo;

namespace VmbCPP {

class FeatureContainer;

class IntFeature final : public BaseFeature 
{
public:
    IntFeature(const VmbFeatureInfo& featureInfo, FeatureContainer& featureContainer);

    virtual VmbErrorType GetValue( VmbInt64_t &value ) const noexcept override;

    virtual VmbErrorType SetValue(VmbInt64_t value) noexcept override;

    virtual VmbErrorType GetRange( VmbInt64_t &minimum, VmbInt64_t &maximum ) const noexcept override;

    virtual VmbErrorType HasIncrement( VmbBool_t &incrementSupported) const noexcept override;

    virtual VmbErrorType GetIncrement( VmbInt64_t &increment ) const noexcept override;
private:
    // Array function to pass data across DLL boundaries
    virtual VmbErrorType GetValidValueSet(VmbInt64_t* buffer, VmbUint32_t bufferSize, VmbUint32_t* setSize) const noexcept override;
};

}  // namespace VmbCPP

#endif
