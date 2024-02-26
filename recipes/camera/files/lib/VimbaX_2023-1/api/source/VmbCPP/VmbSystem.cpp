/*=============================================================================
  Copyright (C) 2012-2022 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------
 
  File:        VmbSystem.cpp

  Description: Implementation of class VmbCPP::VmbSystem.

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

#include <algorithm>
#include <cstring>
#include <functional>
#include <new>
#include <string>
#include <vector>

#include <VmbCPP/VmbSystem.h>

#include <VmbCPP/SharedPointerDefines.h>

#include "ConditionHelper.h"
#include "CopyUtils.hpp"
#include "Clock.h"
#include "DefaultCameraFactory.h"
#include "Helper.h"
#include "Version.h"


namespace VmbCPP {

typedef std::map<std::string, CameraPtr> CameraPtrMap;
typedef std::map<VmbHandle_t, InterfacePtr> InterfacePtrMap;
typedef std::map<VmbHandle_t, TransportLayerPtr> TransportLayerPtrMap; // {Transport Layer Handle, TransportLayerPtr}

typedef std::vector<IInterfaceListObserverPtr> IInterfaceListObserverPtrVector;
typedef std::vector<ICameraListObserverPtr> ICameraListObserverPtrVector;

/**
* \brief Internal information about found system modules, registered observers etc.
*/
struct VmbSystem::Impl
{
    // Found transport layers, cameras and interfaces
    TransportLayerPtrMap                            m_transportLayers;
    LockableMap<std::string, CameraPtr>             m_cameras;                      //<! Map with available cameras, uses extended camera ID as the key
    ConditionHelper                                 m_camerasConditionHelper;
    LockableMap<VmbHandle_t, InterfacePtr>          m_interfaces;
    ConditionHelper                                 m_interfacesConditionHelper;
    // Registered observers
    LockableVector<ICameraListObserverPtr>          m_cameraObservers;
    ConditionHelper                                 m_cameraObserversConditionHelper;
    LockableVector<IInterfaceListObserverPtr>       m_interfaceObservers;
    ConditionHelper                                 m_interfaceObserversConditionHelper;
    
    // CameraFactory
    ICameraFactoryPtr                               m_pCameraFactory;

    // Logger
    Logger*                                         m_pLogger { nullptr};

    VmbErrorType UpdateCameraList();
    VmbErrorType UpdateInterfaceList();
    VmbErrorType AppendCamToMap( VmbCameraInfo_t const& camInfo );

    /**
     * \brief creates a new camera without inserting it into the map
     * 
     * \return a null pointer, if not successful, the pointer to the new object otherwise
     */
    CameraPtr CreateNewCamera(VmbCameraInfo_t const& camInfo)
    {
        InterfacePtr             pInterface;
        if (VmbErrorSuccess == GetInterfaceByHandle(camInfo.interfaceHandle, pInterface))
        {
            // TODO: Remove pCam with Interface change
            return SP_ACCESS(m_pCameraFactory)->CreateCamera(camInfo,
                                                             pInterface);
        }
        
        return CameraPtr();
    }

    bool IsIPAddress( const char *pStrID );

    VmbErrorType GetInterfaceList(std::vector<VmbInterfaceInfo_t>& interfaceInfos);
    VmbErrorType GetInterfaceByHandle(VmbHandle_t pHandle, InterfacePtr& pInterface);
    
    /**
    * \brief Saves the available transport layers into internal map m_transportLayers,
    *        this will be done just once in Startup() and then stay stable the whole session.
    */
    VmbErrorType SetTransportLayerList() noexcept;

    /** 
    * \brief    A look-up in the InterfacePtrMap for a specified strID
    *           No read lock inside!
    */
    VmbErrorType GetInterfaceByID(const std::string &strID, InterfacePtr& pInterface);


    VmbErrorType GetInterfacesByTL(const TransportLayer* pTransportLayer, InterfacePtr* pInterfaces, VmbUint32_t& size);
    VmbErrorType GetCamerasByTL(const TransportLayer* pTransportLayer, CameraPtr* pCameras, VmbUint32_t& size);
    VmbErrorType GetCamerasByInterface(const Interface* pInterface, CameraPtr* pCameras, VmbUint32_t& size);

    static void VMB_CALL CameraDiscoveryCallback( const VmbHandle_t handle, const char *name, void *context );
    static void VMB_CALL InterfaceDiscoveryCallback( const VmbHandle_t handle, const char *name, void *context );
};

VmbSystem &VmbSystem::GetInstance() noexcept
{
    return _instance;
}

VmbErrorType VmbSystem::QueryVersion( VmbVersionInfo_t &rVersion ) const noexcept
{
    rVersion.major = VMBCPP_VERSION_MAJOR;
    rVersion.minor = VMBCPP_VERSION_MINOR;
    rVersion.patch = VMBCPP_VERSION_PATCH;

    return VmbErrorSuccess;
}

VmbErrorType VmbSystem::Startup(const VmbFilePathChar_t* pathConfiguration)
{
    VmbError_t res = VmbErrorSuccess;

    res = VmbStartup(pathConfiguration);
    if ( VmbErrorSuccess == res )
    {
        // save the transport layers in the internal map
        res = m_pImpl->SetTransportLayerList();

        SetHandle( gVmbHandle );
    }

    return (VmbErrorType)res;
}

VmbErrorType VmbSystem::Startup()
{
    return Startup(nullptr);
}


VmbErrorType VmbSystem::Shutdown()
{    
    // Begin exclusive write lock camera observer list
    if ( true == m_pImpl->m_cameraObserversConditionHelper.EnterWriteLock( m_pImpl->m_cameraObservers, true ))
    {
        m_pImpl->m_cameraObservers.Vector.clear();

        // End write lock camera observer list
        m_pImpl->m_cameraObserversConditionHelper.ExitWriteLock( m_pImpl->m_cameraObservers );
    }
        
    // Begin exclusive write lock interface observer list
    if ( true == m_pImpl->m_interfaceObserversConditionHelper.EnterWriteLock( m_pImpl->m_interfaceObservers, true ))
    {
        m_pImpl->m_interfaceObservers.Vector.clear();

        // End write lock interface observer list
        m_pImpl->m_interfaceObserversConditionHelper.ExitWriteLock( m_pImpl->m_interfaceObservers );
    }    

    // Begin exclusive write lock camera list
    if ( true == m_pImpl->m_camerasConditionHelper.EnterWriteLock( m_pImpl->m_cameras, true ))
    {
        for (   CameraPtrMap::iterator iter = m_pImpl->m_cameras.Map.begin();
                m_pImpl->m_cameras.Map.end() != iter;
                ++iter)
        {
            SP_ACCESS( iter->second )->Close();
        }
        m_pImpl->m_cameras.Map.clear();

        // End write lock camera list
        m_pImpl->m_camerasConditionHelper.ExitWriteLock( m_pImpl->m_cameras );
    }    

    // Begin exclusive write lock interface list
    if ( true == m_pImpl->m_interfacesConditionHelper.EnterWriteLock( m_pImpl->m_interfaces, true ))
    {
        m_pImpl->m_interfaces.Map.clear();

        // End write lock interface list
        m_pImpl->m_interfacesConditionHelper.ExitWriteLock( m_pImpl->m_interfaces );
    }    
    m_pImpl->m_transportLayers.clear();
    VmbShutdown();
    
    return VmbErrorSuccess;
}

