/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        EnumFeature.cpp

  Description: Implementation of class VmbCPP::EnumFeature.
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

#include "EnumFeature.h"

#include <cstring>
#include <new>

#include <VmbC/VmbC.h>

#include <VmbCPP/EnumEntry.h>
#include <VmbCPP/FeatureContainer.h>

namespace VmbCPP {

EnumFeature::EnumFeature( const VmbFeatureInfo_t& featureInfo, FeatureContainer& featureContainer )
    :BaseFeature( featureInfo, featureContainer )
{
}

VmbErrorType EnumFeature::GetValue( char * const pStrValue, VmbUint32_t &rnSize ) const noexcept
{
    VmbErrorType res;
    if ( nullptr == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    const char* pStrTempValue;
    res = static_cast<VmbErrorType>(VmbFeatureEnumGet(m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), &pStrTempValue));

    if ( VmbErrorSuccess == res )
    {
        auto const nLength = static_cast<VmbUint32_t>(std::strlen(pStrTempValue));

        if ( nullptr == pStrValue )
        {
            rnSize = nLength;
        }
        else if ( nLength <= rnSize )
        {
            std::memcpy( pStrValue, pStrTempValue, nLength);
            rnSize = nLength;
        }
        else
        {
            res = VmbErrorMoreData;
        }
    }

    return res;
}

VmbErrorType EnumFeature::GetValue( VmbInt64_t &rnValue ) const noexcept
{
    if ( nullptr == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    const char *pName = nullptr;
    VmbError_t res = VmbFeatureEnumGet( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), &pName );
    if ( VmbErrorSuccess == res )
    {
        res = VmbFeatureEnumAsInt( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), pName, &rnValue );
    }

    return static_cast<VmbErrorType>(res);
}

VmbErrorType EnumFeature::GetEntry( EnumEntry &rEntry, const char * pStrEntryName ) const noexcept
{
    if ( nullptr == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    VmbFeatureEnumEntry_t entry;
    VmbError_t res = VmbFeatureEnumEntryGet( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), pStrEntryName, &entry, sizeof( VmbFeatureEnumEntry_t ));
    if ( VmbErrorSuccess == res )
    {
        try
        {
            rEntry = EnumEntry(entry.name, entry.displayName, entry.description, entry.tooltip, entry.sfncNamespace, entry.visibility, entry.intValue);
        }
        catch (...)
        {
            res = VmbErrorResources;
        }
    }

    return static_cast<VmbErrorType>(res);
}

VmbErrorType EnumFeature::SetValue( const char *pStrValue ) noexcept
{
    if ( nullptr == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    return (VmbErrorType)VmbFeatureEnumSet( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), pStrValue );
}

VmbErrorType EnumFeature::SetValue( VmbInt64_t rnValue ) noexcept
{
    if ( nullptr == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    const char *pName = nullptr;
    VmbError_t res = VmbFeatureEnumAsString( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), rnValue, &pName );
    if ( VmbErrorSuccess == res )
    {
        res = VmbFeatureEnumSet( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), pName );
    }

    return static_cast<VmbErrorType>(res);
}

VmbErrorType EnumFeature::GetValues(const char ** const pRange, VmbUint32_t &rnSize) noexcept
{
    if ( nullptr == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    VmbUint32_t nCount = 0;
    VmbError_t res = VmbFeatureEnumRangeQuery( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), nullptr, 0, &nCount );

    if (VmbErrorSuccess == res
        && 0 < nCount )
    {
        try
        {
            std::vector<const char*> data(nCount);

            res = VmbFeatureEnumRangeQuery(m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), &data[0], nCount, &nCount);
            data.resize(nCount);

            if (VmbErrorSuccess == res)
            {
                m_EnumStringValues.clear();
                m_EnumStringValues.reserve(data.size());

                for (std::vector<const char*>::iterator iter = data.begin();
                     data.end() != iter;
                     ++iter)
                {
                    m_EnumStringValues.emplace_back(*iter);
                }

                if (nullptr == pRange)
                {
                    rnSize = static_cast<VmbUint32_t>(m_EnumStringValues.size());
                    res = VmbErrorSuccess;
                }
                else if (m_EnumStringValues.size() <= rnSize)
                {
                    VmbUint32_t i = 0;
                    auto outPos = pRange;
                    for (StringVector::iterator iter = m_EnumStringValues.begin();
                         m_EnumStringValues.end() != iter;
                         ++iter, ++outPos)
                    {
                        *outPos = iter->c_str();
                    }
                    rnSize = static_cast<VmbUint32_t>(m_EnumStringValues.size());
                    res = VmbErrorSuccess;
                }
                else
                {
                    res = VmbErrorMoreData;
                }
            }
        }
        catch (std::bad_alloc const&)
        {
            return VmbErrorResources;
        }
    }

    return static_cast<VmbErrorType>(res);
}

