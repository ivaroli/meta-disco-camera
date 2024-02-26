/*=============================================================================
  Copyright (C) 2022 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------
 
  File:        Interface.cpp

  Description: Implementation of class VmbCPP::Interface.

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
#include <map>
#include <string>
#include <utility>

#include <VmbCPP/Interface.h>

#include "CopyUtils.hpp"


namespace VmbCPP {

struct Interface::Impl
{
    /**
    * \brief Copy of interface infos
    */
    struct InterfaceInfo
    {
        /**
        * \name InterfaceInfo
        * \{
        */
        std::string             interfaceIdString;  //!< Unique identifier for each interface
        VmbTransportLayerType_t interfaceType;      //!< Interface type
        std::string             interfaceName;      //!< Interface name, given by the transport layer
        /**
        * \}
        */
    } m_interfaceInfo;
    
    TransportLayerPtr m_pTransportLayerPtr;                     //!< Pointer to the related transport layer object
    GetCamerasByInterfaceFunction m_getCamerasByInterfaceFunc;
};

Interface::Interface(const VmbInterfaceInfo_t& interfaceInfo, const TransportLayerPtr& pTransportLayerPtr, GetCamerasByInterfaceFunction getCamerasByInterface)
    : m_pImpl( new Impl() )
{
    m_pImpl->m_interfaceInfo.interfaceIdString.assign( interfaceInfo.interfaceIdString ? interfaceInfo.interfaceIdString : "" );
    m_pImpl->m_interfaceInfo.interfaceName.assign( interfaceInfo.interfaceName ? interfaceInfo.interfaceName : "" );
    m_pImpl->m_interfaceInfo.interfaceType = interfaceInfo.interfaceType;
    m_pImpl->m_pTransportLayerPtr = pTransportLayerPtr;
    m_pImpl->m_getCamerasByInterfaceFunc = std::move(getCamerasByInterface);
    SetHandle(interfaceInfo.interfaceHandle);
}

Interface::~Interface()
{
    Reset();
    RevokeHandle();
}

VmbErrorType Interface::GetID( char* const pStrID, VmbUint32_t& rnLength ) const noexcept
{
    return CopyToBuffer(m_pImpl->m_interfaceInfo.interfaceIdString, pStrID, rnLength);
}

VmbErrorType Interface::GetType( VmbTransportLayerType& reType ) const noexcept
{
    reType = static_cast<VmbTransportLayerType>(m_pImpl->m_interfaceInfo.interfaceType);

    return VmbErrorSuccess;
}

VmbErrorType Interface::GetName( char* const pStrName, VmbUint32_t& rnLength ) const noexcept
{
    return CopyToBuffer(m_pImpl->m_interfaceInfo.interfaceName, pStrName, rnLength);
}

VmbErrorType Interface::GetTransportLayer(TransportLayerPtr& reTransportLayer) const
{
    if (SP_ISNULL(m_pImpl->m_pTransportLayerPtr))
    {
        return VmbErrorNotAvailable;
    }
    reTransportLayer = m_pImpl->m_pTransportLayerPtr;

    return VmbErrorSuccess;
}

VmbErrorType Interface::GetCameras(CameraPtr* pCameras, VmbUint32_t& size)
{
    return m_pImpl->m_getCamerasByInterfaceFunc(this, pCameras, size);
}

}  // namespace VmbCPP
