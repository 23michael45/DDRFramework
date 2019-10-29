#!/bin/sh

PrePath=$(pwd)

echo The current directory is ${PrePath}
CurrentPath=$(dirname $0)
echo ${CurrentPath}
cd ${CurrentPath}

# sudo ./protoc BaseCmd.proto --cpp_out=./
sudo ./protoc --cpp_out=dllexport_decl=DDRCommProto_API:./ BaseCmd.proto 
sudo ./protoc BaseCmd.proto --java_out=./
sudo ./protoc BaseCmd.proto --csharp_out=./
# sudo ./protoc BaseCmd.proto --js_out=./
sudo ./protoc BaseCmd.proto --python_out=./

sudo ./protoc --cpp_out=dllexport_decl=DDRCommProto_API:./ DDRModuleCmd.proto 
sudo ./protoc DDRModuleCmd.proto --java_out=./
sudo ./protoc DDRModuleCmd.proto --csharp_out=./
# sudo ./protoc DDRModuleCmd.proto --js_out=./
sudo ./protoc DDRModuleCmd.proto --python_out=./


sudo ./protoc RemoteCmd.proto --cpp_out=./
sudo ./protoc RemoteCmd.proto --java_out=./
sudo ./protoc RemoteCmd.proto --csharp_out=./
# sudo ./protoc BaseCmd.proto --js_out=./
sudo ./protoc RemoteCmd.proto --python_out=./


sudo ./protoc SimulationCmd.proto --cpp_out=./
sudo ./protoc SimulationCmd.proto --java_out=./
sudo ./protoc SimulationCmd.proto --csharp_out=./
# sudo ./protoc BaseCmd.proto --js_out=./
sudo ./protoc SimulationCmd.proto --python_out=./


sudo ./protoc RemoteRoute.proto --cpp_out=./
sudo ./protoc RemoteRoute.proto --java_out=./
sudo ./protoc RemoteRoute.proto --csharp_out=./
# sudo ./protoc RemoteRoute.proto --js_out=./
sudo ./protoc RemoteRoute.proto --python_out=./

sudo ./protoc ThirdpartyCmd.proto --cpp_out=./
sudo ./protoc ThirdpartyCmd.proto --java_out=./
sudo ./protoc ThirdpartyCmd.proto --csharp_out=./
# sudo ./protoc ThirdpartyCmd.proto --js_out=./
sudo ./protoc ThirdpartyCmd.proto --python_out=./

sudo ./protoc DDRVLNMap.proto --cpp_out=./
sudo ./protoc DDRVLNMap.proto --java_out=./
sudo ./protoc DDRVLNMap.proto --csharp_out=./
# sudo ./protoc DDRVLNMap.proto --js_out=./
sudo ./protoc DDRVLNMap.proto --python_out=./



cd ${PrePath}
