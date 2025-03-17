# UniOpen

To build with MinGW, use the following commands:

```powershell
cmake -DCMAKE_C_COMPILER:FILEPATH=$( (Get-Command gcc).Source ) -DCMAKE_CXX_COMPILER:FILEPATH=$( (Get-Command g++).Source ) -GNinja -Bbuild -S.
cmake --build build --config Release --target uniopen
```

