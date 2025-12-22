$buildDir = "..\..\build"

New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
Push-Location $buildDir

cl -Zi ..\game-project\code\game.cpp user32.lib Gdi32.lib

Pop-Location