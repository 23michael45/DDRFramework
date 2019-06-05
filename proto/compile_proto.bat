set PrePath=%CD%
rem %~p0 Will return the path only.
rem %~dp0  Will return the drive+path.

echo The current directory is %~dp0
set CurrentPath=%~dp0
cd %CurrentPath%

rem protoc BaseCmd.proto --cpp_out=./
protoc --cpp_out=dllexport_decl=DDRCommProto_API:./ BaseCmd.proto 
protoc BaseCmd.proto --java_out=./
protoc BaseCmd.proto --csharp_out=./
rem protoc BaseCmd.proto --js_out=./
protoc BaseCmd.proto --python_out=./

protoc --cpp_out=dllexport_decl=DDRCommProto_API:./ DDRModuleCmd.proto 
protoc DDRModuleCmd.proto --java_out=./
protoc DDRModuleCmd.proto --csharp_out=./
rem protoc DDRModuleCmd.proto --js_out=./
protoc DDRModuleCmd.proto --python_out=./


protoc RemoteCmd.proto --cpp_out=./
protoc RemoteCmd.proto --java_out=./
protoc RemoteCmd.proto --csharp_out=./
rem protoc BaseCmd.proto --js_out=./
protoc RemoteCmd.proto --python_out=./


protoc SimulationCmd.proto --cpp_out=./
protoc SimulationCmd.proto --java_out=./
protoc SimulationCmd.proto --csharp_out=./
rem protoc BaseCmd.proto --js_out=./
protoc SimulationCmd.proto --python_out=./


protoc RemoteRoute.proto --cpp_out=./
protoc RemoteRoute.proto --java_out=./
protoc RemoteRoute.proto --csharp_out=./
rem protoc RemoteRoute.proto --js_out=./
protoc RemoteRoute.proto --python_out=./

protoc ThirdpartyCmd.proto --cpp_out=./
protoc ThirdpartyCmd.proto --java_out=./
protoc ThirdpartyCmd.proto --csharp_out=./
rem protoc ThirdpartyCmd.proto --js_out=./
protoc ThirdpartyCmd.proto --python_out=./


cd %PrePath%