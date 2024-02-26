/*=============================================================================
  Copyright (C) 2012 - 2022 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------
 
  File:        Camera.cpp

  Description: Implementation of class VmbCPP::Camera.

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

#include <VmbCPP/Camera.h>

#include <VmbCPP/LoggerDefines.h>

#include "CopyUtils.hpp"
#include "FrameImpl.h"
#include "Helper.h"
#include "MutexGuard.h"


namespace VmbCPP {

/**
*
* \brief  helper to run a command feature for camera.
*
*
* \param[in] cam        camera to run command on
* \param[in] name       command name to run
*/
VmbErrorType RunFeatureCommand( Camera&cam, const char* name)
{
    if(nullptr == name)
    {
        LOG_FREE_TEXT("feature name is null");
        return VmbErrorBadParameter;
    }
    FeaturePtr      pFeature;
    VmbErrorType    res         = cam.GetFeatureByName( name, pFeature );
    if ( VmbErrorSuccess != res )
    {
        LOG_ERROR(std::string("Could not get feature by name for ") + name, res);
        return res;
    }
    res = SP_ACCESS(pFeature)->RunCommand();
    if( VmbErrorSuccess != res)
    {
        LOG_ERROR(std::string("Could not run feature command ") + name , res);
    }
    return res;
}

/**
* \brief  small helper class that keeps track of resources needed for image acquisition
*/
struct AcquireImageHelper
{
private:
    //clean up tasks
    enum tear_down_tasks
    {
        RevokeFrame,
        FlushQueue,
        EndCapture,
        AcquisitionStop,
    };
    typedef std::vector<tear_down_tasks>    task_storage;
    task_storage                            m_Tasks;        // storage for cleanup tasks
    Camera&                                 m_Camera;

    ///get the top most taks and pop it from stack
    tear_down_tasks GetTask()
    {
        tear_down_tasks current_task = m_Tasks.back();
        m_Tasks.pop_back();
        return current_task;
    }

    const AcquireImageHelper& operator=( const AcquireImageHelper &o);

    /**
    * \brief  prepare a frame with given payload size.
    *
    * \param[in,out] pFrame         a frame pointer that can point to Null
    * \param[in]     payload_size   payload size for frame
    * \param[in]     allocationMode     frame allocation mode
    * \param[in]     bufferAlignment    buffer alignment
    */
    static VmbErrorType SetupFrame(FramePtr &pFrame, VmbInt64_t PayloadSize, FrameAllocationMode allocationMode, VmbUint32_t bufferAlignment)
    {
        if( PayloadSize <= 0)
        {
            LOG_FREE_TEXT("payload size has to be larger than 0");
            return VmbErrorBadParameter;
        }
        VmbUint32_t     buffer_size(0);
        VmbErrorType    Result;
        if( ! SP_ISNULL( pFrame) )  // if frame already exists, check its buffer size
        {
            Result = SP_ACCESS( pFrame) ->GetBufferSize(buffer_size);
            if( VmbErrorSuccess != Result)
            {
                LOG_ERROR("Could not get frame buffer size", Result);
                return Result;
            }
            if( buffer_size >= PayloadSize) // buffer is large enough, no need to create new frame
            {
                return VmbErrorSuccess;
            }
        }
        try
        {
            SP_SET( pFrame, new Frame( PayloadSize, allocationMode, bufferAlignment));
            if( SP_ISNULL( pFrame) ) // in case we find a not throwing new
            {
                LOG_FREE_TEXT("error allocating frame");
                return VmbErrorResources;
            }
        }
        catch(...)
        {
            LOG_FREE_TEXT("error allocating frame");
            return VmbErrorResources;
        }
        return VmbErrorSuccess;
    }
public:
    // construct helper from camera
    AcquireImageHelper(Camera &Cam)
        : m_Camera( Cam)
    {}
    // destroy will tear all down
    ~AcquireImageHelper()
    {
        TearDown();
    }
    
