/*=============================================================================
  Copyright (C) 2012-2022 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        BaseFeature.cpp

  Description: Implementation of base class VmbCPP::BaseFeature.

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

#include <new>
#include <utility>

#pragma warning(disable:4996)
#include "BaseFeature.h"
#pragma warning(default:4996)

#include "ConditionHelper.h"
#include "CopyUtils.hpp"
#include "Helper.h"

#include <VmbC/VmbC.h>

#include <VmbCPP/FeatureContainer.h>
#include <VmbCPP/VmbSystem.h>


namespace VmbCPP {

typedef std::vector<IFeatureObserverPtr> IFeatureObserverPtrVector;

struct BaseFeature::Impl
{
    LockableVector<IFeatureObserverPtr> m_observers;

    FeaturePtrVector m_affectedFeatures;
    FeaturePtrVector m_selectedFeatures;
    bool m_bAffectedFeaturesFetched;
    bool m_bSelectedFeaturesFetched;

    ConditionHelper m_observersConditionHelper;
    ConditionHelper m_conditionHelper;

    static void VMB_CALL InvalidationCallback( const VmbHandle_t handle, const char *name, void *context );
};

BaseFeature::BaseFeature( const VmbFeatureInfo& featureInfo, FeatureContainer& featureContainer )
    : m_pImpl(new Impl()),
    m_pFeatureContainer(&featureContainer)
{
    m_pImpl->m_bAffectedFeaturesFetched = false;
    m_pImpl->m_bSelectedFeaturesFetched = false;

    m_featureInfo.category.assign( featureInfo.category ? featureInfo.category : "" );
    m_featureInfo.description.assign( featureInfo.description ? featureInfo.description : "" );
    m_featureInfo.displayName.assign( featureInfo.displayName ? featureInfo.displayName : "" );
    m_featureInfo.featureDataType = featureInfo.featureDataType;
    m_featureInfo.featureFlags = featureInfo.featureFlags;
    m_featureInfo.hasSelectedFeatures = featureInfo.hasSelectedFeatures;
    m_featureInfo.name.assign( featureInfo.name ? featureInfo.name : "" );
    m_featureInfo.pollingTime = featureInfo.pollingTime;
    m_featureInfo.representation.assign( featureInfo.representation ? featureInfo.representation : "" );
    m_featureInfo.sfncNamespace.assign( featureInfo.sfncNamespace ? featureInfo.sfncNamespace : "" );
    m_featureInfo.tooltip.assign( featureInfo.tooltip ? featureInfo.tooltip : "" );
    m_featureInfo.unit.assign( featureInfo.unit ? featureInfo.unit : "" );
    m_featureInfo.visibility = featureInfo.visibility;
    m_featureInfo.isStreamable = featureInfo.isStreamable;
}

BaseFeature::~BaseFeature()
{
    // Before destruction we unregister all observers and all callbacks
    ResetFeatureContainer();
}

// Unregisters all observers before it resets the feature container pointer.
void BaseFeature::ResetFeatureContainer()
{
    if (nullptr != m_pFeatureContainer )
    {
        // Camera still open
        if (nullptr != m_pFeatureContainer->GetHandle() )
        {
            VmbFeatureInvalidationUnregister( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), &BaseFeature::Impl::InvalidationCallback );
        }

        // Begin exclusive write lock this feature
        if ( true == m_pImpl->m_conditionHelper.EnterWriteLock( GetMutex(), true ))
        {
            m_pFeatureContainer = nullptr;

            // End write lock this feature
            m_pImpl->m_conditionHelper.ExitWriteLock( GetMutex() );
        }
        else
        {
            LOG_FREE_TEXT( "Could not reset a feature's feature container reference. ");
        }
        
    }
    
    // Begin exclusive write lock observer list
    if ( true == m_pImpl->m_observersConditionHelper.EnterWriteLock( m_pImpl->m_observers, true ))
    {
        m_pImpl->m_observers.Vector.clear();
        
        // End write lock observer list
        m_pImpl->m_observersConditionHelper.ExitWriteLock( m_pImpl->m_observers );
    }
}

void VMB_CALL BaseFeature::Impl::InvalidationCallback( const VmbHandle_t handle, const char * /*name*/, void *context )
{
    BaseFeature *pFeature = (BaseFeature*)context;
    if (nullptr != pFeature )
    {
        if (nullptr != handle )
        {
            // Begin read lock this feature
            if ( true == pFeature->m_pImpl->m_conditionHelper.EnterReadLock( pFeature->GetMutex() ))
            {
                if (nullptr != pFeature->m_pFeatureContainer )
                {
                    FeaturePtr pFeaturePtrFromMap;
                    if ( VmbErrorSuccess == pFeature->m_pFeatureContainer->GetFeatureByName( pFeature->m_featureInfo.name.c_str(), pFeaturePtrFromMap ) )
                    {
                        // Begin read lock observer list
                        if ( true == pFeature->m_pImpl->m_observersConditionHelper.EnterReadLock( pFeature->m_pImpl->m_observers ))
                        {
                            for (   IFeatureObserverPtrVector::iterator iter = pFeature->m_pImpl->m_observers.Vector.begin();
                                    pFeature->m_pImpl->m_observers.Vector.end() != iter;
                                    ++iter)
                            {
                                SP_ACCESS(( *iter ))->FeatureChanged( pFeaturePtrFromMap );
                            }

                            // End read lock observer list
                            pFeature->m_pImpl->m_observersConditionHelper.ExitReadLock( pFeature->m_pImpl->m_observers );
                        }
                        else
                        {
                            LOG_FREE_TEXT( "Could not lock feature observer list.")
                        }
                    }
                    else // GetFeatureByName() failed
                    {
                        // Do some logging
                        LOG_FREE_TEXT( "GetFeatureByName failed" )
                    }
                }
                else // m_pFeatureContainer == null (Feature destroyed or device closed / destroyed)
                {
                    // Do some logging
                    LOG_FREE_TEXT( "Feature destroyed or device closed / destroyed" );
                }

                // End read lock this feature
                pFeature->m_pImpl->m_conditionHelper.ExitReadLock( pFeature->GetMutex() );
            }
            else
            {
                LOG_FREE_TEXT( "Could not lock feature.")
            }
        }
        else // m_handle == null (device closed / destroyed)
        {
            // Do some logging
            LOG_FREE_TEXT( "Device closed / destroyed" )
        }
    }
    else // pFeature == null (Just for safety)
    {
        // Do some logging
        LOG_FREE_TEXT( "Feature pointer is null" )
    }
}

