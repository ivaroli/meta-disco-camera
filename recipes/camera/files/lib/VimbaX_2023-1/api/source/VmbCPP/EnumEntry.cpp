/*=============================================================================
  Copyright (C) 2012-2022 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        EnumEntry.cpp

  Description: Implementation of class VmbCPP::EnumEntry.

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

#include <VmbCPP/EnumEntry.h>

#include "CopyUtils.hpp"

namespace VmbCPP {

struct EnumEntry::PrivateImpl
{
    std::string                 m_strName;
    std::string                 m_strDisplayName;
    std::string                 m_strDescription;
    std::string                 m_strTooltip;
    std::string                 m_strNamespace;
    VmbFeatureVisibilityType    m_Visibility;
    VmbInt64_t                  m_nValue;

    PrivateImpl(    const char              *pStrName,
                    const char              *pStrDisplayName,
                    const char              *pStrDescription,
                    const char              *pStrTooltip,
                    const char              *pStrSNFCNamespace,
                    VmbFeatureVisibility_t  visibility,
                    VmbInt64_t              nValue)
        : m_strName(pStrName != nullptr ? pStrName : ""),
        m_strDisplayName(pStrDisplayName != nullptr ? pStrDisplayName : ""),
        m_strDescription(pStrDescription != nullptr ? pStrDescription : ""),
        m_strTooltip(pStrTooltip != nullptr ? pStrTooltip : ""),
        m_strNamespace(pStrTooltip != nullptr ? pStrTooltip : ""),
        m_Visibility(static_cast<VmbFeatureVisibilityType>(visibility)),
        m_nValue( nValue )
    {
    }

    VmbErrorType GetName( char * const pStrName, VmbUint32_t &rnSize ) const noexcept
    {
        return CopyToBuffer(m_strName, pStrName, rnSize);
    }

    VmbErrorType GetDisplayName( char * const pStrDisplayName, VmbUint32_t &rnSize ) const noexcept
    {
        return CopyToBuffer(m_strDisplayName, pStrDisplayName, rnSize);
    }

    VmbErrorType GetDescription( char * const pStrDescription, VmbUint32_t &rnSize ) const noexcept
    {
        return CopyToBuffer(m_strDescription, pStrDescription, rnSize);
    }

    VmbErrorType GetTooltip( char * const pStrTooltip, VmbUint32_t &rnSize ) const noexcept
    {
        return CopyToBuffer(m_strTooltip, pStrTooltip, rnSize);
    }

    VmbErrorType GetSFNCNamespace( char * const pStrNamespace, VmbUint32_t &rnSize ) const noexcept
    {
        return CopyToBuffer(m_strNamespace, pStrNamespace, rnSize);
    }

    VmbErrorType GetValue( VmbInt64_t &rnValue ) const noexcept
    {
        rnValue = m_nValue;

        return VmbErrorSuccess;
    }

    VmbErrorType GetVisibility( VmbFeatureVisibilityType &rVisibility ) const noexcept
    {
        rVisibility = m_Visibility;

        return VmbErrorSuccess;
    }


};

EnumEntry::EnumEntry(   const char              *pStrName,
                        const char              *pStrDisplayName,
                        const char              *pStrDescription,
                        const char              *pStrTooltip,
                        const char              *pStrSNFCNamespace,
                        VmbFeatureVisibility_t  visibility,
                        VmbInt64_t              nValue)
    : m_pImpl( new PrivateImpl(pStrName, pStrDisplayName, pStrDescription, pStrTooltip, pStrSNFCNamespace, visibility, nValue) )
{
}

EnumEntry::EnumEntry(const EnumEntry& other)
    : m_pImpl(other.m_pImpl == nullptr ? nullptr : new PrivateImpl(*other.m_pImpl))
{
}

EnumEntry& EnumEntry::operator=(const EnumEntry& other)
{
    if (this != &other)
    {
        m_pImpl.reset(
            (other.m_pImpl == nullptr)
            ? nullptr
            : new PrivateImpl(*other.m_pImpl));
    }
    return *this;
}

EnumEntry::EnumEntry()
    : m_pImpl(nullptr)
{
}

EnumEntry::~EnumEntry() noexcept
{
}

VmbErrorType EnumEntry::GetName( char * const pStrName, VmbUint32_t &rnSize ) const noexcept
{
    if( nullptr == m_pImpl )
    {
        return VmbErrorInternalFault;
    }
    return m_pImpl->GetName( pStrName, rnSize );
}

VmbErrorType EnumEntry::GetDisplayName( char * const pStrDisplayName, VmbUint32_t &rnSize ) const noexcept
{
    if( nullptr == m_pImpl )
    {
        return VmbErrorInternalFault;
    }
    return m_pImpl->GetDisplayName( pStrDisplayName, rnSize);
}

VmbErrorType EnumEntry::GetDescription( char * const pStrDescription, VmbUint32_t &rnSize ) const noexcept
{
    if( nullptr == m_pImpl )
    {
        return VmbErrorInternalFault;
    }
    return m_pImpl->GetDescription( pStrDescription, rnSize);
}

VmbErrorType EnumEntry::GetTooltip( char * const pStrTooltip, VmbUint32_t &rnSize ) const noexcept
{
    if( nullptr == m_pImpl )
    {
        return VmbErrorInternalFault;
    }
    return  m_pImpl->GetTooltip( pStrTooltip, rnSize );
}

VmbErrorType EnumEntry::GetSFNCNamespace( char * const pStrNamespace, VmbUint32_t &rnSize ) const noexcept
{
    if( nullptr == m_pImpl )
    {
        return VmbErrorInternalFault;
    }
    return m_pImpl->GetSFNCNamespace( pStrNamespace, rnSize);
}

VmbErrorType EnumEntry::GetValue( VmbInt64_t &rnValue ) const noexcept
{
    if( nullptr == m_pImpl )
    {
        return VmbErrorInternalFault;
    }
    rnValue = m_pImpl->m_nValue;

    return VmbErrorSuccess;
}

VmbErrorType EnumEntry::GetVisibility( VmbFeatureVisibilityType &rVisibility ) const noexcept
{
    if( nullptr ==  m_pImpl )
    {
        return VmbErrorInternalFault;
    }
    rVisibility = m_pImpl->m_Visibility;

    return VmbErrorSuccess;
}


}  // namespace VmbCPP