    /**
    *
    * \brief     helper to announce a list of frames to the camera.
    *
    * \details   note the function will try to construct and announce nFrameCount frames t o the camera, even if some of them can not be created or announced, only if nFramesAnnounced == 0 the function was unsuccessful
    *
    * \param[in]        Camera              Camera to announce the frames too
    * \param[in,out]    pFrames             storage for frame pointer, if they are null or have no sufficient space the frames will be created
    * \param[in]        nFrameCount         number of frame pointers in pFrames
    * \param[in]        nPayloadSize        payload size for one frame
    * \param[out]       nFramesAnnounced    returns number of successful announced frames
    * \param[in]        allocationMode      frame allocation mode
    * \param[in]        bufferAlignment     buffer alignment
    *
    * \returns   the first error that occurred or ::VmbErrorSuccess if non occurred 
    */
    static VmbErrorType AnnounceFrames(Camera &Camera, FramePtr *pFrames, VmbUint32_t nFrameCount, VmbInt64_t nPayloadSize, VmbUint32_t &nFramesAnnounced, FrameAllocationMode allocationMode, VmbUint32_t bufferAlignment)
    {
        VmbErrorType    Result  = VmbErrorSuccess;
        nFramesAnnounced        = 0;
        for( VmbUint32_t FrameNumber= 0; FrameNumber < nFrameCount; ++FrameNumber)
        {
            VmbErrorType LocalResult = SetupFrame( pFrames[ FrameNumber ], nPayloadSize, allocationMode, bufferAlignment);         //< try to init frame
            if( VmbErrorSuccess == LocalResult)
            {
                LocalResult = Camera.AnnounceFrame( pFrames[ FrameNumber] );       //< announce frame if successful initialized
                if ( VmbErrorSuccess == LocalResult )
                {
                    ++nFramesAnnounced;
                }
                else
                {
                    std::stringstream strMsg("Could only successfully announce ");
                    strMsg << nFramesAnnounced << " of " <<  nFrameCount  << " frames. Will continue with queuing those.";
                    LOG_FREE_TEXT( strMsg.str() );
                }
            }
            if( VmbErrorSuccess == Result )
            {
                Result = LocalResult;
            }
        }
        return Result;
    }
    
    /**
    *
    * \brief  announce a FramePtrVector to the camera.
    *
    * \param[in]        Camera          camera to announce the frames to
    * \param[in,out]    Frames          vector of frame pointers that will contain the announced frames on return, can be empty on input
    * \param[in]        nBufferCount    number of frames to announce, if nBufferCount > Frames.size() on return, some frames could not be announced
    * \param[in]        nPayloadSize    frame payload size
    * \param[in]        Observer        observer to attach to frames
    * \param[in]        allocationMode  frame allocation mode
    * \param[in]        bufferAlignment buffer alignment
    */
    static VmbErrorType AnnounceFrames(Camera &Camera, FramePtrVector &Frames, VmbUint32_t nBufferCount, VmbInt64_t nPayloadSize, const IFrameObserverPtr& Observer, FrameAllocationMode allocationMode, VmbUint32_t bufferAlignment)
    {
        try
        {
            Frames.reserve( nBufferCount);
        }
        catch(...)
        {
            LOG_FREE_TEXT("could not allocate frames");
            return VmbErrorResources;
        }
        VmbErrorType Result = VmbErrorSuccess;
        for( VmbUint32_t i=0; i < nBufferCount; ++i)
        {
            FramePtr tmpFrame;
            VmbErrorType LocalResult = SetupFrame( tmpFrame, nPayloadSize, allocationMode, bufferAlignment );
            if( ! SP_ISNULL( tmpFrame) )
            {
                LocalResult = SP_ACCESS( tmpFrame)->RegisterObserver( Observer );
                if( VmbErrorSuccess == LocalResult )
                {
                    LocalResult = Camera.AnnounceFrame( tmpFrame);
                    if( VmbErrorSuccess == LocalResult )
                    {
                        Frames.emplace_back(std::move(tmpFrame));
                    }
                    else
                    {
                        LOG_ERROR("could not announce frame", LocalResult);
                    }
                }
                else
                {
                    LOG_ERROR("could not register frame observer", LocalResult);
                }
            }
            else
            {
                LOG_ERROR("could not allocate frame", LocalResult);
            }
            if( VmbErrorSuccess == Result)
            {
                Result = LocalResult;
            }
        }
        return Result;
    }