VmbErrorType VmbSystem::GetInterfaces( InterfacePtr *pInterfaces, VmbUint32_t &rnSize )
{
    // update m_interfaces (using write lock)
    VmbErrorType res = m_pImpl->UpdateInterfaceList();

    if (VmbErrorSuccess == res)
    {
        // Begin read lock interface list
        if (true == m_pImpl->m_interfacesConditionHelper.EnterReadLock(m_pImpl->m_interfaces))
        {
            if (nullptr == pInterfaces)
            {
                rnSize = (VmbUint32_t)m_pImpl->m_interfaces.Map.size();
                res = VmbErrorSuccess;
            }
            else if (m_pImpl->m_interfaces.Map.size() <= rnSize)
            {
                VmbUint32_t i = 0;
                for (InterfacePtrMap::iterator iter = m_pImpl->m_interfaces.Map.begin();
                    m_pImpl->m_interfaces.Map.end() != iter;
                    ++iter, ++i)
                {
                    pInterfaces[i] = iter->second;
                }
                rnSize = (VmbUint32_t)m_pImpl->m_interfaces.Map.size();
                res = VmbErrorSuccess;
            }
            else
            {
                res = VmbErrorMoreData;
            }
            // End read lock interface list
            m_pImpl->m_interfacesConditionHelper.ExitReadLock(m_pImpl->m_interfaces);
        }
        else
        {
            LOG_FREE_TEXT("Could not lock interface list")
        }
    }
    return res;
}

VmbErrorType VmbSystem::Impl::GetInterfaceByHandle(VmbHandle_t pHandle, InterfacePtr& rInterface)
{
    if ( nullptr == pHandle)
    {
        return VmbErrorBadParameter;
    }
    VmbErrorType res = VmbErrorNotFound;

    // Begin read lock interface list
    if (true == m_interfacesConditionHelper.EnterReadLock(m_interfaces))
    {
        InterfacePtrMap::iterator iter = m_interfaces.Map.find(pHandle);
        if (m_interfaces.Map.end() != iter)
        {
            rInterface = iter->second;
            // End read lock interface list
            m_interfacesConditionHelper.ExitReadLock(m_interfaces);
            return VmbErrorSuccess;
        }

        // End read lock interface list
        m_interfacesConditionHelper.ExitReadLock(m_interfaces);
    
        // update m_interfaces (using write lock)
        res = UpdateInterfaceList();

        // Begin read lock interface list
        if (true == m_interfacesConditionHelper.EnterReadLock(m_interfaces))
        {
            if (VmbErrorSuccess == res)
            {
                // New attempt to find interface in the map
                InterfacePtrMap::iterator iter = m_interfaces.Map.find(pHandle);
                if (m_interfaces.Map.end() != iter)
                {
                    rInterface = iter->second;
                }
                else
                {
                    res = VmbErrorNotFound;
                }
            }
            // End read lock interface list
            m_interfacesConditionHelper.ExitReadLock(m_interfaces);
        }
        else
        {
            LOG_FREE_TEXT("Could not lock interface list")
        }
    }
    else
    {
        LOG_FREE_TEXT("Could not lock interface list")
    }
    return res;
}

//VmbErrorType VmbSystem::Impl::GetInterfaceByHandle(VmbHandle_t pHandle, InterfacePtr& rInterface)
VmbErrorType VmbSystem::GetInterfaceByID( const char *pStrID, InterfacePtr &rInterface )
{
    if (nullptr == pStrID)
    {
        return VmbErrorBadParameter;
    }
    VmbErrorType res = VmbErrorNotFound;

    // Begin read lock interface list
    if (true == m_pImpl->m_interfacesConditionHelper.EnterReadLock(m_pImpl->m_interfaces))
    {
        res = m_pImpl->GetInterfaceByID(pStrID, rInterface);
        if (VmbErrorSuccess == res)
        {
            // End read lock interface list
            m_pImpl->m_interfacesConditionHelper.ExitReadLock(m_pImpl->m_interfaces);
            return VmbErrorSuccess;
        }

        // End read lock interface list
        m_pImpl->m_interfacesConditionHelper.ExitReadLock(m_pImpl->m_interfaces);
    
        // update m_interfaces (using write lock)
        res = m_pImpl->UpdateInterfaceList();
        if (VmbErrorSuccess == res)
        {
            // Begin read lock interface list
            if (true == m_pImpl->m_interfacesConditionHelper.EnterReadLock(m_pImpl->m_interfaces))
            {
                // New attempt to find interface in the map
                res = m_pImpl->GetInterfaceByID(pStrID, rInterface);
            
                // End read lock interface list
                m_pImpl->m_interfacesConditionHelper.ExitReadLock(m_pImpl->m_interfaces);
            }
            else
            {
                LOG_FREE_TEXT("Could not lock interface list")
            }
        }
    }
    else
    {
        LOG_FREE_TEXT("Could not lock interface list")
    }
    return res;
}

