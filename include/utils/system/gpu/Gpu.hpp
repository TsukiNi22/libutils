/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 17/10/2026 by @author Tsukini

File Name:
##  @file Gpu.hpp

File Description:
##  Every gpu handler of the HybridMemory
\**************************************************************/

#ifndef GPU_H
    #define GPU_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* interface */
    #include "IGpu.hpp"         // utils::system::IGpu, utils::system::GpuBuffer

    /* abstract */
    #include "AGpu.hpp"         // utils::system::AGpu

    /* backend */
    #include "OpenCLGpu.hpp"    // utils::system::OpenCLGpu
    #include "VulkanGpu.hpp"    // utils::system::VulkanGpu
    #include "CudaGpu.hpp"      // utils::system::CudaGpu

#endif /* GPU_H */