    /**
    *
    * \brief  prepare image grab for single image.
    *
    * \param[in,out]    pFrame                      frame to hold the image
    * \param[in]        PayloadSize                 frame payload size
    * \param[in]        allocationMode              frame allocation mode
    * \param[in]        bufferAlignmnet             frame buffer alignment
    * \param[in]        SetFrameSynchronousFlag     function pointer for setting the synchronous acquisition flag in the frame impl
    */
    VmbErrorType Prepare(FramePtr &pFrame, VmbInt64_t PayloadSize, FrameAllocationMode allocationMode, VmbUint32_t bufferAlignment, void (*SetFrameSynchronousFlag)(FramePtr& frame))
    {
        VmbErrorType res;
        res = SetupFrame( pFrame, PayloadSize, allocationMode, bufferAlignment );     // init frame if necessary
        if ( VmbErrorSuccess != res )
        {
            LOG_ERROR("Could not create frame", res);
            return res;
        }
        res = m_Camera.AnnounceFrame( pFrame );                     // announce frame to camera
        if ( VmbErrorSuccess != res )
        {
            LOG_ERROR("Could not Announce frame", res);
            return res;
        }
        m_Tasks.push_back( RevokeFrame);                            // if successful announced we need to revoke frames
        res = m_Camera.StartCapture();                              // start capture logic
        if ( VmbErrorSuccess != res )
        {
            LOG_ERROR( "Could not Start Capture", res);
            return res;
        }
        m_Tasks.push_back( EndCapture);                             // if capture logic is started we need end capture task
                
        SetFrameSynchronousFlag(pFrame);                            // Set Synchronous flag before queuing the frame

        res = m_Camera.QueueFrame( pFrame );                        // queue frame in processing logic
        if ( VmbErrorSuccess != res )
        {
            LOG_ERROR( "Could not queue frame", res );
            return res;
        }
        m_Tasks.pop_back();
        m_Tasks.push_back( FlushQueue);                             // if frame queued we need flush queue task
        m_Tasks.push_back( EndCapture);
        FeaturePtr pFeature;
        res = RunFeatureCommand( m_Camera, "AcquisitionStart" );    // start acquisition
        if ( VmbErrorSuccess != res )
        {
            LOG_ERROR("Could not run command AcquisitionStart", res);
            return res;
        }
        m_Tasks.push_back( AcquisitionStop);
        return res;
    }
    /**
    *
    * \brief  prepare image acquisition for multiple frames.
    *
    * \param[in,out]     pFrames                    non null pointer to field of frame pointers (can point to null) that hold the captured images
    * \param[in]         nFrameCount                number of frames in vector
    * \param[in]         nPayLoadSize               payload size
    * \param[out]        nFramesQueued              returns number of successful queued images
    * \param[in]         allocationMode             frame allocation mode
    * \param[in]         bufferAlignment            frame buffer alignment
    * \param[in]         SetFrameSynchronousFlag    function pointer for setting the synchronous acquisition flag in the frame impl
    */
    VmbErrorType Prepare(FramePtr *pFrames, VmbUint32_t nFrameCount, VmbInt64_t nPayloadSize, VmbUint32_t &nFramesQueued, FrameAllocationMode allocationMode, VmbUint32_t bufferAlignment, void (*SetFrameSynchronousFlag)(FramePtr& frame))
    {
        if(nullptr == pFrames || 0 == nFrameCount)                            // sanity check
        {
            return VmbErrorBadParameter;
        }
        nFramesQueued = 0;
        VmbErrorType    Result          = VmbErrorSuccess;
        VmbUint32_t     FramesAnnounced = 0;
        Result = AnnounceFrames( m_Camera, pFrames, nFrameCount, nPayloadSize, FramesAnnounced, allocationMode, bufferAlignment);
        if( 0 == FramesAnnounced)
        {
            return Result;
        }
        m_Tasks.push_back( RevokeFrame);                                    // add cleanup task for announced frames
        Result = m_Camera.StartCapture();                                   // start capture logic
        if ( VmbErrorSuccess != Result)
        {
            LOG_ERROR( "Could not Start Capture", Result );
            return Result;
        }
        m_Tasks.push_back( EndCapture);                                     // add cleanup task to end capture
        for( VmbUint32_t FrameNumber = 0; FrameNumber < FramesAnnounced; ++FrameNumber)
        {
            SetFrameSynchronousFlag(pFrames[FrameNumber]);                  // Set Synchronous flag before queuing the frame
            Result = m_Camera.QueueFrame( pFrames[ FrameNumber ] );         // try queuing frame
            if ( VmbErrorSuccess != Result )
            {
                std::stringstream strMsg("Could only successfully queue ");
                strMsg << nFramesQueued << " of " << nFrameCount << " frames. Will continue with filling those.";
                LOG_ERROR( strMsg.str(), Result );
                break;
            }
            else
            {
                ++nFramesQueued;
            }
        }
        if( 0 == nFramesQueued) // we cannot capture anything, there are no frames queued
        {
            return Result;
        }
        m_Tasks.pop_back();
        m_Tasks.push_back( FlushQueue);                         // if any frame was queued we need a cleanup task
        m_Tasks.push_back( EndCapture);
        FeaturePtr pFeature;
        Result = RunFeatureCommand( m_Camera, "AcquisitionStart" ); // start acquisition logic
        if ( VmbErrorSuccess != Result )
        {
            LOG_ERROR("Could not run command AcquisitionStart", Result);
            return Result;
        }
        m_Tasks.push_back( AcquisitionStop);
        return Result;
    }
    
