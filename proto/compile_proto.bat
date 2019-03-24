set PrePath=%CD%
rem %~p0 Will return the path only.
rem %~dp0  Will return the drive+path.

echo The current directory is %~dp0
set CurrentPath=%~dp0
cd %CurrentPath%

protoc BaseCmd.proto --cpp_out=./
protoc BaseCmd.proto --java_out=./
protoc BaseCmd.proto --csharp_out=./
rem protoc BaseCmd.proto --js_out=./
protoc BaseCmd.proto --python_out=./


protoc RemoteCmd.proto --cpp_out=./
protoc RemoteCmd.proto --java_out=./
protoc RemoteCmd.proto --csharp_out=./
rem protoc BaseCmd.proto --js_out=./
protoc RemoteCmd.proto --python_out=./

cd %PrePath%