VmbErrorType VmbSystem::GetCameras( CameraPtr *pCameras, VmbUint32_t &rnSize )
{
    VmbErrorType res = VmbErrorInternalFault;

    // Begin write lock camera list
    if ( true == m_pImpl->m_camerasConditionHelper.EnterWriteLock( m_pImpl->m_cameras ))
    {
        res = m_pImpl->UpdateCameraList();

        if ( VmbErrorSuccess == res )
        {
            if ( nullptr == pCameras )
            {
                rnSize = (VmbUint32_t)m_pImpl->m_cameras.Map.size();
                res = VmbErrorSuccess;
            }
            else if ( m_pImpl->m_cameras.Map.size() <= rnSize )
            {
                VmbUint32_t i = 0;
                for (   CameraPtrMap::iterator iter = m_pImpl->m_cameras.Map.begin();
                        m_pImpl->m_cameras.Map.end() != iter;
                        ++iter, ++i )
                {
                    pCameras[i] = iter->second;
                }
                rnSize = (VmbUint32_t)m_pImpl->m_cameras.Map.size();
                res = VmbErrorSuccess;
            }
            else
            {
                res = VmbErrorMoreData;
            }
        }

        // End write lock camera list
        m_pImpl->m_camerasConditionHelper.ExitWriteLock( m_pImpl->m_cameras );
    }    

    return res;
}

VmbErrorType VmbSystem::GetCameraByID( const char *pStrID, CameraPtr &rCamera )
{
    if ( nullptr == pStrID )
    {
        return VmbErrorBadParameter;
    }

    VmbError_t res = VmbErrorNotFound;

    // Begin write lock camera list
    if ( true == m_pImpl->m_camerasConditionHelper.EnterWriteLock( m_pImpl->m_cameras ))
    {
        // Try to identify the desired camera by its ID (in the list of known cameras)
        CameraPtrMap::iterator iter = m_pImpl->m_cameras.Map.find( pStrID );
        if ( m_pImpl->m_cameras.Map.end() != iter )
        {
            rCamera = iter->second;
            res = VmbErrorSuccess;
        }
        else
        {
            VmbCameraInfo_t camInfo;
            res = VmbCameraInfoQuery( pStrID, &camInfo, sizeof(camInfo));
            if ( VmbErrorSuccess == res )
            {
                iter = m_pImpl->m_cameras.Map.find( camInfo.cameraIdExtended );
                if ( m_pImpl->m_cameras.Map.end() != iter )
                {
                    rCamera = iter->second;
                }
                else
                {
                    res = m_pImpl->AppendCamToMap( camInfo );

                    if (res == VmbErrorSuccess)
                    {
                        iter = m_pImpl->m_cameras.Map.find(camInfo.cameraIdExtended);
                        if (m_pImpl->m_cameras.Map.end() != iter)
                        {
                            rCamera = iter->second;
                        }
                        else
                        {
                            res = VmbErrorNotFound;
                        }
                    }
                }
            }
        }

        // End write lock camera list
        m_pImpl->m_camerasConditionHelper.ExitWriteLock( m_pImpl->m_cameras );
    }

    return (VmbErrorType)res;
}

VmbErrorType VmbSystem::OpenCameraByID( const char *pStrID, VmbAccessModeType eAccessMode, CameraPtr &rCamera )
{
    if ( nullptr == pStrID )
    {
        return VmbErrorBadParameter;
    }

    VmbErrorType res = GetCameraByID( pStrID, rCamera );
    if ( VmbErrorSuccess == res )
    {
        return SP_ACCESS( rCamera )->Open( eAccessMode );
    }

    return res;
}

CameraPtr VmbSystem::GetCameraPtrByHandle( const VmbHandle_t handle ) const
{
    CameraPtr res;

    // Begin read lock camera list
    if ( true == m_pImpl->m_camerasConditionHelper.EnterReadLock( m_pImpl->m_cameras ) )
    {
        for (   CameraPtrMap::const_iterator iter = m_pImpl->m_cameras.Map.begin();
                m_pImpl->m_cameras.Map.end() != iter;
                ++iter)
        {
            if ( SP_ACCESS( iter->second )->GetHandle() == handle )
            {
                res = iter->second;
                break;
            }
        }

        // End read lock camera list
        m_pImpl->m_camerasConditionHelper.ExitReadLock( m_pImpl->m_cameras );
    }
    else
    {
        LOG_FREE_TEXT( "Could not lock camera list")
    }

    return res;
}


VmbErrorType VmbSystem::Impl::GetInterfacesByTL( const TransportLayer *pTransportLayer, InterfacePtr *pInterfaces, VmbUint32_t &rnSize )
{
    // update m_interfaces (using write lock)
    VmbErrorType res = UpdateInterfaceList();
    if (VmbErrorSuccess != res)
    {
        return res;
    }
    // Begin read lock interface list
    if (true ==m_interfacesConditionHelper.EnterReadLock(m_interfaces))
    {
        if (nullptr == pInterfaces)
        {
            rnSize = (VmbUint32_t)m_interfaces.Map.size();
            res = VmbErrorSuccess;
        }
        else if (m_interfaces.Map.size() <= rnSize)
        {
            VmbUint32_t foundCnt = 0;  // counter for matching items

            for (InterfacePtrMap::iterator iter = m_interfaces.Map.begin();
                m_interfaces.Map.end() != iter;
                ++iter)
            {
                InterfacePtr tmpInterface = iter->second;
                TransportLayerPtr tmpTransportLayer;
                if (VmbErrorSuccess == SP_ACCESS(tmpInterface)->GetTransportLayer(tmpTransportLayer))
                {
                    if ( pTransportLayer->GetHandle() == SP_ACCESS(tmpTransportLayer)->GetHandle() ) {
                        pInterfaces[foundCnt++] = tmpInterface;
                    }
                }
                else
                {
                    res = VmbErrorInternalFault;
                }
            }
            rnSize = foundCnt;  // the real number of items in the returned list
        }
        else
        {
            res = VmbErrorMoreData;
        }
        // End read lock interface list
        m_interfacesConditionHelper.ExitReadLock(m_interfaces);
    }
    else
    {
        LOG_FREE_TEXT("Could not lock interface list")
    }
    return res;
}

