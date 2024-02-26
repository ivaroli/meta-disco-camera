/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        Frame.cpp

  Description: Implementation of class VmbCPP::Frame.

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

#include <VmbCPP/Frame.h>

#include <VmbCPP/LoggerDefines.h>
#include <VmbCPP/SharedPointerDefines.h>
#include <VmbCPP/VmbSystem.h>

#include "ConditionHelper.h"
#include "FrameImpl.h"
#include "MutexGuard.h"

#include <stdlib.h>


namespace
{
    void* VmbAlignedAlloc(size_t alignment, size_t size)
    {
#ifdef _WIN32
        return _aligned_malloc(size, alignment);
#else
        return aligned_alloc(alignment, size);
#endif
    }

    void VmbAlignedFree(void* buffer)
    {
#ifdef _WIN32
        _aligned_free(buffer);
#else
        free(buffer);
#endif
    }
}

namespace VmbCPP {

Frame::Frame( VmbInt64_t nBufferSize, FrameAllocationMode allocationMode /*=FrameAllocation_AnnounceFrame*/, VmbUint32_t bufferAlignment /*=1*/ )
    :   m_pImpl( new Impl() )
{
    m_pImpl->m_bAlreadyAnnounced = false;
    m_pImpl->m_bAlreadyQueued = false;
    m_pImpl->m_bIsSelfAllocatedBuffer = (allocationMode == FrameAllocation_AnnounceFrame) ? true : false;
    m_pImpl->m_bSynchronousGrab = false;
    SP_SET( m_pImpl->m_pObserverMutex, new Mutex() );
    m_pImpl->Init();
    m_pImpl->m_frame.buffer = (allocationMode == FrameAllocation_AnnounceFrame) ? static_cast<VmbUchar_t*>(VmbAlignedAlloc(bufferAlignment, nBufferSize)) : nullptr;
    m_pImpl->m_frame.bufferSize = static_cast<VmbUint32_t>(nBufferSize);
}

Frame::Frame( VmbUchar_t *pBuffer, VmbInt64_t nBufferSize )
    :   m_pImpl( new Impl() )
{
    m_pImpl->m_bAlreadyAnnounced = false;
    m_pImpl->m_bAlreadyQueued = false;
    m_pImpl->m_bIsSelfAllocatedBuffer = false;
    m_pImpl->m_bSynchronousGrab = false;
    m_pImpl->m_frame.buffer = nullptr;
    SP_SET( m_pImpl->m_pObserverMutex, new Mutex());
    m_pImpl->Init();
    if ( nullptr != pBuffer )
    {
        m_pImpl->m_frame.buffer = pBuffer;
        m_pImpl->m_frame.bufferSize = static_cast<VmbUint32_t>(nBufferSize);
    }
    else
    {
        // Do some logging
        LOG_FREE_TEXT( "No valid buffer passed when constructing frame." )
    }
}

void Frame::Impl::Init()
{
    m_frame.imageData = nullptr;
    m_frame.payloadType = VmbPayloadTypeUnknown;
    m_frame.chunkDataPresent = false;
    m_frame.buffer = nullptr;
    m_frame.bufferSize = 0;
    for ( int i=0; i<4; ++i)
    {
        m_frame.context[i] = nullptr;
    }
    m_frame.frameID = 0;
    m_frame.height = 0;
    m_frame.offsetX = 0;
    m_frame.offsetY = 0;
    m_frame.pixelFormat = 0;
    m_frame.receiveFlags = VmbFrameFlagsNone;
    m_frame.receiveStatus = VmbFrameStatusInvalid;
    m_frame.timestamp = 0;
    m_frame.width = 0;
}

Frame::~Frame()
{
    UnregisterObserver();
    if (    true == m_pImpl->m_bIsSelfAllocatedBuffer
         && nullptr != m_pImpl->m_frame.buffer)
    {
        VmbAlignedFree(m_pImpl->m_frame.buffer);
    }
}

VmbErrorType Frame::RegisterObserver( const IFrameObserverPtr &rObserver )
{
    if ( SP_ISNULL( rObserver ))
    {
        return VmbErrorBadParameter;
    }

    // Begin exclusive write lock observer
    MutexGuard local_lock( m_pImpl->m_pObserverMutex );
    m_pImpl->m_pObserver = rObserver;
    return VmbErrorSuccess;
}

VmbErrorType Frame::UnregisterObserver()
{
    VmbErrorType res = VmbErrorSuccess;

    // Begin exclusive write lock observer
    MutexGuard local_lock( m_pImpl->m_pObserverMutex );
    if ( SP_ISNULL( m_pImpl->m_pObserver ))
    {
        res = VmbErrorNotFound;
    }
    else
    {
        SP_RESET( m_pImpl->m_pObserver );
    }
    return res;
}

bool Frame::GetObserver( IFrameObserverPtr &rObserver ) const
{
    MutexGuard local_lock( m_pImpl->m_pObserverMutex );
    if ( SP_ISNULL( m_pImpl->m_pObserver ))
    {
        return false;
    }
    rObserver = m_pImpl->m_pObserver;
    return true;
}

VmbErrorType Frame::GetBuffer( VmbUchar_t* &rpBuffer )
{
    rpBuffer = static_cast<VmbUchar_t*>(m_pImpl->m_frame.buffer);

    return VmbErrorSuccess;
}

VmbErrorType Frame::GetBuffer( const VmbUchar_t* &rpBuffer ) const
{
    rpBuffer = static_cast<const VmbUchar_t*>(m_pImpl->m_frame.buffer);

    return VmbErrorSuccess;
}

VmbErrorType Frame::GetImage( VmbUchar_t* &rpBuffer )
{
    rpBuffer = m_pImpl->m_frame.imageData;

    return VmbErrorSuccess;
}

VmbErrorType Frame::GetImage( const VmbUchar_t* &rpBuffer ) const
{
    rpBuffer = m_pImpl->m_frame.imageData;

    return VmbErrorSuccess;
}

VmbErrorType Frame::GetReceiveStatus( VmbFrameStatusType &rStatus ) const
{
    rStatus = static_cast<VmbFrameStatusType>(m_pImpl->m_frame.receiveStatus);

    return VmbErrorSuccess;
}

VmbErrorType Frame::GetPayloadType( VmbPayloadType& payloadType ) const
{
    if (m_pImpl->m_frame.receiveFlags & VmbFrameFlagsPayloadType)
    {
        payloadType = static_cast<VmbPayloadType>(m_pImpl->m_frame.payloadType);
        return VmbErrorSuccess;
    }

    return VmbErrorNotAvailable;
}

VmbErrorType Frame::GetBufferSize( VmbUint32_t &rnBufferSize ) const
{
    rnBufferSize = m_pImpl-> m_frame.bufferSize;

    return VmbErrorSuccess;
}

VmbErrorType Frame::GetPixelFormat( VmbPixelFormatType &rPixelFormat ) const
{
    rPixelFormat = static_cast<VmbPixelFormatType>(m_pImpl->m_frame.pixelFormat);

    return VmbErrorSuccess;
}

VmbErrorType Frame::GetWidth( VmbUint32_t &rnWidth ) const
{
    if (m_pImpl->m_frame.receiveFlags & VmbFrameFlagsDimension)
    {
        rnWidth = m_pImpl->m_frame.width;
        return VmbErrorSuccess;
    }

    return VmbErrorNotAvailable;
}

VmbErrorType Frame::GetHeight( VmbUint32_t &rnHeight ) const
{
    if (m_pImpl->m_frame.receiveFlags & VmbFrameFlagsDimension)
    {
        rnHeight = m_pImpl->m_frame.height;
        return VmbErrorSuccess;
    }

    return VmbErrorNotAvailable;
}

VmbErrorType Frame::GetOffsetX( VmbUint32_t &rnOffsetX ) const
{
    if (m_pImpl->m_frame.receiveFlags & VmbFrameFlagsOffset)
    {
        rnOffsetX = m_pImpl->m_frame.offsetX;
        return VmbErrorSuccess;
    }

    return VmbErrorNotAvailable;
}

VmbErrorType Frame::GetOffsetY( VmbUint32_t &rnOffsetY ) const
{
    if (m_pImpl->m_frame.receiveFlags & VmbFrameFlagsOffset)
    {
        rnOffsetY = m_pImpl->m_frame.offsetY;
        return VmbErrorSuccess;
    }

    return VmbErrorNotAvailable;
}

VmbErrorType Frame::GetFrameID( VmbUint64_t &rnFrameID ) const
{
    if (m_pImpl->m_frame.receiveFlags & VmbFrameFlagsFrameID)
    {
        rnFrameID = m_pImpl->m_frame.frameID;
        return VmbErrorSuccess;
    }

    return VmbErrorNotAvailable;
}

VmbErrorType Frame::GetTimestamp( VmbUint64_t &rnTimestamp ) const
{
    if (m_pImpl->m_frame.receiveFlags & VmbFrameFlagsTimestamp)
    {
        rnTimestamp = m_pImpl->m_frame.timestamp;
        return VmbErrorSuccess;
    }

    return VmbErrorNotAvailable;
}

VmbErrorType Frame::GetFrameStruct(VmbFrame_t*& frame)
{
    frame = &m_pImpl->m_frame;

    return VmbErrorSuccess;
}

VmbErrorType Frame::ChunkDataAccess(const VmbFrame_t* frame, VmbChunkAccessCallback chunkAccessCallback, void* userContext)
{
    if ((m_pImpl->m_frame.receiveFlags & VmbFrameFlagsChunkDataPresent) && !m_pImpl->m_frame.chunkDataPresent)
    {
        return VmbErrorNotAvailable;
    }

    return static_cast<VmbErrorType>(VmbChunkDataAccess(frame, chunkAccessCallback, userContext));
}

}  // namespace VmbCPP