    /**
    *
    * \brief  free all acquired resources.
    */
    VmbErrorType TearDown()
    {
        VmbErrorType res = VmbErrorSuccess;
        while( ! m_Tasks.empty() )
        {
            VmbErrorType local_result = VmbErrorSuccess;
            switch( GetTask() )
            {
            case AcquisitionStop:
                    local_result = RunFeatureCommand(m_Camera, "AcquisitionStop");
                    if( VmbErrorSuccess != local_result)
                    {
                        LOG_ERROR("Could not run command AquireStop", local_result);
                    }
                    break;
            case EndCapture:
                    local_result = m_Camera.EndCapture();
                    if( VmbErrorSuccess != local_result)
                    {
                        LOG_ERROR("Could Not run EndCapture", local_result);
                    }
                    break;
            case FlushQueue:
                    local_result = m_Camera.FlushQueue();
                    if( VmbErrorSuccess != local_result)
                    {
                        LOG_ERROR("Could not run Flush Queue command", local_result);
                    }
                    break;
            case RevokeFrame:
                    local_result = m_Camera.RevokeAllFrames();
                    if( VmbErrorSuccess != local_result)
                    {
                        LOG_ERROR("Could Not Run Revoke Frames command", local_result);
                    }
                    break;
            }
            if( VmbErrorSuccess == res)
                res = local_result;
        }
        return res;
    }
};


struct Camera::Impl
{
    /**
    * \brief Copy of camera infos
    */
    struct CameraInfo
    {
        /**
        * \name CameraInfo
        * \{
        */
        std::string     cameraIdString;             //!< Identifier for each camera (maybe not unique with multiple transport layers and interfaces)
        std::string     cameraIdExtended;           //!< Globally unique identifier for the camera
        std::string     cameraName;                 //!< Name of the camera
        std::string     modelName;                  //!< Model name
        std::string     serialString;               //!< Serial number       
        /**
        * \}
        */
    } m_cameraInfo;

    MutexPtr                        m_pQueueFrameMutex;
    bool                            m_bAllowQueueFrame;

    InterfacePtr                    m_pInterface;       //<! Shared pointer to the interface the camera is connected to
    LocalDevicePtr                  m_pLocalDevice;     //<! Shared pointer to the local device
    StreamPtrVector                 m_streams;          //<! Verctor of available stream pointers in the same order, as it will be delivered by the VmbC camera opening function

};


Camera::Camera(const VmbCameraInfo_t& camInfo,
               const InterfacePtr& pInterface)
    : m_pImpl(new Impl())
{
    m_pImpl->m_cameraInfo.cameraIdString.assign(camInfo.cameraIdString ? camInfo.cameraIdString : "");
    m_pImpl->m_cameraInfo.cameraIdExtended.assign(camInfo.cameraIdExtended ? camInfo.cameraIdExtended : "");

    m_pImpl->m_pInterface = pInterface;
    m_pImpl->m_cameraInfo.cameraName.assign(camInfo.cameraName ? camInfo.cameraName : "");
    m_pImpl->m_cameraInfo.modelName.assign(camInfo.modelName ? camInfo.modelName : "");
    m_pImpl->m_cameraInfo.serialString.assign(camInfo.serialString ? camInfo.serialString : "");
    m_pImpl->m_bAllowQueueFrame = true;
    SP_SET(m_pImpl->m_pQueueFrameMutex, new Mutex);
}

Camera::~Camera()
{
    Close();
}

VmbErrorType Camera::Open( VmbAccessModeType eAccessMode )
{
    VmbError_t res;
    VmbHandle_t hHandle;

    res = VmbCameraOpen( m_pImpl->m_cameraInfo.cameraIdExtended.c_str(), (VmbAccessMode_t)eAccessMode, &hHandle );

    if ( VmbErrorSuccess == res )
    {
        SetHandle( hHandle );
        m_pImpl->m_streams.clear();
        VmbCameraInfo_t camInfo;
        res = VmbCameraInfoQueryByHandle(hHandle, &camInfo, sizeof(camInfo));
        if (VmbErrorSuccess == res)
        {
            // create stream objects
            for (VmbUint32_t i = 0; i < camInfo.streamCount; ++i)
            {
                VmbHandle_t sHandle = camInfo.streamHandles[i];
                if (nullptr != sHandle)
                {
                    StreamPtr pStream { new Stream(sHandle, (i == 0 ? true : false) ) };
                    m_pImpl->m_streams.emplace_back(pStream);
                }
            }

            //create local device object
            m_pImpl->m_pLocalDevice = LocalDevicePtr(new LocalDevice(camInfo.localDeviceHandle));
        }
    }

    return (VmbErrorType)res;
}

