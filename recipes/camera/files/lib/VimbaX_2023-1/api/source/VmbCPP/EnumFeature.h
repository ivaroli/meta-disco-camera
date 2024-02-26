/*=============================================================================
  Copyright (C) 2012-2022 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        EnumFeature.h

  Description: Definition of class VmbCPP::EnumFeature.
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

#ifndef VMBCPP_ENUMFEATURE_H
#define VMBCPP_ENUMFEATURE_H

/**
* \file        EnumFeature.h
*
* \brief       Definition of class VmbCPP::EnumFeature.
*              Intended for use in the implementation of VmbCPP.
*/

#include <VmbC/VmbCTypeDefinitions.h>

#include <VmbCPP/VmbCPPCommon.h>

#include "BaseFeature.h"


namespace VmbCPP {

class EnumEntry;

class EnumFeature final : public BaseFeature 
{
  public:
    EnumFeature( const VmbFeatureInfo_t& featureInfo, FeatureContainer& featureContainer );

    virtual VmbErrorType SetValue( const char *pValue ) noexcept override;
    
    virtual VmbErrorType GetEntry( EnumEntry &entry, const char *pEntryName ) const noexcept override;

    virtual VmbErrorType GetValue( VmbInt64_t &value ) const noexcept override;
    virtual VmbErrorType SetValue(VmbInt64_t value) noexcept override;

    virtual VmbErrorType IsValueAvailable( const char *pStrValue, bool &available ) const noexcept override;
    virtual VmbErrorType IsValueAvailable( const VmbInt64_t value, bool &available ) const noexcept override;

protected:
    // Array functions to pass data across DLL boundaries
    VmbErrorType GetValues(const char** pValues, VmbUint32_t& size) noexcept override;
    VmbErrorType GetValues(VmbInt64_t* pValue, VmbUint32_t& size) noexcept override;
    VmbErrorType GetEntries(EnumEntry* pEntries, VmbUint32_t& size) noexcept override;
    VmbErrorType GetValue( char * const pValue, VmbUint32_t &size ) const noexcept override;

private:
    // Copy of enum elements
    StringVector    m_EnumStringValues;
    Int64Vector     m_EnumIntValues;
    EnumEntryVector m_EnumEntries;

};

}  // namespace VmbCPP

#endif