VmbErrorType EnumFeature::GetValues( VmbInt64_t * const pValues, VmbUint32_t &rnSize ) noexcept
{
    if ( nullptr == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    VmbUint32_t nCount = 0;
    VmbError_t res = GetValues(static_cast<const char**>(nullptr), nCount);

    if (VmbErrorSuccess == res
        && 0 < nCount )
    {
        try
        {
            std::vector<const char*> data(nCount);

            res = GetValues(&data[0], nCount);

            if (VmbErrorSuccess == res)
            {
                m_EnumIntValues.clear();

                for (std::vector<const char*>::iterator iter = data.begin();
                     data.end() != iter;
                     ++iter)
                {
                    VmbInt64_t nValue;
                    res = VmbFeatureEnumAsInt(m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), (*iter), &nValue);

                    if (VmbErrorSuccess == res)
                    {
                        m_EnumIntValues.push_back(nValue);
                    }
                    else
                    {
                        m_EnumIntValues.clear();
                        break;
                    }
                }

                if (VmbErrorSuccess == res)
                {
                    if (nullptr == pValues)
                    {
                        rnSize = static_cast<VmbUint32_t>(m_EnumIntValues.size());
                    }
                    else if (m_EnumIntValues.size() <= rnSize)
                    {
                        auto outPos = pValues;
                        for (Int64Vector::iterator iter = m_EnumIntValues.begin();
                             m_EnumIntValues.end() != iter;
                             ++iter, ++outPos)
                        {
                            *outPos = (*iter);
                        }
                        rnSize = static_cast<VmbUint32_t>(m_EnumIntValues.size());
                    }
                    else
                    {
                        res = VmbErrorMoreData;
                    }
                }
            }
        }
        catch (std::bad_alloc const&)
        {
            return VmbErrorResources;
        }
    }

    return static_cast<VmbErrorType>(res);
}

VmbErrorType EnumFeature::GetEntries( EnumEntry *pEntries, VmbUint32_t &rnSize ) noexcept
{
    VmbErrorType res = GetValues( (const char**)nullptr, rnSize );

    if (VmbErrorSuccess == res && !m_EnumStringValues.empty())
    {
        m_EnumEntries.clear();

        try
        {
            m_EnumEntries.reserve(rnSize);
        }
        catch (...)
        {
            res = VmbErrorResources;
        }

        if (res == VmbErrorSuccess)
        {
            for (auto const& entryName : m_EnumStringValues)
            {
                EnumEntry entry;
                res = GetEntry(entry, entryName.c_str());
                if (VmbErrorSuccess == res)
                {
                    m_EnumEntries.emplace_back(std::move(entry));
                }
                else
                {
                    m_EnumEntries.clear();
                    break;
                }
            }

            if (VmbErrorSuccess == res)
            {
                if (nullptr == pEntries)
                {
                    rnSize = static_cast<VmbUint32_t>(m_EnumEntries.size());
                }
                else if (m_EnumEntries.size() <= rnSize)
                {
                    try
                    {
                        std::copy(m_EnumEntries.begin(), m_EnumEntries.end(), pEntries);
                        rnSize = static_cast<VmbUint32_t>(m_EnumEntries.size());
                    }
                    catch (...)
                    {
                        res = VmbErrorResources;
                        rnSize = 0;
                    }
                }
                else
                {
                    res = VmbErrorMoreData;
                }
            }
        }
    }

    return res;
}

VmbErrorType EnumFeature::IsValueAvailable( const char *pStrValue, bool &bAvailable ) const noexcept
{
    if ( nullptr == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    return static_cast<VmbErrorType>(VmbFeatureEnumIsAvailable(m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), pStrValue, &bAvailable));
}

VmbErrorType EnumFeature::IsValueAvailable( const VmbInt64_t nValue, bool &rbAvailable ) const noexcept
{
    if ( nullptr == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    const char* pName = nullptr;
    VmbError_t res = VmbFeatureEnumAsString( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), nValue, &pName );
    if ( VmbErrorSuccess == res )
    {
        res = IsValueAvailable( pName, rbAvailable );
    }

    return static_cast<VmbErrorType>(res);
}

}  // namespace VmbCPP

