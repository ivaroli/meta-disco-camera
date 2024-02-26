/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------
 
  File:        DefaultCameraFactory.h

  Description: Definition of class VmbCPP::DefaultCameraFactory used to
               create new Camera objects if no user defined camera factory
               class was provided.
               Intended for use in the implementation of VmbCPP.

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

#ifndef VMBCPP_DEFAULTCAMERAFACTORY_H
#define VMBCPP_DEFAULTCAMERAFACTORY_H

/**
* \file      DefaultCameraFactory.h
*
* \brief     Definition of class VmbCPP::DefaultCameraFactory used to
*            create new Camera objects if no user defined camera factory
*            class was provided.
*            Intended for use in the implementation of VmbCPP.
*/

#include <VmbCPP/ICameraFactory.h>


namespace VmbCPP {

class DefaultCameraFactory : public ICameraFactory
{
  public:
    virtual CameraPtr CreateCamera( const VmbCameraInfo_t& camInfo,
                                    const InterfacePtr& pInterface) override;
};

}  // namespace VmbCPP

#endif
