/*=============================================================================
  Copyright (C) 2022 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------
 
  File:        IFrameObserver.cpp

  Description: Implementation of class VmbCPP::IFrameObserver.

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

#include <VmbCPP/IFrameObserver.h>
#include <VmbCPP/Camera.h>


namespace VmbCPP {

    IFrameObserver::IFrameObserver(CameraPtr pCamera) : m_pCamera(pCamera)
    {
        if (!SP_ISNULL(pCamera))
        {
            StreamPtrVector streams;
            if (VmbErrorSuccess == SP_ACCESS(pCamera)->GetStreams(streams) && streams.size() > 0)
            {
                m_pStream = streams[0];
            }
        }
    }

    IFrameObserver::IFrameObserver(CameraPtr pCamera, StreamPtr pStream) : m_pCamera(pCamera), m_pStream(pStream) {}
    

}  // namespace VmbCPP
