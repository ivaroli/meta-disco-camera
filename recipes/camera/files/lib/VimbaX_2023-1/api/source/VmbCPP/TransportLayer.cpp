/*=============================================================================
  Copyright (C) 2022 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------
 
  File:        TransportLayer.cpp

  Description: Implementation of class VmbCPP::TransportLayer.

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
#pragma warning(disable:4996)
#include <map>
#pragma warning(default:4996)

#include <memory>
#include <utility>

#include <VmbCPP/TransportLayer.h>

#include "CopyUtils.hpp"

namespace VmbCPP {

struct TransportLayer::Impl
{
    /**
    * \brief   Copy of transport layer infos
    */
    struct TransportLayerInfo
    {
        /**
        * \name TransportLayerInfo
        * \{
        */
        std::string                 transportLayerIdString;     //!< Unique identifier for each TL
        VmbTransportLayerType_t     transportLayerType;         //!< TL type
        std::string                 transportLayerName;         //!< TL name
        std::string                 transportLayerModelName;    //!< TL model name
        std::string                 transportLayerVendor;       //!< TL vendor
        std::string                 transportLayerVersion;      //!< TL version
        std::string                 transportLayerPath;         //!< TL full path
        /**
        * \}
        */
    } m_transportLayerInfo;

    GetInterfacesByTLFunction m_getInterfacesByTLFunc;
    GetCamerasByTLFunction m_getCamerasByTLFunc;
};

TransportLayer::TransportLayer(const VmbTransportLayerInfo_t& transportLayerInfo, GetInterfacesByTLFunction getInterfacesByTLFunc, GetCamerasByTLFunction getCamerasByTLFunc )
    :   m_pImpl(new Impl())
{
    m_pImpl->m_transportLayerInfo.transportLayerIdString.assign( transportLayerInfo.transportLayerIdString ? transportLayerInfo.transportLayerIdString : "" );
    m_pImpl->m_transportLayerInfo.transportLayerType = transportLayerInfo.transportLayerType;
    m_pImpl->m_transportLayerInfo.transportLayerName.assign( transportLayerInfo.transportLayerName ? transportLayerInfo.transportLayerName : "" );
    m_pImpl->m_transportLayerInfo.transportLayerModelName.assign(transportLayerInfo.transportLayerModelName ? transportLayerInfo.transportLayerModelName : "");
    m_pImpl->m_transportLayerInfo.transportLayerVendor.assign(transportLayerInfo.transportLayerVendor ? transportLayerInfo.transportLayerVendor : "");
    m_pImpl->m_transportLayerInfo.transportLayerVersion.assign(transportLayerInfo.transportLayerVersion ? transportLayerInfo.transportLayerVersion : "");
    m_pImpl->m_transportLayerInfo.transportLayerPath.assign(transportLayerInfo.transportLayerPath ? transportLayerInfo.transportLayerPath : "");
    m_pImpl->m_getInterfacesByTLFunc = std::move(getInterfacesByTLFunc);
    m_pImpl->m_getCamerasByTLFunc = std::move(getCamerasByTLFunc);
    SetHandle(transportLayerInfo.transportLayerHandle);
}

TransportLayer::~TransportLayer() noexcept
{
    Reset();
    RevokeHandle();
}

// HINT: This information remains static throughout the object's lifetime
VmbErrorType TransportLayer::GetID(char* const pStrID, VmbUint32_t& rnLength) const noexcept
{
    return CopyToBuffer(m_pImpl->m_transportLayerInfo.transportLayerIdString, pStrID, rnLength);
}

// HINT: This information remains static throughout the object's lifetime
VmbErrorType TransportLayer::GetName(char* const rName, VmbUint32_t& rnLength) const noexcept
{
    return CopyToBuffer(m_pImpl->m_transportLayerInfo.transportLayerName, rName, rnLength);
}

// HINT: This information remains static throughout the object's lifetime
VmbErrorType TransportLayer::GetModelName(char* const rModelName, VmbUint32_t& rnLength) const noexcept
{
    return CopyToBuffer(m_pImpl->m_transportLayerInfo.transportLayerModelName, rModelName, rnLength);
}

// HINT: This information remains static throughout the object's lifetime
VmbErrorType TransportLayer::GetVendor(char* const rVendor, VmbUint32_t& rnLength) const noexcept
{
    return CopyToBuffer(m_pImpl->m_transportLayerInfo.transportLayerVendor, rVendor, rnLength);
}

// HINT: This information remains static throughout the object's lifetime
VmbErrorType TransportLayer::GetVersion(char* const rVersion, VmbUint32_t& rnLength) const noexcept
{
    return CopyToBuffer(m_pImpl->m_transportLayerInfo.transportLayerVersion, rVersion, rnLength);
}

// HINT: This information remains static throughout the object's lifetime
VmbErrorType TransportLayer::GetPath(char* const rPath, VmbUint32_t& rnLength) const noexcept
{
    return CopyToBuffer(m_pImpl->m_transportLayerInfo.transportLayerPath, rPath, rnLength);
}

VmbErrorType TransportLayer::GetType(VmbTransportLayerType& reType) const noexcept
{
    reType = static_cast<VmbTransportLayerType>(m_pImpl->m_transportLayerInfo.transportLayerType);
    return VmbErrorSuccess;
}

VmbErrorType TransportLayer::GetInterfaces(InterfacePtr* pInterfaces, VmbUint32_t& size)
{
    return m_pImpl->m_getInterfacesByTLFunc(this, pInterfaces, size);
}

VmbErrorType TransportLayer::GetCameras(CameraPtr* pCameras, VmbUint32_t& size)
{
    return m_pImpl->m_getCamerasByTLFunc(this, pCameras, size);
}

}  // namespace VmbCPP