VmbErrorType Camera::Close()
{
    VmbError_t res = VmbErrorDeviceNotOpen;
    
    for (auto pStream : m_pImpl->m_streams)
    {
        SP_ACCESS(pStream)->Close();
    }
    m_pImpl->m_streams.clear();

    if (nullptr != GetHandle() )
    {
        Reset();
        res = VmbCameraClose( GetHandle() );
        RevokeHandle();
    }

    SP_RESET(m_pImpl->m_pLocalDevice);

    return static_cast<VmbErrorType>(res);
}

VmbErrorType Camera::GetID( char * const pStrID, VmbUint32_t &rnLength, bool extended ) const noexcept
{
    const std::string& strID = extended ?
        m_pImpl->m_cameraInfo.cameraIdExtended
        : m_pImpl->m_cameraInfo.cameraIdString;

    return CopyToBuffer(strID, pStrID, rnLength);
}

VmbErrorType Camera::GetName( char * const pStrName, VmbUint32_t &rnLength ) const noexcept
{
    return CopyToBuffer(m_pImpl->m_cameraInfo.cameraName, pStrName, rnLength);
}

VmbErrorType Camera::GetModel( char * const pStrModel, VmbUint32_t &rnLength ) const noexcept
{
    return CopyToBuffer(m_pImpl->m_cameraInfo.modelName, pStrModel, rnLength);
}

VmbErrorType Camera::GetSerialNumber( char * const pStrSerial, VmbUint32_t &rnLength ) const noexcept
{
    return CopyToBuffer(m_pImpl->m_cameraInfo.serialString, pStrSerial, rnLength);
}

VmbErrorType Camera::GetInterfaceType( VmbTransportLayerType &reInterfaceType ) const
{
    if (SP_ISNULL(m_pImpl->m_pInterface))
    {
        return VmbErrorNotAvailable;
    }

    return SP_ACCESS(m_pImpl->m_pInterface)->GetType(reInterfaceType);
}

VmbErrorType Camera::GetInterface(InterfacePtr &rInterface) const
{
    if (SP_ISNULL(m_pImpl->m_pInterface))
    {
        return VmbErrorNotAvailable;
    }
    rInterface = m_pImpl->m_pInterface;
    return VmbErrorSuccess;
}

VmbErrorType Camera::GetLocalDevice(LocalDevicePtr& rLocalDevice)
{
    if (SP_ISNULL(m_pImpl->m_pLocalDevice))
    {
        return VmbErrorDeviceNotOpen;
    }

    rLocalDevice = m_pImpl->m_pLocalDevice;
    return VmbErrorSuccess;
}

VmbErrorType Camera::GetTransportLayer(TransportLayerPtr& rTransportLayer) const
{
    if (SP_ISNULL(m_pImpl->m_pInterface))
    {
        return VmbErrorNotAvailable;
    }

    return SP_ACCESS(m_pImpl->m_pInterface)->GetTransportLayer(rTransportLayer);
}

VmbErrorType Camera::GetPermittedAccess( VmbAccessModeType &rePermittedAccess ) const
{
    VmbError_t res;
    VmbCameraInfo_t info;

    res = VmbCameraInfoQuery( m_pImpl->m_cameraInfo.cameraIdExtended.c_str(), &info, sizeof( VmbCameraInfo_t ));

    if ( VmbErrorSuccess == res )
    {
        rePermittedAccess = static_cast<VmbAccessModeType>(info.permittedAccess);
    }

    return static_cast<VmbErrorType>(res);
}

VmbErrorType Camera::ReadMemory( const VmbUint64_t address, VmbUchar_t *pBuffer, VmbUint32_t nBufferSize, VmbUint32_t *pSizeComplete ) const noexcept
{
    return static_cast<VmbErrorType>( VmbMemoryRead( GetHandle(), address, nBufferSize, (char*)pBuffer, pSizeComplete ) );
}

VmbErrorType Camera::WriteMemory( const VmbUint64_t address, const VmbUchar_t *pBuffer, VmbUint32_t nBufferSize, VmbUint32_t *pSizeComplete ) noexcept
{
    return static_cast<VmbErrorType>( VmbMemoryWrite( GetHandle(), address, nBufferSize, (char *)pBuffer, pSizeComplete ) );
}