VmbErrorType VmbSystem::Impl::GetCamerasByTL(const TransportLayer* pTransportLayer, CameraPtr* pCameras, VmbUint32_t& rnSize)
{
    VmbErrorType res = VmbErrorNotFound;

    // Begin write lock camera list
    if (true == m_camerasConditionHelper.EnterWriteLock(m_cameras))
    {
        res = UpdateCameraList();
        if (VmbErrorSuccess == res)
        {
        if (nullptr == pCameras)
            {
            rnSize = (VmbUint32_t)m_cameras.Map.size();
            res = VmbErrorSuccess;
        }
        else if (m_cameras.Map.size() <= rnSize)
        {
            VmbUint32_t foundCnt = 0;  // counter for matching items

            for (CameraPtrMap::iterator iter = m_cameras.Map.begin();
                m_cameras.Map.end() != iter;
                ++iter)
            {
                CameraPtr tmpCamera = iter->second;
                TransportLayerPtr tmpTransportLayer;
                    if (VmbErrorSuccess == SP_ACCESS(tmpCamera)->GetTransportLayer(tmpTransportLayer))
                {
                    if (pTransportLayer->GetHandle() == SP_ACCESS(tmpTransportLayer)->GetHandle()) {
                        pCameras[foundCnt++] = tmpCamera;
                    }
                }
                else
                {
                    res = VmbErrorInternalFault;
                }
            }
            rnSize = foundCnt;  // the real number of items in the returned list
        }
        else
        {
            res = VmbErrorMoreData;
        }
        }
        // End write lock camera list
        m_camerasConditionHelper.ExitWriteLock(m_cameras);
    }
    else
    {
        LOG_FREE_TEXT("Could not lock camera list")        
    }

    return res;
}

VmbErrorType VmbSystem::Impl::GetCamerasByInterface(const Interface* pInterface, CameraPtr* pCameras, VmbUint32_t& rnSize)
{
    VmbErrorType res = VmbErrorNotFound;

    // Begin write lock camera list
    if (true == m_camerasConditionHelper.EnterWriteLock(m_cameras))
    {
        res = UpdateCameraList();
        if (VmbErrorSuccess == res)
        {
        if (nullptr == pCameras)
        {
            rnSize = (VmbUint32_t)m_cameras.Map.size();
            res = VmbErrorSuccess;
        }
        else if (m_cameras.Map.size() <= rnSize)
        {
            VmbUint32_t foundCnt = 0;  // counter for matching items

                for (CameraPtrMap::iterator iter = m_cameras.Map.begin();
                m_cameras.Map.end() != iter;
                ++iter)
            {
                CameraPtr tmpCamera = iter->second;
                InterfacePtr tmpInterface;
                if (VmbErrorSuccess == SP_ACCESS(tmpCamera)->GetInterface(tmpInterface))
                {
                    if (pInterface->GetHandle() == SP_ACCESS(tmpInterface)->GetHandle()) {
                        pCameras[foundCnt++] = tmpCamera;
                    }
                }
                else
                {
                    res = VmbErrorInternalFault;
                }
            }
            rnSize = foundCnt;  // the real number of items in the returned list
        }
        else
        {
            res = VmbErrorMoreData;
        }
        }
        // End write lock camera list
        m_camerasConditionHelper.ExitWriteLock(m_cameras);
    }
    else
    {
        LOG_FREE_TEXT("Could not lock camera list")
    }

    return res;
}

VmbErrorType VmbSystem::GetTransportLayers( TransportLayerPtr *pTransportLayers, VmbUint32_t &rnSize ) noexcept
{
    return CopyMapValuesToBuffer(m_pImpl->m_transportLayers, pTransportLayers, rnSize);
}

VmbErrorType VmbSystem::GetTransportLayerByID( const char *pStrID, TransportLayerPtr &rTransportLayer )
{
    if ( nullptr == pStrID )
    {
        return VmbErrorBadParameter;
    }

    VmbErrorType res = VmbErrorNotFound;
    // iterate through the map comparing the TL-ID's
    for(auto iter = m_pImpl->m_transportLayers.begin(); iter != m_pImpl->m_transportLayers.end(); ++iter) {
        std::string tmpStrID;
        res = SP_ACCESS(iter->second)->GetID(tmpStrID);
        if ( tmpStrID == std::string(pStrID) )
        {
            rTransportLayer = iter->second;
            res = VmbErrorSuccess;
            break;
        }
    }

    return res;
}

void VMB_CALL VmbSystem::Impl::CameraDiscoveryCallback( const VmbHandle_t pHandle, const char* /*name*/, void* /*context*/ )
{
    VmbError_t err;
    std::string cameraId;
    VmbUint32_t nCount = 0;
    
    err = VmbFeatureStringGet(pHandle, "EventCameraDiscoveryCameraID", nullptr, nCount, &nCount);
    if (0 < nCount && VmbErrorSuccess == err)
    {
        std::vector<char> strID(nCount);
        err = VmbFeatureStringGet(pHandle, "EventCameraDiscoveryCameraID", &strID[0], nCount, &nCount);
        if (VmbErrorSuccess == err)
        {
            cameraId = &*strID.begin();
        }
    }

    if ( VmbErrorSuccess == err )
    {
        UpdateTriggerType reason = UpdateTriggerPluggedIn;
        const char* pReason = nullptr;

        // Get the reason that has triggered the callback
        err = VmbFeatureEnumGet(pHandle, "EventCameraDiscoveryType", &pReason );
        if ( VmbErrorSuccess == err )
        {
            if (std::strcmp(pReason, "Missing") == 0)
            {
                reason = UpdateTriggerPluggedOut;
            }
            else if (std::strcmp(pReason, "Detected") == 0)
            {
                reason = UpdateTriggerPluggedIn;
            }
            else
            {
                reason = UpdateTriggerOpenStateChanged;
            }
                    
            // Begin read lock camera list
            if ( true == _instance.m_pImpl->m_camerasConditionHelper.EnterReadLock( _instance.m_pImpl->m_cameras ))
            {
                // will only find the entry if EventCameraDiscoveryCameraID returns the extended ID
                CameraPtrMap::iterator iter = _instance.m_pImpl->m_cameras.Map.find( cameraId );
                CameraPtr pCam;

                bool bFound;

                // Was the camera known before?
                if ( _instance.m_pImpl->m_cameras.Map.end() != iter )
                {
                    bFound = true;
                    pCam = iter->second;
                }
                else
                {
                    bFound = false;

                    // As long as EventCameraDiscoveryCameraID will return the regular ID and not the extended one,
                    // we need to compare every entry entry in the camera map (which uses the extended ID as key)
                    for (auto& cameraMapEntry : _instance.m_pImpl->m_cameras.Map)
                    {
                        std::string currentCamId;
                        if (VmbErrorSuccess == cameraMapEntry.second->GetID(currentCamId)
                            && currentCamId == cameraId)
                        {
                            bFound = true;
                        }
                    }                        
                }

                // End read lock camera list
                _instance.m_pImpl->m_camerasConditionHelper.ExitReadLock( _instance.m_pImpl->m_cameras );

                // If the camera was not known before we query for it
                if ( false == bFound )
                {
                    err = _instance.GetCameraByID( cameraId.c_str(), pCam );
                    if ( VmbErrorSuccess != err )
                    {
                        err = VmbErrorInternalFault;
                        LOG_FREE_TEXT( "Could not find a known camera in camera list")
                    }
                }

                // Now that we know about the reason for the callback and the camera we can call all registered observers
                if ( VmbErrorSuccess == err )
                {
                    // Begin read lock camera observer list
                    if ( true == _instance.m_pImpl->m_cameraObserversConditionHelper.EnterReadLock( _instance.m_pImpl->m_cameraObservers ))
                    {
                        for (   ICameraListObserverPtrVector::iterator iter = _instance.m_pImpl->m_cameraObservers.Vector.begin();
                                _instance.m_pImpl->m_cameraObservers.Vector.end() != iter;
                                ++iter )
                        {
                            SP_ACCESS(( *iter ))->CameraListChanged( pCam, reason );
                        }

                        // End read lock camera observer list
                        _instance.m_pImpl->m_cameraObserversConditionHelper.ExitReadLock( _instance.m_pImpl->m_cameraObservers );
                    }
                    else
                    {
                        LOG_FREE_TEXT( "Could not lock camera observer list")
                    }
                }
            }
            else
            {
                LOG_FREE_TEXT( "Could not lock camera list")
            }
        }
        else
        {
            LOG_FREE_TEXT( "Could not get callback trigger" )
        }
    }
    else
    {
        LOG_FREE_TEXT( "Could not get camera ID" )
    }
}

