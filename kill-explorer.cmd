taskkill /f /im:explorer.exe
start "" explorer.exe
ping 127.0.0.1 -n 1>nul 2>nul
start "" explorer.exe