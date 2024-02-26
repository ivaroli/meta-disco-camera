/*=============================================================================
  Copyright (C) 2022 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------
 
  File:        PersistableFeatureContainer.cpp

  Description: Implementation of class VmbCPP::PersistableFeatureContainer.

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

#include <VmbCPP/PersistableFeatureContainer.h>


namespace VmbCPP {

PersistableFeatureContainer::PersistableFeatureContainer() :FeatureContainer() {}

    
VmbErrorType PersistableFeatureContainer::SaveSettings( const VmbFilePathChar_t* const filePath, VmbFeaturePersistSettings_t *pSettings ) const noexcept
{
    if(nullptr == filePath)
    {
        return VmbErrorBadParameter;
    }
    
    return static_cast<VmbErrorType>(VmbSettingsSave(GetHandle(), filePath, pSettings, sizeof(*pSettings)));
}

VmbErrorType PersistableFeatureContainer::LoadSettings( const VmbFilePathChar_t* const filePath, VmbFeaturePersistSettings_t *pSettings ) const noexcept
{
    if (nullptr == filePath)
    {
        return VmbErrorBadParameter;
    }

    return static_cast<VmbErrorType>(VmbSettingsLoad(GetHandle(), filePath, pSettings, sizeof(*pSettings)));
}
    
    
}   // namespace VmbCPP