void VMB_CALL VmbSystem::Impl::InterfaceDiscoveryCallback( const VmbHandle_t pHandle, const char* /*name*/, void* /*context*/)
{
    VmbError_t err;
    std::string interfaceID;
    VmbUint32_t nCount = 0;

    // Get the ID of the interface that has triggered the callback
    err = VmbFeatureStringGet(pHandle, "EventInterfaceDiscoveryInterfaceID", nullptr, nCount, &nCount);
    if (0 < nCount && VmbErrorSuccess == err)
    {
        std::vector<char> strID(nCount);
        err = VmbFeatureStringGet(pHandle, "EventInterfaceDiscoveryInterfaceID", &strID[0], nCount, &nCount);
        if (VmbErrorSuccess == err)
        {
            interfaceID = &*strID.begin();
        }
    }

    if ( VmbErrorSuccess == err )
    {
        // Begin read lock interface list
        if ( true == _instance.m_pImpl->m_interfacesConditionHelper.EnterReadLock( _instance.m_pImpl->m_interfaces ))
        {
            InterfacePtr pInterface;
            bool bFoundBeforeUpdate = _instance.m_pImpl->GetInterfaceByID(interfaceID, pInterface) == VmbErrorSuccess;

            // End read lock interface list
            _instance.m_pImpl->m_interfacesConditionHelper.ExitReadLock( _instance.m_pImpl->m_interfaces );
                        
            err = _instance.m_pImpl->UpdateInterfaceList();
                            
            if ( VmbErrorSuccess == err )
            {
                // Begin read lock interface list
                if ( true == _instance.m_pImpl->m_interfacesConditionHelper.EnterReadLock( _instance.m_pImpl->m_interfaces ))
                {
                    UpdateTriggerType reason = UpdateTriggerPluggedIn;
                    InterfacePtr pInterface2;
                    bool bFoundAfterUpdate = _instance.m_pImpl->GetInterfaceByID(interfaceID, pInterface2) == VmbErrorSuccess;

                    // The interface was known before
                    if ( true == bFoundBeforeUpdate )
                    {
                        // The interface now has been removed
                        if ( false == bFoundAfterUpdate )
                        {
                            reason = UpdateTriggerPluggedOut;
                        }
                        else
                        {
                            reason = UpdateTriggerOpenStateChanged;
                        }
                    }
                    // The interface is new
                    else
                    {
                        if ( true == bFoundAfterUpdate)
                        {
                            pInterface = pInterface2;
                            reason = UpdateTriggerPluggedIn;
                        }
                        else
                        {
                            err = VmbErrorInternalFault;
                            // Do some logging
                            LOG_FREE_TEXT( "Could not find interface in interface list." )
                        }
                    }

                    // End read lock interface list
                    _instance.m_pImpl->m_interfacesConditionHelper.ExitReadLock( _instance.m_pImpl->m_interfaces );

                    if ( VmbErrorSuccess == err )
                    {
                        // Begin read lock interface observer list
                        if ( true == _instance.m_pImpl->m_interfaceObserversConditionHelper.EnterReadLock( _instance.m_pImpl->m_interfaceObservers ))
                        {
                            for (   IInterfaceListObserverPtrVector::iterator iter = _instance.m_pImpl->m_interfaceObservers.Vector.begin();
                                _instance.m_pImpl->m_interfaceObservers.Vector.end() != iter;
                                ++iter)
                            {
                                SP_ACCESS(( *iter ))->InterfaceListChanged( pInterface, reason );
                            }

                            // End read lock interface observer list
                            _instance.m_pImpl->m_interfaceObserversConditionHelper.ExitReadLock( _instance.m_pImpl->m_interfaceObservers );
                        }
                        else
                        {
                            LOG_FREE_TEXT( "Could not lock interface observer list")
                        }
                    }                        
                }
                else
                {
                    LOG_FREE_TEXT( "Could not lock interface list")
                }
            }                
        }            
        
    }
}