VmbErrorType BaseFeature::RegisterObserver( const IFeatureObserverPtr &rObserver )
{
    if ( SP_ISNULL( rObserver ))
    {
        return VmbErrorBadParameter;
    }    

    if (nullptr == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    VmbError_t res = VmbErrorSuccess;

    // Begin write lock observer list
    if ( true == m_pImpl->m_observersConditionHelper.EnterWriteLock( m_pImpl->m_observers ))
    {
        // The very same observer cannot be registered twice
        for ( size_t i=0; i<m_pImpl->m_observers.Vector.size(); ++i )
        {
            if ( SP_ISEQUAL( rObserver, m_pImpl->m_observers.Vector[i] ))
            {
                res = VmbErrorAlready;
                break;
            }
        }

        if ( VmbErrorSuccess == res )
        {
            if ( 0 == m_pImpl->m_observers.Vector.size() )
            {
                res = VmbFeatureInvalidationRegister( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), m_pImpl->InvalidationCallback, this );
            }

            if ( VmbErrorSuccess == res )
            {
                m_pImpl->m_observers.Vector.push_back( rObserver );
            }
        }
        
        // End write lock observer list
        m_pImpl->m_observersConditionHelper.ExitWriteLock( m_pImpl->m_observers );
    }

    return (VmbErrorType)res;
}

VmbErrorType BaseFeature::UnregisterObserver( const IFeatureObserverPtr &rObserver )
{
    if ( SP_ISNULL( rObserver ))
    {
        return VmbErrorBadParameter;
    }    

    if (nullptr == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    VmbError_t res = VmbErrorNotFound;

    // Begin exclusive write lock observer list
    if ( true == m_pImpl->m_observersConditionHelper.EnterWriteLock( m_pImpl->m_observers, true ))
    {
        for (   IFeatureObserverPtrVector::iterator iter = m_pImpl->m_observers.Vector.begin();
                m_pImpl->m_observers.Vector.end() != iter;)
        {
            if ( SP_ISEQUAL( rObserver, *iter ))
            {
                // If we are about to unregister the last observer we cancel all invalidation notifications
                if ( 1 == m_pImpl->m_observers.Vector.size() )
                {
                    res = VmbFeatureInvalidationUnregister( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), m_pImpl->InvalidationCallback );
                }
                if (    VmbErrorSuccess == res
                     || 1 < m_pImpl->m_observers.Vector.size() )
                {
                    iter = m_pImpl->m_observers.Vector.erase( iter );
                    res = VmbErrorSuccess;
                }
                break;
            }
            else
            {
                ++iter;
            }
        }

        // End write lock observer list
        m_pImpl->m_observersConditionHelper.ExitWriteLock( m_pImpl->m_observers );
    }
    else
    {
        LOG_FREE_TEXT( "Could not lock feature observer list.")
        res = VmbErrorInternalFault;
    }
    
    return (VmbErrorType)res;
}