//Get one image synchronously.
VmbErrorType Camera::AcquireSingleImage( FramePtr &rFrame, VmbUint32_t nTimeout, FrameAllocationMode allocationMode)
{
    if (nullptr == GetHandle())
    {
        return VmbErrorDeviceNotOpen;
    }
    if (m_pImpl->m_streams.empty())
    {
        return VmbErrorNotAvailable;
    }

    VmbErrorType    res;
    VmbUint32_t     nPayloadSize;
    VmbUint32_t     nStreamBufferAlignment;
    FeaturePtr      pFeature;

    res = GetPayloadSize(nPayloadSize);

    if ( VmbErrorSuccess == res )
    {
        res = SP_ACCESS(m_pImpl->m_streams.at(0))->GetStreamBufferAlignment(nStreamBufferAlignment);
    }

    if ( VmbErrorSuccess == res )
    {
        AcquireImageHelper AcquireHelper( *this );

        auto SetFrameSynchronousFlagLambda = [](FramePtr& frame) {SP_ACCESS(frame)->m_pImpl->m_bSynchronousGrab = true; };

        res = AcquireHelper.Prepare( rFrame, nPayloadSize, allocationMode, nStreamBufferAlignment, SetFrameSynchronousFlagLambda);
        if ( VmbErrorSuccess == res )
        {
            res = (VmbErrorType)VmbCaptureFrameWait( GetHandle(), &(SP_ACCESS( rFrame )->m_pImpl->m_frame), nTimeout );
            if ( VmbErrorSuccess != res )
            {
                LOG_FREE_TEXT( "Could not acquire single image." )
            }
        }
        else
        {
            LOG_FREE_TEXT( "Preparing image acquisition failed." );
        }
        VmbErrorType local_result = AcquireHelper.TearDown();
        if( VmbErrorSuccess != local_result )
        {
            LOG_ERROR( "Tear down capture logic failed.", local_result )
            if( VmbErrorSuccess == res)
            {
                res = local_result;
            }
        }
    }
    else
    {
        LOG_ERROR( "Could not get payload size", res );
    }

    return res;
}

VmbErrorType Camera::AcquireMultipleImages( FramePtr *pFrames, VmbUint32_t nSize, VmbUint32_t nTimeout, VmbUint32_t *pNumFramesCompleted, FrameAllocationMode allocationMode)
{
    VmbErrorType res = VmbErrorBadParameter;

    if (nullptr == pFrames
         || 0 == nSize )
    {
        return res;
    }
    
    if (nullptr == GetHandle())
    {
        return VmbErrorDeviceNotOpen;
    }
    if (m_pImpl->m_streams.empty())
    {
        return VmbErrorNotAvailable;
    }

    if (nullptr != pNumFramesCompleted )
    {
        *pNumFramesCompleted = 0;
    }

    VmbUint32_t nPayloadSize;
    VmbUint32_t nStreamBufferAlignment;
    FeaturePtr pFeature;

    res = GetPayloadSize(nPayloadSize);

    if (VmbErrorSuccess == res)
    {
        res = SP_ACCESS(m_pImpl->m_streams.at(0))->GetStreamBufferAlignment(nStreamBufferAlignment);
    }

    if ( VmbErrorSuccess == res )
    {
        AcquireImageHelper AquireHelper( *this );
        VmbUint32_t nFramesQueued = 0;

        auto SetFrameSynchronousFlagLambda = [](FramePtr& frame) {SP_ACCESS(frame)->m_pImpl->m_bSynchronousGrab = true; };
        res = AquireHelper.Prepare( pFrames, nSize, nPayloadSize, nFramesQueued, allocationMode, nStreamBufferAlignment, SetFrameSynchronousFlagLambda);

        if ( VmbErrorSuccess == res )
        {
            for ( VmbUint32_t nFrameCount = 0; nFrameCount <nFramesQueued; ++ nFrameCount )
            {
                res = (VmbErrorType)VmbCaptureFrameWait( GetHandle(), &(SP_ACCESS( pFrames[nFrameCount] )->m_pImpl->m_frame), nTimeout );
                if ( VmbErrorSuccess != res )
                {
                    std::stringstream strMsg;
                    strMsg << "Could only successfully fill " << 
                        (nFrameCount > 0 ? nFrameCount - 1 : 0)  << " of " << nSize << " frames. Will stop acquisition now.";
                    LOG_FREE_TEXT( strMsg.str() );
                    break;
                }
                else if (nullptr !=  pNumFramesCompleted )
                {
                    ++(*pNumFramesCompleted);
                }
            }
            VmbErrorType local_res = AquireHelper.TearDown();
            if( VmbErrorSuccess == res)
            {
                res = local_res;
            }
        }
        else
        {
            LOG_ERROR( "Could not start capture", res )
        }
    }
    else
    {
        LOG_ERROR( "Could not get feature PayloadSize", res);
    }

    return res;
}