VmbErrorType VmbSystem::RegisterCameraListObserver( const ICameraListObserverPtr &rObserver )
{
    if ( SP_ISNULL( rObserver ))
    {
        return VmbErrorBadParameter;
    }

    VmbError_t res = VmbErrorSuccess;

    // Begin write lock camera observer list
    if ( true == _instance.m_pImpl->m_cameraObserversConditionHelper.EnterWriteLock( m_pImpl->m_cameraObservers ))
    {
        // The very same observer cannot be registered twice
        for ( size_t i=0; i<m_pImpl->m_cameraObservers.Vector.size(); ++i )
        {
            if ( SP_ISEQUAL( rObserver, m_pImpl->m_cameraObservers.Vector[i] ))
            {
                res = VmbErrorInvalidCall;
                break;
            }
        }

        if ( VmbErrorSuccess == res )
        {
            m_pImpl->m_cameraObservers.Vector.push_back( rObserver );

            // first camera observer is registered
            if ( 1 == m_pImpl->m_cameraObservers.Vector.size() )
            {
                // make sure camera list is initialized (to be able to detect missing cameras correctly)
                if (true == m_pImpl->m_camerasConditionHelper.EnterWriteLock(m_pImpl->m_cameras))
                {
                    if(VmbErrorSuccess != m_pImpl->UpdateCameraList())
                    {
                        LOG_FREE_TEXT("Could not update camera list")
                    }

                    m_pImpl->m_camerasConditionHelper.ExitWriteLock(m_pImpl->m_cameras);
                }
                else
                {
                    LOG_FREE_TEXT("Could not lock camera list")
                }

                // enable camera discovery events at supervisor
                res = VmbFeatureEnumSet(gVmbHandle, "EventSelector", "CameraDiscovery");

                if (VmbErrorSuccess == res)
                {
                    res = VmbFeatureEnumSet(gVmbHandle, "EventNotification", "On");

                    if (VmbErrorSuccess == res)
                    {
                        res = VmbFeatureInvalidationRegister(gVmbHandle, "EventCameraDiscovery", m_pImpl->CameraDiscoveryCallback, nullptr);
                    }
                }
                
                if ( VmbErrorSuccess != res )
                {
                    // Rollback
                    m_pImpl->m_cameraObservers.Vector.pop_back();
                    // Do some logging
                    LOG_FREE_TEXT( "Could not register camera list observer" )
                }
            }
        }

        // End write lock camera observer list
        _instance.m_pImpl->m_cameraObserversConditionHelper.ExitWriteLock( m_pImpl->m_cameraObservers );
    }    
    
    return static_cast<VmbErrorType>(res);
}

VmbErrorType VmbSystem::UnregisterCameraListObserver( const ICameraListObserverPtr &rObserver )
{
    if ( SP_ISNULL( rObserver ))
    {
        return VmbErrorBadParameter;
    }

    VmbError_t res = VmbErrorNotFound;

    // Begin exclusive write lock camera observer list
    if ( true == m_pImpl->m_cameraObserversConditionHelper.EnterWriteLock( m_pImpl->m_cameraObservers, true ))
    {
        for (   ICameraListObserverPtrVector::iterator iter = m_pImpl->m_cameraObservers.Vector.begin();
                m_pImpl->m_cameraObservers.Vector.end() != iter;)    
        {
            if ( SP_ISEQUAL( rObserver, *iter ))
            {
                // If we are about to unregister the last observer we cancel all camera discovery notifications
                if ( 1 == m_pImpl->m_cameraObservers.Vector.size() )
                {
                    res = VmbFeatureInvalidationUnregister( gVmbHandle, "EventCameraDiscovery", m_pImpl->CameraDiscoveryCallback );

                    if (VmbErrorSuccess == res)
                    {
                        res = VmbFeatureEnumSet(gVmbHandle, "EventSelector", "CameraDiscovery");

                        if (VmbErrorSuccess == res)
                        {
                            res = VmbFeatureEnumSet(gVmbHandle, "EventNotification", "Off");
                        }
                    }
                }
                
                if (    VmbErrorSuccess == res
                     || 1 < m_pImpl->m_cameraObservers.Vector.size() )
                {
                    iter = m_pImpl->m_cameraObservers.Vector.erase( iter );
                    res = VmbErrorSuccess;
                }
                break;
            }
            else
            {
                ++iter;
            }
        }

        // End write lock camera observer list
        m_pImpl->m_cameraObserversConditionHelper.ExitWriteLock( m_pImpl->m_cameraObservers );
    }
    else
    {
        LOG_FREE_TEXT( "Could not lock camera observer list.")
        res = VmbErrorInternalFault;
    }

    return static_cast<VmbErrorType>(res);
}

VmbErrorType VmbSystem::RegisterInterfaceListObserver( const IInterfaceListObserverPtr &rObserver )
{
    if ( SP_ISNULL( rObserver ))
    {
        return VmbErrorBadParameter;
    }

    VmbError_t res = VmbErrorSuccess;

    // Begin write lock interface observer list
    if ( true == _instance.m_pImpl->m_interfaceObserversConditionHelper.EnterWriteLock( m_pImpl->m_interfaceObservers ))
    {
        // The very same observer cannot be registered twice
        for ( size_t i=0; i<m_pImpl->m_interfaceObservers.Vector.size(); ++i )
        {
            if ( SP_ISEQUAL( rObserver, m_pImpl->m_interfaceObservers.Vector[i] ))
            {
                res = VmbErrorInvalidCall;
                break;
            }
        }

        if ( VmbErrorSuccess == res )
        {
            m_pImpl->m_interfaceObservers.Vector.push_back( rObserver );

            if ( 1 == m_pImpl->m_interfaceObservers.Vector.size() )
            {
                res = VmbFeatureEnumSet(gVmbHandle, "EventSelector", "InterfaceDiscovery");
                
                if (VmbErrorSuccess == res)
                {
                    res = VmbFeatureEnumSet(gVmbHandle, "EventNotification", "On");

                    if (VmbErrorSuccess == res)
                    {
                        res = VmbFeatureInvalidationRegister(gVmbHandle, "EventInterfaceDiscovery", m_pImpl->InterfaceDiscoveryCallback, nullptr);
                    }
                }

                if ( VmbErrorSuccess != res )
                {
                    // Rollback
                    m_pImpl->m_interfaceObservers.Vector.pop_back();

                    // Do some logging
                    LOG_FREE_TEXT( "Could not register interface list observer" )
                }
            }
        }

        // End write lock interface observer list
        _instance.m_pImpl->m_interfaceObserversConditionHelper.ExitWriteLock( m_pImpl->m_interfaceObservers );
    }    

    return static_cast<VmbErrorType>(res);
}