// Gets the value of a feature of type VmbFeatureDataInt
VmbErrorType BaseFeature::GetValue( VmbInt64_t & /*rnValue*/ ) const noexcept
{
    return VmbErrorWrongType;
}

// Sets the value of a feature of type VmbFeatureDataInt
VmbErrorType BaseFeature::SetValue( VmbInt64_t /*rnValue*/ ) noexcept
{
    return VmbErrorWrongType;
}

// Gets the range of a feature of type VmbFeatureDataInt
VmbErrorType BaseFeature::GetRange( VmbInt64_t & /*rnMinimum*/, VmbInt64_t & /*rnMaximum*/ ) const noexcept
{
    return VmbErrorWrongType;
}

VmbErrorType BaseFeature::HasIncrement( VmbBool_t & /*incrementSupported*/) const noexcept
{
    return VmbErrorWrongType;
}

// Gets the increment of a feature of type VmbFeatureDataInt
VmbErrorType BaseFeature::GetIncrement( VmbInt64_t & /*rnIncrement*/ ) const noexcept
{
    return VmbErrorWrongType;
}

// Gets the increment of a feature of type VmbFeatureDataFloat
VmbErrorType BaseFeature::GetIncrement( double & /*rnIncrement*/ ) const noexcept
{
    return VmbErrorWrongType;
}

// Gets the value of a feature of type VmbFeatureDataFloat
VmbErrorType BaseFeature::GetValue( double & /*rfValue*/) const noexcept
{
    return VmbErrorWrongType;
}

// Sets the value of a feature of type VmbFeatureDataFloat
VmbErrorType BaseFeature::SetValue( double /*rfValue*/ ) noexcept
{
    return VmbErrorWrongType;
}

// Gets the range of a feature of type VmbFeatureDataFloat
VmbErrorType BaseFeature::GetRange( double & /*rfMinimum*/, double & /*rfMaximum*/ ) const noexcept
{
    return VmbErrorWrongType;
}

// Sets the value of a feature of type VmbFeatureDataEnum
// Sets the value of a feature of type VmbFeatureDataString
VmbErrorType BaseFeature::SetValue( const char * /*pStrValue*/ ) noexcept
{
    return VmbErrorWrongType;
}

// Gets the enum entry of a feature of type VmbFeatureDataEnum
VmbErrorType BaseFeature::GetEntry( EnumEntry & /*entry*/, const char * /*pStrEntryName*/ ) const noexcept
{
    return VmbErrorWrongType;
}

// Gets all possible enum entries of a feature of type VmbFeatureDataEnum
VmbErrorType BaseFeature::GetEntries( EnumEntry * /*pEnumEntries*/, VmbUint32_t & /*size*/ ) noexcept
{
    return VmbErrorWrongType;
}

// Gets all possible values as string of a feature of type VmbFeatureDataEnum
VmbErrorType BaseFeature::GetValues( const char ** /*pStrValues*/, VmbUint32_t & /*rnSize*/ ) noexcept
{
    return VmbErrorWrongType;
}

// Gets all possible values as integer of a feature of type VmbFeatureDataEnum
VmbErrorType BaseFeature::GetValues( VmbInt64_t * /*pnValues*/, VmbUint32_t & /*rnSize*/ ) noexcept
{
    return VmbErrorWrongType;
}

// Indicates whether a particular enum value as string of a feature of type VmbFeatureDataEnum is available
VmbErrorType BaseFeature::IsValueAvailable( const char * /*pStrValue*/, bool & /*bAvailable*/ ) const noexcept
{
    return VmbErrorWrongType;
}

// Indicates whether a particular enum value as integer of a feature of type VmbFeatureDataEnum is available
VmbErrorType BaseFeature::IsValueAvailable( const VmbInt64_t /*nValue*/, bool & /*bAvailable*/ ) const noexcept
{
    return VmbErrorWrongType;
}

