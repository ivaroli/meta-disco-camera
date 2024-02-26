/*=============================================================================
  Copyright (C) 2012-2022 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        BoolFeature.h

  Description: Definition of class VmbCPP::BoolFeature.
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

#ifndef VMBCPP_BOOLFEATURE_H
#define VMBCPP_BOOLFEATURE_H

/**
* \file        BoolFeature.h
*
* \brief       Definition of class VmbCPP::BoolFeature.
*              Intended for use in the implementation of VmbCPP.
*/

#include <VmbC/VmbCTypeDefinitions.h>

#include "BaseFeature.h"


namespace VmbCPP {

class BoolFeature final : public BaseFeature
{
public:
    BoolFeature( const VmbFeatureInfo_t& featureInfo, FeatureContainer& pFeatureContainer );

    /**
    *  \brief                Get the value of a boolean feature
    *
    * \param[out]  value     Value of type 'bool'
    *
    * \returns ::VmbErrorType
    * \retval ::VmbErrorSuccess     If no error
    * \retval ::VmbErrorWrongType   Feature is not a bool feature
    * \retval VmbInternalError:     Value could not get queried
    */ 
    virtual VmbErrorType GetValue( bool &value ) const noexcept override;

    /**
    * \brief      Set the value of a boolean feature
    */ 
    virtual VmbErrorType SetValue( bool value ) noexcept override;
};

}  // namespace VmbCPP

#endif
