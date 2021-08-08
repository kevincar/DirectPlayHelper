echo "Install Windows Dependencies!"

Write-Output "Creating Temp directory..."
New-Item -Path .\temp -ItemType directory
Write-Output "Creating Temp directory... Done"

Write-Output "Moving into the temp directory..."
Set-Location -Path .\temp
Write-Output "Moving into the temp directory... Done"

Write-Output "Downloading nmap..."
Invoke-WebRequest -Uri "https://nmap.org/dist/nmap-7.12-setup.exe" -OutFile .\nmap-7.12-setup.exe
Write-Output "Downloading nmap... Done"

Write-Output "Installing 7zip..."
choco install 7zip -y
Write-Output "Installing 7zip... done"

Write-Output "Extracting winpcap..."
7z e "nmap-7.12-setup.exe" "winpcap-nmap-4.13.exe"
Write-Output "Extracting winpcap... Done"

Write-Output "Installing winpcap..."
Start-Process -FilePath ".\winpcap-nmap-4.13.exe" -ArgumentList "/S" -Wait
Write-Output "Installing winpcap... Done"

Write-Output "Moving to previous directory..."
Set-Location -Path "..\"
Write-Output "Moving to previous directory... Done"

Write-Output "Removing temp directory..."
Remove-Item -Force -Recurse ".\temp"
Write-Output "Removing temp directory... Done"