VmbErrorType Camera::StartContinuousImageAcquisition( int nBufferCount, const IFrameObserverPtr &rObserver, FrameAllocationMode allocationMode)
{
    if (nullptr == GetHandle())
    {
        return VmbErrorDeviceNotOpen;
    }
    if (m_pImpl->m_streams.empty())
    {
        return VmbErrorNotAvailable;
    }

    VmbErrorType        res;
    FramePtrVector      Frames;
    VmbUint32_t         nPayloadSize;
    VmbUint32_t         nStreamBufferAlignment;

    res = GetPayloadSize(nPayloadSize);

    if (VmbErrorSuccess == res)
    {
        res = SP_ACCESS(m_pImpl->m_streams.at(0))->GetStreamBufferAlignment(nStreamBufferAlignment);
    }

    if ( VmbErrorSuccess == res )
    {
        res = AcquireImageHelper::AnnounceFrames( *this, Frames, nBufferCount, nPayloadSize, rObserver, allocationMode, nStreamBufferAlignment);
        if( Frames.empty() )
        {
            return res;
        }
        res = StartCapture();
        if ( VmbErrorSuccess == res )
        {
            VmbUint32_t FramesQueued = 0;
            for (   size_t FrameNumber = 0; FrameNumber < Frames.size(); ++ FrameNumber )
            {
                VmbErrorType LocalResult =  QueueFrame( Frames[ FrameNumber] );
                if ( VmbErrorSuccess == LocalResult)
                {
                    ++FramesQueued;
                }
                else
                {
                    LOG_ERROR( "Could not queue frame", LocalResult )
                }
                if( VmbErrorSuccess == res)
                {
                    res = LocalResult;
                }
            }
            if( 0 != FramesQueued)
            {
                res = RunFeatureCommand(*this, "AcquisitionStart" );
                if ( VmbErrorSuccess != res )
                {
                    EndCapture();
                    FlushQueue();
                    RevokeAllFrames();
                    LOG_ERROR( "Could not start acquisition", res )
                    return res;
                }

            }
            else
            {
                EndCapture();
                RevokeAllFrames();
                LOG_FREE_TEXT( "Could not queue frames" )
                return res;
            }

        }
        else
        {
            RevokeAllFrames();
            LOG_ERROR( "Could not start capturing", res )
        }
    }
    else
    {
        LOG_ERROR( "Could not get feature PayloadSize", res )
    }

    return res;
}

VmbErrorType Camera::StopContinuousImageAcquisition()
{
    if (nullptr == GetHandle())
    {
        return VmbErrorDeviceNotOpen;
    }

    VmbErrorType    res;
    FeaturePtr      pFeature;

    // Prevent queuing of new frames while stopping
    {
        MutexGuard guard( m_pImpl->m_pQueueFrameMutex );
        m_pImpl->m_bAllowQueueFrame = false;
    }

    res = RunFeatureCommand( *this, "AcquisitionStop" );
    if ( VmbErrorSuccess != res )
    {
        LOG_ERROR( "Could not run feature AcquisitionStop", res )
    }

    res = EndCapture();
    if ( VmbErrorSuccess == res )
    {
        res = FlushQueue();
        if( VmbErrorSuccess != res)
        {
            LOG_ERROR( "Could not flush queue", res )
        }
        res = RevokeAllFrames();
        if ( VmbErrorSuccess != res )
        {
            LOG_FREE_TEXT("Could not revoke frames")
        }
    }
    else
    {
        LOG_ERROR("Could not stop capture, unable to revoke frames", res)
    }

    {
        MutexGuard guard(m_pImpl->m_pQueueFrameMutex);
        m_pImpl->m_bAllowQueueFrame = true;
    }

    return res;
}

VmbErrorType Camera::GetPayloadSize(VmbUint32_t& nPayloadSize) noexcept
{
    return static_cast<VmbErrorType>(VmbPayloadSizeGet(GetHandle(), &nPayloadSize));
}

VmbErrorType Camera::AnnounceFrame(const FramePtr& frame)
{
    if (nullptr == GetHandle())
    {
        return VmbErrorDeviceNotOpen;
    }
    if (m_pImpl->m_streams.empty())
    {
        return VmbErrorNotAvailable;
    }
    return SP_ACCESS(m_pImpl->m_streams.at(0))->AnnounceFrame(frame);
}