// Gets the value of a feature of type VmbFeatureDataString
// Gets the value of a feature of type VmbFeatureDataEnum
VmbErrorType BaseFeature::GetValue( char * const /*pStrValue*/, VmbUint32_t & /*length*/ ) const noexcept
{
    return VmbErrorWrongType;
}

// Gets the value of a feature of type VmbFeatureDataBool
VmbErrorType BaseFeature::GetValue( bool & /*rbValue*/ ) const noexcept
{
    return VmbErrorWrongType;
}

// Sets the value of a feature of type VmbFeatureDataBool
VmbErrorType BaseFeature::SetValue( bool /*bValue*/ ) noexcept
{
    return VmbErrorWrongType;
}

// Executes a feature of type VmbFeatureDataCommand
VmbErrorType BaseFeature::RunCommand() noexcept
{
    return VmbErrorWrongType;
}

// Indicates whether a feature of type VmbFeatureDataCommand finished execution
VmbErrorType BaseFeature::IsCommandDone( bool & /*bIsDone*/ ) const noexcept
{
    return VmbErrorWrongType;
}

// Gets the value of a feature of type VmbFeatureDataRaw
VmbErrorType BaseFeature::GetValue( VmbUchar_t * /*pValue*/, VmbUint32_t & /*rnSize*/, VmbUint32_t & /*rnSizeFilled*/ ) const noexcept
{
    return VmbErrorWrongType;
}

// Sets the value of a feature of type VmbFeatureDataRaw
VmbErrorType BaseFeature::SetValue( const VmbUchar_t * /*pValue*/, VmbUint32_t /*nSize*/ ) noexcept
{
    return VmbErrorWrongType;
}

VmbErrorType BaseFeature::GetName( char * const pStrName, VmbUint32_t &rnLength ) const noexcept
{
    return CopyToBuffer(m_featureInfo.name, pStrName, rnLength);
}

VmbErrorType BaseFeature::GetDisplayName( char * const pStrDisplayName, VmbUint32_t &rnLength ) const noexcept
{
    return CopyToBuffer(m_featureInfo.displayName, pStrDisplayName, rnLength);
}

VmbErrorType BaseFeature::GetDataType( VmbFeatureDataType &reDataType ) const noexcept
{
    reDataType = static_cast<VmbFeatureDataType>(m_featureInfo.featureDataType);
    
    return VmbErrorSuccess;
}

VmbErrorType BaseFeature::GetFlags( VmbFeatureFlagsType &reFlags ) const noexcept
{
    reFlags = static_cast<VmbFeatureFlagsType>(m_featureInfo.featureFlags);
        
    return VmbErrorSuccess;
}

VmbErrorType BaseFeature::GetCategory( char * const pStrCategory, VmbUint32_t &rnLength ) const noexcept
{
    return CopyToBuffer(m_featureInfo.category, pStrCategory, rnLength);
}

VmbErrorType BaseFeature::GetPollingTime( VmbUint32_t &rnPollingTime ) const noexcept
{
    rnPollingTime = m_featureInfo.pollingTime;
    
    return VmbErrorSuccess;
}

VmbErrorType BaseFeature::GetUnit( char * const pStrUnit, VmbUint32_t &rnLength ) const noexcept
{
    return CopyToBuffer(m_featureInfo.unit, pStrUnit, rnLength);
}

VmbErrorType BaseFeature::GetRepresentation( char * const pStrRepresentation, VmbUint32_t &rnLength ) const noexcept
{
    return CopyToBuffer(m_featureInfo.representation, pStrRepresentation, rnLength);
}

VmbErrorType BaseFeature::GetVisibility( VmbFeatureVisibilityType &reVisibility ) const noexcept
{
    reVisibility = static_cast<VmbFeatureVisibilityType>(m_featureInfo.visibility);

    return VmbErrorSuccess;
}

VmbErrorType BaseFeature::GetToolTip( char * const pStrToolTip, VmbUint32_t &rnLength ) const noexcept
{
    return CopyToBuffer(m_featureInfo.tooltip, pStrToolTip, rnLength);
}

VmbErrorType BaseFeature::GetDescription( char * const pStrDescription, VmbUint32_t &rnLength ) const noexcept
{
    return CopyToBuffer(m_featureInfo.description, pStrDescription, rnLength);
}

VmbErrorType BaseFeature::GetSFNCNamespace( char * const pStrSFNCNamespace, VmbUint32_t &rnLength ) const noexcept
{
    return CopyToBuffer(m_featureInfo.sfncNamespace, pStrSFNCNamespace, rnLength);
}