VmbErrorType VmbSystem::UnregisterInterfaceListObserver( const IInterfaceListObserverPtr &rObserver )
{
    if ( SP_ISNULL( rObserver ))
    {
        return VmbErrorBadParameter;
    }

    VmbError_t res = VmbErrorNotFound;

    // Begin exclusive write lock interface observer list
    if ( true == _instance.m_pImpl->m_interfaceObserversConditionHelper.EnterWriteLock( m_pImpl->m_interfaceObservers, true ))
    {
        for (   IInterfaceListObserverPtrVector::iterator iter = m_pImpl->m_interfaceObservers.Vector.begin();
                m_pImpl->m_interfaceObservers.Vector.end() != iter;)
        {
            if ( SP_ISEQUAL( rObserver, *iter ))
            {
                // If we are about to unregister the last observer we cancel all interface discovery notifications
                if ( 1 == m_pImpl->m_interfaceObservers.Vector.size() )
                {
                    res = VmbFeatureInvalidationUnregister( gVmbHandle, "EventInterfaceDiscovery", m_pImpl->InterfaceDiscoveryCallback );

                    if (VmbErrorSuccess == res)
                    {
                        res = VmbFeatureEnumSet(gVmbHandle, "EventSelector", "InterfaceDiscovery");

                        if (VmbErrorSuccess == res)
                        {
                            res = VmbFeatureEnumSet(gVmbHandle, "EventNotification", "Off");
                        }
                    }
                }
                if (    VmbErrorSuccess == res
                     || 1 < m_pImpl->m_interfaceObservers.Vector.size() )
                {
                    iter = m_pImpl->m_interfaceObservers.Vector.erase( iter );
                    res = VmbErrorSuccess;
                }
                break;
            }
            else
            {
                ++iter;
            }
        }

        // End write lock interface observer list
        _instance.m_pImpl->m_interfaceObserversConditionHelper.ExitWriteLock( m_pImpl->m_interfaceObservers );
    }
    else
    {
        LOG_FREE_TEXT( "Could not lock interface observer list.")
        res = VmbErrorInternalFault;
    }

    return static_cast<VmbErrorType>(res);
}

VmbErrorType VmbSystem::RegisterCameraFactory( const ICameraFactoryPtr &cameraFactory )
{
    if ( SP_ISNULL( cameraFactory ))
    {
        return VmbErrorBadParameter;
    }

    m_pImpl->m_pCameraFactory = cameraFactory;
    
    return VmbErrorSuccess;
}

VmbErrorType VmbSystem::UnregisterCameraFactory()
{
    m_pImpl->m_pCameraFactory = ICameraFactoryPtr( new DefaultCameraFactory() );

    if ( SP_ISNULL( m_pImpl->m_pCameraFactory ))
    {
        return VmbErrorInternalFault;
    }

    return VmbErrorSuccess;
}

// Singleton
VmbSystem::VmbSystem()
    :   m_pImpl( new Impl() )
{
    m_pImpl->m_pLogger = CreateLogger();
    m_pImpl->m_pCameraFactory = ICameraFactoryPtr( new DefaultCameraFactory() );
}

VmbSystem::~VmbSystem()
{
    delete m_pImpl->m_pLogger;
    m_pImpl->m_pLogger = nullptr;
}

// Instance
VmbSystem VmbSystem::_instance;