VmbErrorType Camera::RevokeFrame(const FramePtr& frame)
{
    if (nullptr == GetHandle())
    {
        return VmbErrorDeviceNotOpen;
    }
    if (m_pImpl->m_streams.empty())
    {
        return VmbErrorNotAvailable;
    }
    return SP_ACCESS(m_pImpl->m_streams.at(0))->RevokeFrame(frame);
}

VmbErrorType Camera::RevokeAllFrames()
{
    if (nullptr == GetHandle())
    {
        return VmbErrorDeviceNotOpen;
    }
    if (m_pImpl->m_streams.empty())
    {
        return VmbErrorNotAvailable;
    }
    return SP_ACCESS(m_pImpl->m_streams.at(0))->RevokeAllFrames();
}

VmbErrorType Camera::QueueFrame(const FramePtr& frame)
{
    if (nullptr == GetHandle())
    {
        return VmbErrorDeviceNotOpen;
    }
    if (m_pImpl->m_streams.empty())
    {
        return VmbErrorNotAvailable;
    }

    MutexGuard guard(m_pImpl->m_pQueueFrameMutex);
    if (false == m_pImpl->m_bAllowQueueFrame)
    {
        LOG_FREE_TEXT("Queuing of new frames is not possible while flushing and revoking the currently queued frames.");
        return VmbErrorInvalidCall;
    }

    return SP_ACCESS(m_pImpl->m_streams.at(0))->QueueFrame(frame);
}

VmbErrorType Camera::FlushQueue()
{
    if (nullptr == GetHandle())
    {
        return VmbErrorDeviceNotOpen;
    }
    if (m_pImpl->m_streams.empty())
    {
        return VmbErrorNotAvailable;
    }
    return SP_ACCESS(m_pImpl->m_streams.at(0))->FlushQueue();
}

VmbErrorType Camera::StartCapture()
{
    if (nullptr == GetHandle())
    {
        return VmbErrorDeviceNotOpen;
    }
    if (m_pImpl->m_streams.empty())
    {
        return VmbErrorNotAvailable;
    }
    return SP_ACCESS(m_pImpl->m_streams.at(0))->StartCapture();
}

VmbErrorType Camera::EndCapture()
{
    if (nullptr == GetHandle())
    {
        return VmbErrorDeviceNotOpen;
    }
    if (m_pImpl->m_streams.empty())
    {
        return VmbErrorNotAvailable;
    }
    return SP_ACCESS(m_pImpl->m_streams.at(0))->EndCapture();
}

VmbErrorType Camera::GetStreams(StreamPtr* pStreams, VmbUint32_t& rnSize) noexcept
{
    if (nullptr == GetHandle())
    {
        return VmbErrorDeviceNotOpen;
    }
    VmbErrorType res = VmbErrorInternalFault;
    if (nullptr == pStreams)
    {
        rnSize = (VmbUint32_t)m_pImpl->m_streams.size();
        res = VmbErrorSuccess;
    }
    else if (m_pImpl->m_streams.empty())
    {
        rnSize = 0;
        pStreams = nullptr;
        res = VmbErrorSuccess;
    }
    else if (m_pImpl->m_streams.size() <= rnSize)
    {
        VmbUint32_t i = 0;
        try
        {
            for (auto iter = m_pImpl->m_streams.begin();
                 m_pImpl->m_streams.end() != iter;
                 ++iter, ++i)
            {
                pStreams[i] = *iter;
            }
            rnSize = (VmbUint32_t)m_pImpl->m_streams.size();
            res = VmbErrorSuccess;
        }
        catch (...)
        {
            res = VmbErrorInternalFault; // failure in copy assignment operator
        }
    }
    else
    {
        res = VmbErrorMoreData;
    }
    return res;
}

IMEXPORT bool Camera::ExtendedIdEquals(char const* extendedId) noexcept
{
    return extendedId != nullptr
        && m_pImpl->m_cameraInfo.cameraIdExtended == extendedId;
}

VmbErrorType Camera::GetStreamBufferAlignment(VmbUint32_t& nBufferAlignment)
{
    if (nullptr == GetHandle())
    {
        return VmbErrorDeviceNotOpen;
    }
    if (m_pImpl->m_streams.empty())
    {
        return VmbErrorNotAvailable;
    }
    return SP_ACCESS(m_pImpl->m_streams.at(0))->GetStreamBufferAlignment(nBufferAlignment);
}


}  // namespace VmbCPP
