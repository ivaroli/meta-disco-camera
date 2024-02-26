/*=============================================================================
  Copyright (C) 2012 - 2022 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        main.c

  Description: Implementation of main entry point of AsynchronousGrab example
               of VmbCPP.

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

#include "AcquisitionHelper.h"

#include <exception>
#include <iostream>

int main()
{
    std::cout << "////////////////////////////////////////\n";
    std::cout << "/// VmbCPP Asynchronous Grab Example ///\n";
    std::cout << "////////////////////////////////////////\n\n";

    try
    {
        VmbCPP::Examples::AcquisitionHelper acquisitionHelper;

        acquisitionHelper.Start();

        std::cout << "Press <enter> to stop acquisition...\n";
        getchar();
    }
    catch (std::runtime_error& e)
    {
        std::cout << e.what() << std::endl;
        return 1;
    }

    return 0;
    // AcquisitionHelper's destructor will stop the acquisition and shutdown the VmbCPP API
}
