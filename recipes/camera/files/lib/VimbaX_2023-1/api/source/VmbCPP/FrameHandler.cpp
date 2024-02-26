/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        FrameHandler.cpp

  Description: Implementation of class VmbCPP::FrameHandler.

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

#include "FrameHandler.h"

#include "MutexGuard.h"

#include <VmbCPP/LoggerDefines.h>


namespace VmbCPP {

FrameHandler::FrameHandler( FramePtr pFrame, IFrameObserverPtr pFrameObserver )
    :   m_pFrame( pFrame )
    ,   m_pObserver( pFrameObserver )
    ,   m_pMutex( new VmbCPP::Mutex() )
{
}

FramePtr FrameHandler::GetFrame() const
{
    return m_pFrame;
}

void VMB_CALL FrameHandler::FrameDoneCallback(const VmbHandle_t /*cameraHandle*/, const VmbHandle_t /*streamHandle*/, VmbFrame_t* frame )
{
    if ( nullptr != frame)
    {
        FrameHandler* pFrameHandler = reinterpret_cast<FrameHandler*>(frame->context[FRAME_HDL] );
        if ( nullptr != pFrameHandler)
        {            
            // Begin read lock frame handler
            MutexGuard local_lock( pFrameHandler->Mutex() );
            {
                IFrameObserverPtr pObs;
                if ( true == SP_ACCESS( pFrameHandler->m_pFrame )->GetObserver( pObs ))
                {
                    SP_ACCESS( pObs )->FrameReceived( pFrameHandler->m_pFrame );
                }
            }// scope to destroy observer pointer before releasing the lock
        }
        else // No FrameHandler
        {
            LOG_FREE_TEXT( "No frame handler passed. Frame has been removed from the frame queue." )
        }
    }
    else // pVmbFrame == null (Just for safety)
    {
        LOG_FREE_TEXT( "Received callback for already freed frame." )
    }
}

}
