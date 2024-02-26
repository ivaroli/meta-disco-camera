/*=============================================================================
  Copyright (C) 2012-2022 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        BaseFeature.h

  Description: Definition of base class VmbCPP::BaseFeature.

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

#ifndef VMBCPP_BASEFEATURE_H
#define VMBCPP_BASEFEATURE_H

/**
* \file     BaseFeature.h
*
* \brief    Definition of base class VmbCPP::BaseFeature.
*/

#include <VmbC/VmbCTypeDefinitions.h>

#include <VmbCPP/BasicLockable.h>
#include <VmbCPP/SharedPointerDefines.h>
#include <VmbCPP/UniquePointer.hpp>
#include <VmbCPP/VmbCPPCommon.h>

struct VmbFeatureInfo;

namespace VmbCPP {

class EnumEntry;
class Feature;
class FeatureContainer;

class BaseFeature : protected BasicLockable
{
    friend class Feature;

public:
    BaseFeature(const VmbFeatureInfo& featureInfo, FeatureContainer & featureContainer );

    /**
     * \brief object is not default constructible 
     */
    BaseFeature() = delete;

    /**
     * object is not copyable
     */
    BaseFeature(const BaseFeature&) = delete;

    /**
     * object is not copyable
     */
    BaseFeature& operator=(const BaseFeature&) = delete;

    virtual ~BaseFeature();

    virtual    VmbErrorType GetValue( VmbInt64_t &value ) const noexcept;
    virtual    VmbErrorType GetValue( double &value ) const noexcept;
    virtual    VmbErrorType GetValue( bool &value ) const noexcept;

    virtual    VmbErrorType SetValue( VmbInt64_t value ) noexcept;
    virtual    VmbErrorType SetValue( double value ) noexcept;
    virtual    VmbErrorType SetValue( const char *pValue ) noexcept;
    virtual    VmbErrorType SetValue( bool value ) noexcept;

    virtual    VmbErrorType GetEntry( EnumEntry &entry, const char * pStrEntryName ) const noexcept;

    virtual    VmbErrorType GetRange( VmbInt64_t &minimum, VmbInt64_t &maximum ) const noexcept;
    virtual    VmbErrorType GetRange( double &minimum, double &maximum ) const noexcept;

    virtual    VmbErrorType HasIncrement( VmbBool_t &incrementSupported) const noexcept;
    virtual    VmbErrorType GetIncrement( VmbInt64_t &increment ) const noexcept;
    virtual    VmbErrorType GetIncrement( double &increment ) const noexcept;

    virtual    VmbErrorType IsValueAvailable( const char *pValue, bool &available ) const noexcept;
    virtual    VmbErrorType IsValueAvailable( const VmbInt64_t value, bool &available ) const noexcept;

    virtual    VmbErrorType RunCommand() noexcept;
    virtual    VmbErrorType IsCommandDone( bool &isDone ) const noexcept;

               VmbErrorType GetDataType( VmbFeatureDataType &dataType ) const noexcept;
               VmbErrorType GetFlags( VmbFeatureFlagsType &flags ) const noexcept;
               VmbErrorType GetPollingTime( VmbUint32_t &pollingTime ) const noexcept;
               VmbErrorType GetVisibility( VmbFeatureVisibilityType &visibility ) const noexcept;
               VmbErrorType IsReadable( bool &isReadable ) noexcept;
               VmbErrorType IsWritable( bool &isWritable ) noexcept;
               VmbErrorType IsStreamable( bool &isStreamable ) const noexcept;

               VmbErrorType RegisterObserver( const IFeatureObserverPtr &observer );
               VmbErrorType UnregisterObserver( const IFeatureObserverPtr &observer );

    void ResetFeatureContainer();

  protected:
    /**
    * \brief  Copy of feature infos.
    */
    struct FeatureInfo
    {
        /**
        * \name FeatureInfo
        * \{
        */
        std::string                     name;                   //!< Verbose name
        VmbFeatureData_t                featureDataType;        //!< Data type of this feature
        VmbFeatureFlags_t               featureFlags;           //!< Access flags for this feature
        bool                            hasSelectedFeatures;    //!< true if the feature selects other features
        std::string                     category;               //!< Category this feature can be found in
        std::string                     displayName;            //!< Feature name to be used in GUIs
        VmbUint32_t                     pollingTime;            //!< Predefined polling time for volatile features
        std::string                     unit;                   //!< Measuring unit as given in the XML file
        std::string                     representation;         //!< Representation of a numeric feature
        VmbFeatureVisibility_t          visibility;             //!< GUI visibility
        std::string                     tooltip;                //!< Short description
        std::string                     description;            //!< Longer description
        std::string                     sfncNamespace;          //!< Namespace this feature resides in
        bool                            isStreamable;           //!< Feature can be stored or loaded from/into a file
        
        /**
        * \}
        */
    };

    FeatureInfo m_featureInfo;

    FeatureContainer *m_pFeatureContainer;

    // Array functions to pass data across DLL boundaries
    virtual    VmbErrorType GetEntries(EnumEntry* pEntries, VmbUint32_t& size) noexcept;
    virtual    VmbErrorType GetValues(const char** pValues, VmbUint32_t& size) noexcept;
    virtual    VmbErrorType GetValues(VmbInt64_t* pValues, VmbUint32_t& Size) noexcept;
    virtual    VmbErrorType GetValue(char* const pValue, VmbUint32_t& length) const noexcept;
    virtual    VmbErrorType GetValue(VmbUchar_t* pValue, VmbUint32_t& size, VmbUint32_t& sizeFilled) const noexcept;
    virtual    VmbErrorType SetValue(const VmbUchar_t* pValue, VmbUint32_t size) noexcept;
    virtual    VmbErrorType GetValidValueSet(VmbInt64_t* buffer, VmbUint32_t bufferSize, VmbUint32_t* setSize) const noexcept;

private:
    struct Impl;
    UniquePointer<Impl> m_pImpl;

    VmbErrorType GetName( char * const pName, VmbUint32_t &length ) const noexcept;
    VmbErrorType GetDisplayName( char * const pDisplayName, VmbUint32_t &length ) const noexcept;
    VmbErrorType GetCategory( char * const pCategory, VmbUint32_t &length ) const noexcept;
    VmbErrorType GetUnit( char * const pUnit, VmbUint32_t &length ) const noexcept;
    VmbErrorType GetRepresentation( char * const pRepresentation, VmbUint32_t &length ) const noexcept;
    VmbErrorType GetToolTip( char * const pToolTip, VmbUint32_t &length ) const noexcept;
    VmbErrorType GetDescription( char * const pDescription, VmbUint32_t &length ) const noexcept;
    VmbErrorType GetSFNCNamespace( char * const pSFNCNamespace, VmbUint32_t &length ) const noexcept;
    VmbErrorType GetSelectedFeatures( FeaturePtr *pSelectedFeatures, VmbUint32_t &nSize ) noexcept;
};

}  // namespace VmbCPP

#endif
