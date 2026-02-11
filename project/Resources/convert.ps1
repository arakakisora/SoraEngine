$exe = Join-Path $PSScriptRoot "TextureConverter.exe"

Get-ChildItem -File | Where-Object {
    $_.Extension -in ".jpg", ".png"
} | ForEach-Object {
    Start-Process -FilePath $exe -ArgumentList @($_.FullName, "-ml", "1") -Wait -NoNewWindow
}

pause
