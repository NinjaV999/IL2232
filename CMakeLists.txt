# cmake -DCMAKE_PREFIX_PATH=//home/yz/myprojects/2022/kth09/pytorchc++2023/libtorch/ ..
#cmake --build . --config Release

cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(SimplifiedNoC202309)

#libtorch begin
#find_package(Torch REQUIRED)

#libtorch end

#
#set(Torch_DIR "/home/yz/myprojects/2022/kth09/pytorchc++2023/libtorch/share/cmake/Torch")
#

# 设定一个变量方便后续使用，相当于 ${SOURCES} = "src/Hello.cpp src/main.cpp"
set(SOURCES
        main.cpp

        parameters.hpp  printfSW.hpp
        FunctionDefineSwitch.h

        AXI4/AXI4Signal.cpp

        AXI4/Converter.cpp
        AXI4/MasterPort.cpp
        AXI4/SlaveNI.cpp

        AXI4/BasicNI.cpp
        AXI4/MasterNI.cpp
        AXI4/Message.cpp
        AXI4/SlavePort.cpp

        TDM/BlockBuffer.cpp  TDM/Signal.cpp   TDM/TLink.cpp     TDM/TNI.cpp       TDM/TRouter.cpp

        TDM/Block.cpp        TDM/TInPort.cpp  TDM/TNetwork.cpp  TDM/TOutPort.cpp


        VC/FlitBuffer.cpp      VC/PacketBuffer.cpp     VC/VCNetwork.cpp
          VC/NI.cpp        VC/RInPort.cpp
        VC/Flit.cpp            VC/Packet.cpp         VC/VCRouter.cpp
              VC/NRBase.cpp        VC/ROutPort.cpp
        VC/Link.cpp        VC/Port.cpp


        ComplexTrafficControl/TrafficController.cpp  ComplexTrafficControl/TrafficGenerator.cpp
        AXI4/Converter_Master.cpp AXI4/Converter_Master.hpp AXI4/SlavePortInMasterNI.h AXI4/SlavePortInMasterNI.cpp VC/VCNI_MMP.cpp VC/VCNI_MMP.h
        interfaceCPP.cpp
        interfaceCPP.hpp

)
include_directories(copy_4)
include_directories(Default)
include_directories(RecordFiles)
include_directories(file)
include_directories(file_64)
include_directories(.)
include_directories(VC)
include_directories(AXI4)
include_directories(TDM)
include_directories(ComplexTrafficControl)
add_executable(hello_cmake  ${SOURCES})
#
#target_link_libraries(hello_cmake "${TORCH_LIBRARIES}")
#
set_property(TARGET hello_cmake PROPERTY CXX_STANDARD 14)