/*=============================================================================
  Copyright (C) 2022 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------
 
  File:        Stream.cpp

  Description: Implementation of class VmbCPP::Stream.

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
#include <sstream>
#pragma warning(default:4996)
#include <cstring>
#include <limits>
#include <utility>

#include <VmbCPP/Stream.h>
#include <VmbCPP/LoggerDefines.h>

#include "ConditionHelper.h"
#include "FrameImpl.h"
#include "FrameHandler.h"
#include "Helper.h"
#include "MutexGuard.h"


namespace VmbCPP {

struct Stream::Impl {
  
    LockableVector<FrameHandlerPtr> m_frameHandlers;
    ConditionHelper                 m_conditionHelper;
    bool                            m_deviceIsOpen = false;

    VmbErrorType AppendFrameToVector(const FramePtr& frame);

};

VmbErrorType Stream::Impl::AppendFrameToVector(const FramePtr& rFrame)
{
    try
    {
        FrameHandlerPtr pFH(new FrameHandler(rFrame, SP_ACCESS(rFrame)->m_pImpl->m_pObserver));
        if (SP_ISNULL(pFH))
        {
            return VmbErrorResources;
        }
        SP_ACCESS(rFrame)->m_pImpl->m_frame.context[FRAME_HDL] = SP_ACCESS(pFH);
        m_frameHandlers.Vector.emplace_back(std::move(pFH));
        return VmbErrorSuccess;
    }
    catch (...)
    {
        return VmbErrorResources;
    }
}

Stream::Stream(VmbHandle_t streamHandle, bool deviceIsOpen)
    :m_pImpl(new Impl())
{
    SetHandle(streamHandle);
    m_pImpl->m_deviceIsOpen = deviceIsOpen;
}

Stream::~Stream()
{
    Close();
}

VmbErrorType Stream::Open()
{
    if (nullptr == GetHandle())
    {
        return VmbErrorDeviceNotOpen;
    }
    m_pImpl->m_deviceIsOpen = true;
    return VmbErrorSuccess;
}

VmbErrorType Stream::Close()
{
    if (true != m_pImpl->m_deviceIsOpen)
    {
        return VmbErrorDeviceNotOpen;
    }
    VmbErrorType res = VmbErrorSuccess;

    if (0 < m_pImpl->m_frameHandlers.Vector.size())
    {
        VmbErrorType res = EndCapture();
        if (VmbErrorSuccess != res && VmbErrorAlready != res)
        {
            LOG_FREE_TEXT("Could not successfully end capturing");
        }
        res = FlushQueue();
        if (VmbErrorSuccess != res && VmbErrorAlready != res)
        {
            LOG_FREE_TEXT("Could not successfully flush queue");
        }
        res = RevokeAllFrames();
        if (VmbErrorSuccess != res && VmbErrorAlready != res)
        {
            LOG_FREE_TEXT("Could not successfully revoke all frames");
        }
        m_pImpl->m_frameHandlers.Vector.clear();
    }
    Reset();
    RevokeHandle();
    m_pImpl->m_deviceIsOpen = false;
    return res;
}

VmbErrorType Stream::AnnounceFrame(const FramePtr& frame)
{
    if (true != m_pImpl->m_deviceIsOpen)
    {
        return VmbErrorDeviceNotOpen;
    }
    if (SP_ISNULL(frame))
    {
        return VmbErrorBadParameter;
    }

    if (true == SP_ACCESS(frame)->m_pImpl->m_bAlreadyAnnounced
     || true == SP_ACCESS(frame)->m_pImpl->m_bAlreadyQueued)
    {
        return VmbErrorInvalidCall;
    }

    VmbError_t res = VmbFrameAnnounce(GetHandle(), &(SP_ACCESS(frame)->m_pImpl->m_frame), sizeof SP_ACCESS(frame)->m_pImpl->m_frame);

    if (VmbErrorSuccess == res)
    {
        // Begin write lock frame handler list
        if (true == m_pImpl->m_conditionHelper.EnterWriteLock(m_pImpl->m_frameHandlers))
        {
            res = m_pImpl->AppendFrameToVector(frame);
            if (VmbErrorSuccess == res)
            {
                SP_ACCESS(frame)->m_pImpl->m_bAlreadyAnnounced = true;
            }
            else
            {
                LOG_FREE_TEXT("could not append frame to internal vector");
            }
            // End write lock frame handler list
            m_pImpl->m_conditionHelper.ExitWriteLock(m_pImpl->m_frameHandlers);
        }
        else
        {
            LOG_FREE_TEXT("Could not lock announced frame queue for appending frame.");
            res = VmbErrorResources;
        }
    }

    return static_cast<VmbErrorType>(res);
}


VmbErrorType Stream::RevokeFrame(const FramePtr& frame)
{
    if (true != m_pImpl->m_deviceIsOpen)
    {
        return VmbErrorDeviceNotOpen;
    }
    if (SP_ISNULL(frame))
    {
        return VmbErrorBadParameter;
    }

    VmbError_t res = VmbFrameRevoke(GetHandle(), &(SP_ACCESS(frame)->m_pImpl->m_frame));

    if (VmbErrorSuccess == res)
    {
        // Begin (exclusive) write lock frame handler list
        if (true == m_pImpl->m_conditionHelper.EnterWriteLock(m_pImpl->m_frameHandlers, true))
        {
            // Dequeue, revoke and delete frame
            for (FrameHandlerPtrVector::iterator iter = m_pImpl->m_frameHandlers.Vector.begin();
                m_pImpl->m_frameHandlers.Vector.end() != iter;)
            {
                // Begin exclusive write lock frame handler
                MutexGuard lockal_lock(SP_ACCESS((*iter))->Mutex());
                if (SP_ISEQUAL(frame, SP_ACCESS((*iter))->GetFrame()))
                {
                    SP_ACCESS(frame)->m_pImpl->m_frame.context[FRAME_HDL] = nullptr;
                    SP_ACCESS(frame)->m_pImpl->m_bAlreadyQueued = false;
                    SP_ACCESS(frame)->m_pImpl->m_bAlreadyAnnounced = false;
                    // End exclusive write lock frame handler
                    iter = m_pImpl->m_frameHandlers.Vector.erase(iter);
                    return VmbErrorSuccess;
                }
                else
                {
                    ++iter;
                }
            }

            // End (exclusive) write lock frame handler list
            m_pImpl->m_conditionHelper.ExitWriteLock(m_pImpl->m_frameHandlers);
        }
        else
        {
            LOG_FREE_TEXT("Could not lock announced frame queue for removing frame.");
            res = VmbErrorResources;
        }
    }
    else
    {
        LOG_FREE_TEXT("Could not revoke frames")
    }

    return (VmbErrorType)res;
}

VmbErrorType Stream::RevokeAllFrames()
{
    if (true != m_pImpl->m_deviceIsOpen)
    {
        return VmbErrorDeviceNotOpen;
    }

    VmbError_t res = VmbFrameRevokeAll(GetHandle());

    if (VmbErrorSuccess == res)
    {
        // Begin (exclusive) write lock frame handler list
        if (true == m_pImpl->m_conditionHelper.EnterWriteLock(m_pImpl->m_frameHandlers, true))
        {
            // Dequeue, revoke and delete frames
            for (FrameHandlerPtrVector::iterator iter = m_pImpl->m_frameHandlers.Vector.begin();
                m_pImpl->m_frameHandlers.Vector.end() != iter;
                ++iter)
            {
                // Begin exclusive write lock frame handler
                MutexGuard  local_lock(SP_ACCESS((*iter))->Mutex());
                SP_ACCESS(SP_ACCESS((*iter))->GetFrame())->m_pImpl->m_frame.context[FRAME_HDL] = nullptr;
                SP_ACCESS(SP_ACCESS((*iter))->GetFrame())->m_pImpl->m_bAlreadyQueued = false;
                SP_ACCESS(SP_ACCESS((*iter))->GetFrame())->m_pImpl->m_bAlreadyAnnounced = false;
                // End exclusive write lock frame handler
            }

            m_pImpl->m_frameHandlers.Vector.clear();

            // End exclusive write lock frame handler list
            m_pImpl->m_conditionHelper.ExitWriteLock(m_pImpl->m_frameHandlers);
        }
        else
        {
            LOG_FREE_TEXT("Could not lock frame handler list.")
        }
    }

    return (VmbErrorType)res;
}

VmbErrorType Stream::QueueFrame(const FramePtr& frame)
{
    if (true != m_pImpl->m_deviceIsOpen)
    {
        return VmbErrorDeviceNotOpen;
    }
    if (SP_ISNULL(frame))
    {
        return VmbErrorBadParameter;
    }

    // HINT: The same frame cannot be queued twice (VmbErrorOther)
    // Don't pass a FrameCallback for synchronous acquisition
    VmbError_t res = VmbCaptureFrameQueue(GetHandle(), &(SP_ACCESS(frame)->m_pImpl->m_frame), SP_ACCESS(frame)->m_pImpl->m_bSynchronousGrab ? nullptr : FrameHandler::FrameDoneCallback);

    if (VmbErrorSuccess == res
        && false == SP_ACCESS(frame)->m_pImpl->m_bAlreadyQueued)
    {
        if (false == SP_ACCESS(frame)->m_pImpl->m_bAlreadyAnnounced)
        {
            // Begin write lock frame handler list
            if (true == m_pImpl->m_conditionHelper.EnterWriteLock(m_pImpl->m_frameHandlers))
            {
                m_pImpl->AppendFrameToVector(frame);
                SP_ACCESS(frame)->m_pImpl->m_bAlreadyQueued = true;

                // End write lock frame handler list
                m_pImpl->m_conditionHelper.ExitWriteLock(m_pImpl->m_frameHandlers);
            }
            else
            {
                LOG_FREE_TEXT("Could not lock frame queue for appending frame.");
                res = VmbErrorResources;
            }
        }
    }

    return static_cast<VmbErrorType>(res);
}

VmbErrorType Stream::FlushQueue()
{
    if (true != m_pImpl->m_deviceIsOpen)
    {
        return VmbErrorDeviceNotOpen;
    }
    VmbError_t res = VmbCaptureQueueFlush(GetHandle());

    if (VmbErrorSuccess == res)
    {
        // Begin exclusive write lock frame handler list
        if (true == m_pImpl->m_conditionHelper.EnterWriteLock(m_pImpl->m_frameHandlers, true))
        {
            for (FrameHandlerPtrVector::iterator iter = m_pImpl->m_frameHandlers.Vector.begin();
                m_pImpl->m_frameHandlers.Vector.end() != iter;)
            {
                // Begin exclusive write lock of every single frame handler
                MutexPtr tmpMutex = SP_ACCESS((*iter))->Mutex();
                SP_ACCESS(tmpMutex)->Lock();
                //SP_ACCESS(( *iter)) ->Mutex()->Lock();
                // Dequeue frame
                SP_ACCESS(SP_ACCESS((*iter))->GetFrame())->m_pImpl->m_bAlreadyQueued = false;
                if (false == SP_ACCESS(SP_ACCESS((*iter))->GetFrame())->m_pImpl->m_bAlreadyAnnounced)
                {
                    // Delete frame if it was not announced / was revoked before
                    SP_ACCESS(SP_ACCESS((*iter))->GetFrame())->m_pImpl->m_frame.context[FRAME_HDL] = nullptr;
                    // End write lock frame handler
                    SP_ACCESS(tmpMutex)->Unlock();
                    iter = m_pImpl->m_frameHandlers.Vector.erase(iter);
                }
                else
                {
                    // End write lock frame handler
                    SP_ACCESS(tmpMutex)->Unlock();
                    ++iter;
                }
            }
            // End write lock frame handler list
            m_pImpl->m_conditionHelper.ExitWriteLock(m_pImpl->m_frameHandlers);
        }
        else
        {
            LOG_FREE_TEXT("Could not lock frame handler list.")
        }
    }
    else
    {
        LOG_FREE_TEXT("Could not flush frame queue")
    }

    return static_cast<VmbErrorType>(res);
}


VmbErrorType Stream::StartCapture() noexcept
{
    if (true != m_pImpl->m_deviceIsOpen)
    {
        return VmbErrorDeviceNotOpen;
    }
    return static_cast<VmbErrorType>(VmbCaptureStart(GetHandle()));
}

VmbErrorType Stream::EndCapture() noexcept
{
    if (true != m_pImpl->m_deviceIsOpen)
    {
        return VmbErrorDeviceNotOpen;
    }
    return static_cast<VmbErrorType>(VmbCaptureEnd(GetHandle()));
}

VmbErrorType Stream::GetStreamBufferAlignment(VmbUint32_t& nBufferAlignment)
{
    if (true != m_pImpl->m_deviceIsOpen)
    {
        return VmbErrorDeviceNotOpen;
    }
    FeaturePtr feat;
    if (VmbErrorSuccess == GetFeatureByName("StreamBufferAlignment", feat))
    {
        VmbInt64_t value;
        VmbError_t err;
        if (VmbErrorSuccess == (err = feat->GetValue(value)))
        {            
            if (value >= 0 && value <= (std::numeric_limits<VmbUint32_t>::max)())
            {
                nBufferAlignment = static_cast<VmbUint32_t>(value);
                return VmbErrorSuccess;
            }
            else
            {
                return VmbErrorInvalidValue;
            }
        }
        else
        {
            return static_cast<VmbErrorType>(err);
        }
    }

    nBufferAlignment = 1;
    return VmbErrorSuccess;
}

}  // namespace VmbCPP
