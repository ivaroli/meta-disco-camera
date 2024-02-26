/*=============================================================================
  Copyright (C) 2012-2022 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        Feature.cpp

  Description: Implementation of wrapper class VmbCPP::Feature.

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

#include <VmbCPP/Feature.h>

#pragma warning(disable:4996)
#include "BaseFeature.h"
#pragma warning(default:4996)

#include "BoolFeature.h"
#include "CommandFeature.h"
#include "EnumFeature.h"
#include "FloatFeature.h"
#include "IntFeature.h"
#include "RawFeature.h"
#include "StringFeature.h"


namespace VmbCPP {

Feature::Feature( const VmbFeatureInfo& featureInfo, FeatureContainer& featureContainer )
{
    switch ( featureInfo.featureDataType )
    {
    case VmbFeatureDataBool:
        m_pImpl = new BoolFeature( featureInfo, featureContainer );
        break;
    case VmbFeatureDataEnum:
        m_pImpl = new EnumFeature( featureInfo, featureContainer );
        break;
    case VmbFeatureDataFloat:
        m_pImpl = new FloatFeature( featureInfo, featureContainer );
        break;
    case VmbFeatureDataInt:
        m_pImpl = new IntFeature( featureInfo, featureContainer );
        break;
    case VmbFeatureDataString:
        m_pImpl = new StringFeature( featureInfo, featureContainer );
        break;
    case VmbFeatureDataCommand:
        m_pImpl = new CommandFeature( featureInfo, featureContainer );
        break;
    case VmbFeatureDataRaw:
        m_pImpl = new RawFeature( featureInfo, featureContainer );
        break;
    default:
        m_pImpl = new BaseFeature( featureInfo, featureContainer );
    }
}

Feature::~Feature()
{
    delete m_pImpl;
}

void Feature::ResetFeatureContainer()
{
    m_pImpl->ResetFeatureContainer();
}

VmbErrorType Feature::RegisterObserver( const IFeatureObserverPtr &rObserver )
{
    return m_pImpl->RegisterObserver( rObserver );
}

VmbErrorType Feature::UnregisterObserver( const IFeatureObserverPtr &rObserver )
{
    return m_pImpl->UnregisterObserver( rObserver );
}

// Gets the value of a feature of type VmbFeatureDataInt
VmbErrorType Feature::GetValue( VmbInt64_t &rnValue ) const noexcept
{
    return m_pImpl->GetValue( rnValue );
}

// Sets the value of a feature of type VmbFeatureDataInt
VmbErrorType Feature::SetValue( VmbInt64_t rnValue ) noexcept
{
    return m_pImpl->SetValue( rnValue );
}

// Gets the range of a feature of type VmbFeatureDataInt
VmbErrorType Feature::GetRange( VmbInt64_t &rnMinimum, VmbInt64_t &rnMaximum ) const noexcept
{
    return m_pImpl->GetRange( rnMinimum, rnMaximum );
}

VmbErrorType Feature::HasIncrement( VmbBool_t &incrementSupported) const noexcept
{
    return m_pImpl->HasIncrement( incrementSupported);
}
// Gets the increment of a feature of type VmbFeatureDataInt
VmbErrorType Feature::GetIncrement( VmbInt64_t &rnIncrement ) const noexcept
{
    return m_pImpl->GetIncrement( rnIncrement );
}

// Gets the increment of a feature of type VmbFeatureDataFloat
VmbErrorType Feature::GetIncrement( double &rnIncrement ) const noexcept
{
    return m_pImpl->GetIncrement( rnIncrement );
}

// Gets the value of a feature of type VmbFeatureDataFloat
VmbErrorType Feature::GetValue( double &rfValue) const noexcept
{
    return m_pImpl->GetValue( rfValue );
}

// Sets the value of a feature of type VmbFeatureDataFloat
VmbErrorType Feature::SetValue( double rfValue ) noexcept
{
    return m_pImpl->SetValue( rfValue );
}

// Gets the range of a feature of type VmbFeatureDataFloat
VmbErrorType Feature::GetRange( double &rfMinimum, double &rfMaximum ) const noexcept
{
    return m_pImpl->GetRange( rfMinimum, rfMaximum );
}

// Sets the value of a feature of type VmbFeatureDataEnum
// Sets the value of a feature of type VmbFeatureDataString
VmbErrorType Feature::SetValue( const char *pStrValue ) noexcept
{
    return m_pImpl->SetValue( pStrValue );
}

// Gets all possible values as string of a feature of type VmbFeatureDataEnum
VmbErrorType Feature::GetValues( const char **pStrValues, VmbUint32_t &rnSize ) noexcept
{
    return m_pImpl->GetValues( pStrValues, rnSize );
}

// Gets all possible values as integer of a feature of type VmbFeatureDataEnum
VmbErrorType Feature::GetValues( VmbInt64_t *pnValues, VmbUint32_t &rnSize ) noexcept
{
    return m_pImpl->GetValues( pnValues, rnSize );
}

// Gets the currently selected enum entry of a feature of type VmbFeatureDataEnum
VmbErrorType Feature::GetEntry( EnumEntry &entry, const char *pStrEntryName ) const noexcept
{
    return m_pImpl->GetEntry( entry, pStrEntryName );
}

// Gets all possible enum entries of a feature of type VmbFeatureDataEnum
VmbErrorType Feature::GetEntries( EnumEntry *pEnumEntries, VmbUint32_t &rnSize ) noexcept
{
    return m_pImpl->GetEntries( pEnumEntries, rnSize );
}

// Indicates whether a particular enum value as string of a feature of type VmbFeatureDataEnum is available
VmbErrorType Feature::IsValueAvailable( const char *pStrValue, bool &rbAvailable ) const noexcept
{
    return m_pImpl->IsValueAvailable( pStrValue, rbAvailable );
}

// Indicates whether a particular enum value as integer of a feature of type VmbFeatureDataEnum is available
VmbErrorType Feature::IsValueAvailable( const VmbInt64_t nValue, bool &rbAvailable ) const noexcept
{
    return m_pImpl->IsValueAvailable( nValue, rbAvailable );
}

// Gets the value of a feature of type VmbFeatureDataString
VmbErrorType Feature::GetValue( char * const pStrValue, VmbUint32_t &rnLength ) const noexcept
{
    return m_pImpl->GetValue( pStrValue, rnLength );
}

// Gets the value of a feature of type VmbFeatureDataBool
VmbErrorType Feature::GetValue( bool &rbValue ) const noexcept
{
    return m_pImpl->GetValue( rbValue );
}

// Sets the value of a feature of type VmbFeatureDataBool
VmbErrorType Feature::SetValue( bool bValue ) noexcept
{
    return m_pImpl->SetValue( bValue );
}

// Executes a feature of type VmbFeatureDataCommand
VmbErrorType Feature::RunCommand() noexcept
{
    return m_pImpl->RunCommand();
}

// Indicates whether a feature of type VmbFeatureDataCommand finished execution
VmbErrorType Feature::IsCommandDone( bool &bIsDone ) const noexcept
{
    return m_pImpl->IsCommandDone( bIsDone );
}

// Gets the value of a feature of type VmbFeatureDataRaw
VmbErrorType Feature::GetValue( VmbUchar_t *pValue, VmbUint32_t &rnSize, VmbUint32_t &rnSizeFilled ) const noexcept
{
    return m_pImpl->GetValue( pValue, rnSize, rnSizeFilled );
}

// Sets the value of a feature of type VmbFeatureDataRaw
VmbErrorType Feature::SetValue( const VmbUchar_t *pValue, VmbUint32_t nSize ) noexcept
{
    return m_pImpl->SetValue( pValue, nSize );
}

VmbErrorType Feature::GetName( char * const pStrName, VmbUint32_t &rnLength ) const noexcept
{
    return m_pImpl->GetName( pStrName, rnLength );
}

VmbErrorType Feature::GetDisplayName( char * const pStrDisplayName, VmbUint32_t &rnLength ) const noexcept
{
    return m_pImpl->GetDisplayName( pStrDisplayName, rnLength );
}

VmbErrorType Feature::GetDataType( VmbFeatureDataType &reDataType ) const noexcept
{
    return m_pImpl->GetDataType( reDataType );
}

VmbErrorType Feature::GetFlags( VmbFeatureFlagsType &reFlags ) const noexcept
{
    return m_pImpl->GetFlags( reFlags );
}

VmbErrorType Feature::GetCategory( char * const pStrCategory, VmbUint32_t &rnLength ) const noexcept
{
    return m_pImpl->GetCategory( pStrCategory, rnLength );
}

VmbErrorType Feature::GetPollingTime( VmbUint32_t &rnPollingTime ) const noexcept
{
    return m_pImpl->GetPollingTime( rnPollingTime );
}

VmbErrorType Feature::GetUnit( char * const pStrUnit, VmbUint32_t &rnLength ) const noexcept
{
    return m_pImpl->GetUnit( pStrUnit, rnLength );
}

VmbErrorType Feature::GetRepresentation( char * const pStrRepresentation, VmbUint32_t &rnLength ) const noexcept
{
    return m_pImpl->GetRepresentation( pStrRepresentation, rnLength );
}

VmbErrorType Feature::GetVisibility( VmbFeatureVisibilityType &reVisibility ) const noexcept
{
    return m_pImpl->GetVisibility( reVisibility );
}

VmbErrorType Feature::GetToolTip( char * const pStrToolTip, VmbUint32_t &rnLength ) const noexcept
{
    return m_pImpl->GetToolTip( pStrToolTip, rnLength );
}

VmbErrorType Feature::GetDescription( char * const pStrDescription, VmbUint32_t &rnLength ) const noexcept
{
    return m_pImpl->GetDescription( pStrDescription, rnLength );
}

VmbErrorType Feature::GetSFNCNamespace( char * const pStrSFNCNamespace, VmbUint32_t &rnLength ) const noexcept
{
    return m_pImpl->GetSFNCNamespace( pStrSFNCNamespace, rnLength );
}

VmbErrorType Feature::GetSelectedFeatures( FeaturePtr *pSelectedFeatures, VmbUint32_t &rnSize ) noexcept
{
    return m_pImpl->GetSelectedFeatures( pSelectedFeatures, rnSize );
}

VmbErrorType Feature::IsReadable( bool &rbIsReadable ) noexcept
{
    return m_pImpl->IsReadable( rbIsReadable );
}

VmbErrorType Feature::IsWritable( bool &rbIsWritable ) noexcept
{
    return m_pImpl->IsWritable( rbIsWritable );
}

VmbErrorType Feature::IsStreamable( bool &rbIsStreamable ) const noexcept
{
    return m_pImpl->IsStreamable( rbIsStreamable );
}

VmbErrorType Feature::GetValidValueSet(VmbInt64_t* buffer, VmbUint32_t bufferSize, VmbUint32_t* setSize) const noexcept
{
    return m_pImpl->GetValidValueSet(buffer, bufferSize, setSize);
}


}  // namespace VmbCPP
