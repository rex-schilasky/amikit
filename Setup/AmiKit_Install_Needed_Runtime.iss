[Setup]
AppName=AmiKit
AppVerName=AmiKit
DefaultDirName=AmiKit
PrivilegesRequired=admin

[Files]
; copy the VS2005 C++ redistributable setup to temp (will be deleted when setup exits)
Source: "Packages\vcredist_x86_v140.exe"; DestDir: "{tmp}"

[Run]
; check for existing VS2005 runtime; install it if needed
Filename: "{tmp}\vcredist_x86_v140.exe"; Parameters: "/q:a /c:""msiexec /i vcredist.msi /qn"""; StatusMsg: "Installing Microsoft Visual C++ 2015 Redistributable Package .."; Check: VCRuntime()

[Code]
// true if VS2015 C++ runtime dlls are not installed
function VCRuntime(): Boolean;
begin
  Result := not RegKeyExists(HKLM, 'Software\Microsoft\DevDiv\VC\Servicing\8.0\RED\1033')
end;