VmbErrorType BaseFeature::GetSelectedFeatures( FeaturePtr *pSelectedFeatures, VmbUint32_t &rnSize ) noexcept
{
    VmbError_t res;

    if (nullptr == pSelectedFeatures)
    {
        // Selected features were fetched before
        if (m_pImpl->m_bSelectedFeaturesFetched)
        {
            rnSize = static_cast<VmbUint32_t>(m_pImpl->m_selectedFeatures.size());

            res = VmbErrorSuccess;
        }
        // Selected features have not been fetched before
        else
        {
            return static_cast<VmbErrorType>(VmbFeatureListSelected(
                m_pFeatureContainer->GetHandle(),
                m_featureInfo.name.c_str(),
                nullptr,
                0,
                &rnSize,
                sizeof(VmbFeatureInfo_t)));
        }
    }
    else
    {
        // Selected features were fetched before
        if (m_pImpl->m_bSelectedFeaturesFetched)
        {
            if ( rnSize < m_pImpl->m_selectedFeatures.size() )
            {
                return VmbErrorMoreData;
            }

            rnSize = static_cast<VmbUint32_t>(m_pImpl->m_selectedFeatures.size());

            try
            {
                std::copy(m_pImpl->m_selectedFeatures.begin(), m_pImpl->m_selectedFeatures.end(), pSelectedFeatures);
                res = VmbErrorSuccess;
            }
            catch (std::bad_alloc const&)
            {
                res = VmbErrorResources;
            }
        }
        // Selected features have not been fetched before
        else
        {
            // Check whether the given array size fits
            VmbUint32_t nSize = 0;
            res = VmbFeatureListSelected( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), nullptr, 0, &nSize, sizeof(VmbFeatureInfo_t) );

            m_pImpl->m_bSelectedFeaturesFetched = true;

            if ( rnSize < nSize )
            {
                return VmbErrorMoreData;
            }

            rnSize = static_cast<VmbUint32_t>(nSize);

            if (VmbErrorSuccess != res
                || 0 == rnSize)
            {
                return static_cast<VmbErrorType>(res);
            }

            // Fetch selected features and store them as well as hand them out
            std::vector<VmbFeatureInfo_t> selectedFeatureInfos;
            try
            {
                selectedFeatureInfos.resize(rnSize);

                res = VmbFeatureListSelected(m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), &selectedFeatureInfos[0], (VmbUint32_t)selectedFeatureInfos.size(), &nSize, sizeof(VmbFeatureInfo_t));

                if (rnSize < nSize)
                {
                    return VmbErrorMoreData;
                }

                rnSize = (VmbUint32_t)nSize;

                for (VmbUint32_t i = 0; i < rnSize; ++i)
                {
                    FeaturePtr pFeature;
                    res = m_pFeatureContainer->GetFeatureByName(selectedFeatureInfos[i].name, pFeature);
                    if (VmbErrorSuccess != res)
                    {
                        m_pImpl->m_selectedFeatures.clear();
                        return static_cast<VmbErrorType>(res);
                    }
                    m_pImpl->m_selectedFeatures.emplace_back(std::move(pFeature));
                    pSelectedFeatures[i] = m_pImpl->m_selectedFeatures[i];
                }
            }
            catch (std::bad_alloc const&)
            {
                res = VmbErrorResources;
            }
        }
    }

    return static_cast<VmbErrorType>(res);
}

VmbErrorType BaseFeature::IsReadable( bool &rbIsReadable ) noexcept
{
    bool bIsWritable = false;

    if (nullptr == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }
    
    return (VmbErrorType)VmbFeatureAccessQuery( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), &rbIsReadable, &bIsWritable );
}

VmbErrorType BaseFeature::IsWritable( bool &rbIsWritable ) noexcept
{
    bool bIsReadable = false;

    if (nullptr == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    return (VmbErrorType)VmbFeatureAccessQuery( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), &bIsReadable, &rbIsWritable );
}

VmbErrorType BaseFeature::IsStreamable( bool &rbIsStreamable ) const noexcept
{
    rbIsStreamable = m_featureInfo.isStreamable;

    return VmbErrorSuccess;
}

VmbErrorType BaseFeature::GetValidValueSet(VmbInt64_t* buffer, VmbUint32_t bufferSize, VmbUint32_t* setSize) const noexcept
{
    return VmbErrorWrongType;
}

}  // namespace VmbCPP