// Gets a list of all connected interfaces and updates the internal interfaces map accordingly.
// Reference counting for removed interfaces is decreased,
// new interfaces are added.
VmbErrorType VmbSystem::Impl::UpdateInterfaceList()
{
    std::vector<VmbInterfaceInfo_t> interfaceInfos;
    VmbErrorType res = GetInterfaceList( interfaceInfos );
    VmbUint32_t nCount = (VmbUint32_t)interfaceInfos.size();

    if ( VmbErrorSuccess == res )
    {
        // Begin write lock interface list
        if (true == m_interfacesConditionHelper.EnterWriteLock(m_interfaces))
        {
            InterfacePtrMap::iterator iter = m_interfaces.Map.begin();
            std::vector<VmbInterfaceInfo_t>::iterator iterInfo = interfaceInfos.begin();
            bool bFound = false;

            // Delete removed Interfaces from m_interfaces
            while ( m_interfaces.Map.end() != iter )
            {
                for ( VmbUint32_t i=0; i<nCount; ++i, ++iterInfo )
                {
                    if ( iterInfo->interfaceIdString == iter->first )
                    {
                        bFound = true;
                        break;
                    }
                }

                if ( false == bFound )
                {
                    m_interfaces.Map.erase( iter++ );
                }
                else
                {
                    ++iter;
                }

                bFound = false;
                iterInfo = interfaceInfos.begin();
            }

            // Add new Interfaces to m_Interfaces
            while ( 0 < nCount-- )
            {
                iter = m_interfaces.Map.find( iterInfo->interfaceHandle );

                if ( m_interfaces.Map.end() == iter )
                {
                    auto tlIter = m_transportLayers.find(iterInfo->transportLayerHandle);
                    if (m_transportLayers.end() != tlIter)
                    {
                        TransportLayerPtr pTransportLayer = tlIter->second;
                        Interface::GetCamerasByInterfaceFunction getCamerasFunc = std::bind(&VmbSystem::Impl::GetCamerasByInterface, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
                        SP_SET(m_interfaces.Map[iterInfo->interfaceHandle], new Interface(*iterInfo, pTransportLayer, getCamerasFunc));
                    }
                    else
                    {
                        res = VmbErrorTLNotFound;
                    }
                }

                ++iterInfo;
            }
            // End write lock interface list
            m_interfacesConditionHelper.ExitWriteLock(m_interfaces);
        }
        else
        {
            LOG_FREE_TEXT("Could not lock interface list")
        }
    }
    return res;
}

// Gets a list of all connected cameras and updates the internal cameras map accordingly.
// Reference counting for removed cameras is decreased,
// new cameras are added.
VmbErrorType VmbSystem::Impl::UpdateCameraList()
{
    VmbError_t res = VmbErrorSuccess;
    VmbUint32_t nCount = 0;

    try
    {
        std::vector<VmbCameraInfo_t> cameraInfos( 10 );

        // First get 10 cameras at most
        res = VmbCamerasList( &cameraInfos[0], (VmbUint32_t)cameraInfos.size(), &nCount, sizeof(VmbCameraInfo_t) );
        // If there are more get them eventually
        // If even more new cameras were discovered in between the function calls we increase the allocated memory consecutively
        while ( VmbErrorMoreData == res )
        {
            cameraInfos.resize( nCount );
            res = VmbCamerasList( &cameraInfos[0], (VmbUint32_t)cameraInfos.size(), &nCount, sizeof(VmbCameraInfo_t) );
        }

        if ( VmbErrorSuccess == res )
        {
            if( 0 != nCount )
            {
                if( nCount < cameraInfos.size() )
                {
                    cameraInfos.resize( nCount );
                }
                CameraPtrMap::iterator  mapPos  = m_cameras.Map.begin();
                typedef std::vector<VmbCameraInfo_t>::const_iterator const_info_iterator;

                // Delete removed cameras from m_cameras
                while ( m_cameras.Map.end() != mapPos )
                {
                    bool bFound = false;
                    for( const_info_iterator infoPos = cameraInfos.begin(); cameraInfos.end() != infoPos; ++infoPos )
                    {
                        if ( infoPos->cameraIdExtended == mapPos->first )
                        {
                            bFound = true;
                            break;
                        }
                    }

                    if ( false == bFound )
                    {
                        m_cameras.Map.erase( mapPos++ );
                    }
                    else
                    {
                        ++mapPos;
                    }
                }

                // Add new cameras to m_cameras
                for (const_info_iterator infoPos = cameraInfos.begin(); infoPos != cameraInfos.end(); ++infoPos )
                {
                    CameraPtrMap::const_iterator findPos = m_cameras.Map.find( infoPos->cameraIdExtended);
            
                    if ( m_cameras.Map.end() == findPos )
                    {
                        AppendCamToMap( *infoPos );
                    }
                }
            }
            else
            {
                m_cameras.Map.clear();
            }
        }
    }
    catch( const std::bad_alloc& /*badAlloc*/ )
    {
        return VmbErrorResources;
    }
    

    return (VmbErrorType)res;
}

Logger* VmbSystem::GetLogger() const noexcept
{
    if(m_pImpl == nullptr)
        return nullptr;
    return m_pImpl->m_pLogger;
}

bool VmbSystem::Impl::IsIPAddress( const char *pStrID )
{
    if( nullptr == pStrID )
    {
        return false;
    }

    size_t nCount = 0;
    size_t nSize = 0;
    size_t nIndex = 0;
    while( pStrID[nIndex] != '\0' )
    {
        if( isdigit( pStrID[nIndex] ) != 0 )
        {
            if( nSize >= 3 )
            {
                return false;
            }
            nSize++;
        }
        else if( '.' == pStrID[nIndex] )
        {
            if(     (nSize <= 0)
                ||  (nSize > 3)
                ||  (nCount >= 3) )
            {
                return false;
            }
            nCount++;
            nSize = 0;
        }
        else
        {
            return false;
        }

        nIndex++;
    }
    if(     (nSize <= 0)
        ||  (nSize > 3)
        ||  (nCount != 3) )
    {
        return false;
    }

    return true;
}

VmbErrorType VmbSystem::Impl::AppendCamToMap( VmbCameraInfo_t const& camInfo )
{
    InterfacePtr             pInterface;

    // HINT: Before inserting (and potentially overwriting) a camera, we check whether it is present already
    try
    {
        auto insertionResult = m_cameras.Map.emplace(camInfo.cameraIdExtended, nullptr);
        if (insertionResult.second
            || insertionResult.first->second == nullptr
            || !insertionResult.first->second->ExtendedIdEquals(camInfo.cameraIdExtended))
        {
            bool insertionFailure = false;
            // new camera or camera that needs to overwrite the existing non-extended id
            try
            {
                insertionResult.first->second = CreateNewCamera(camInfo);
                if (insertionResult.first->second == nullptr)
                {
                    insertionFailure = true;
                }
            }
            catch (...)
            {
                insertionFailure = true;
            }

            if (insertionFailure)
            {
                // we cannot fill the value of the entry -> remove the entry to avoid
                // having an entry pointing to null
                m_cameras.Map.erase(insertionResult.first);
                return VmbErrorResources;
            }
        }
    }
    catch (std::bad_alloc const&)
    {
        return VmbErrorResources;
    }
    return VmbErrorSuccess;
}

VmbErrorType VmbSystem::Impl::GetInterfaceList( std::vector<VmbInterfaceInfo_t> &rInterfaceInfos )
{
    VmbError_t res;
    VmbUint32_t nCount;

    res = VmbInterfacesList( nullptr, 0, &nCount, sizeof(VmbInterfaceInfo_t));
    if ( VmbErrorSuccess == res )
    {
        rInterfaceInfos.resize( nCount );
        res = VmbInterfacesList( &rInterfaceInfos[0], nCount, &nCount, sizeof(VmbInterfaceInfo_t));
    }

    return (VmbErrorType)res;
}

VmbErrorType VmbSystem::Impl::SetTransportLayerList() noexcept
{
    // check if the function has been already called
    if ( !m_transportLayers.empty())
    {
        return VmbErrorInvalidAccess;
    }
    
    VmbUint32_t nCount;

    VmbError_t res = VmbTransportLayersList( nullptr, 0, &nCount, sizeof(VmbTransportLayerInfo_t));
    if ( VmbErrorSuccess == res )
    {
        try
        {
            std::vector<VmbTransportLayerInfo_t> rTransportLayerInfos(nCount);
            res = VmbTransportLayersList(&rTransportLayerInfos[0], nCount, &nCount, sizeof(VmbTransportLayerInfo_t));
            if (VmbErrorSuccess == res)
            {
                for (std::vector<VmbTransportLayerInfo_t>::const_iterator iter = rTransportLayerInfos.begin();
                     rTransportLayerInfos.end() != iter;
                     ++iter)
                {
                    TransportLayer::GetInterfacesByTLFunction getInterfacesFunc = std::bind(&VmbSystem::Impl::GetInterfacesByTL, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
                    TransportLayer::GetCamerasByTLFunction getCamerasFunc = std::bind(&VmbSystem::Impl::GetCamerasByTL, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
                    SP_SET(m_transportLayers[iter->transportLayerHandle], new TransportLayer(*iter, getInterfacesFunc, getCamerasFunc));
                }
            }
            else
            {
                return VmbErrorTLNotFound;
            }
        }
        catch (std::bad_alloc const&)
        {
            return VmbErrorResources;
        }
    }

    return static_cast<VmbErrorType>(res);
}

VmbErrorType VmbSystem::Impl::GetInterfaceByID(const std::string& strID, InterfacePtr& rInterface)
{
    for (InterfacePtrMap::iterator iter = m_interfaces.Map.begin();
        m_interfaces.Map.end() != iter;
        ++iter)
    {
        std::string tmpID;
        VmbErrorType res = SP_ACCESS(iter->second)->GetID(tmpID);
        if (VmbErrorSuccess == res && strID == tmpID)
        {
            rInterface = iter->second;
            return VmbErrorSuccess;
        }
    }

    return VmbErrorNotFound;
}

}  // namespace VmbCPP
