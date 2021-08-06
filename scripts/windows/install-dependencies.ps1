echo "Install Windows Dependencies!"

New-Item -Path .\temp -ItemType directory
Set-Location -Path .\temp
Invoke-WebRequest -Uri "https://nmap.org/dist/nmap-7.12-setup.exe" -OutFile .\nmap-7.12-setup.exe
choco install 7zip -y
7z e "nmap-7.12-setup.exe" "winpcap-nmap-4.13.exe"
.\winpcap-nmap-4.13.exe /S
Set-Location -Path "..\"
Remove-Item -Force -Recurse ".\temp"
