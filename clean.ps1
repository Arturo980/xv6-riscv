# PowerShell script to clean XV6 build artifacts
# Use this instead of 'make clean' on Windows

Write-Host "Cleaning XV6 build artifacts..." -ForegroundColor Cyan

# Remove files matching patterns
$patterns = @(
    "*.tex", "*.dvi", "*.idx", "*.aux", "*.log", "*.ind", "*.ilg"
)

foreach ($pattern in $patterns) {
    Get-ChildItem -Path . -Filter $pattern -Recurse -ErrorAction SilentlyContinue | Remove-Item -Force
}

# Remove kernel files
Remove-Item -Path "kernel\*.o", "kernel\*.d", "kernel\*.asm", "kernel\*.sym" -ErrorAction SilentlyContinue -Force
Remove-Item -Path "kernel\kernel" -ErrorAction SilentlyContinue -Force

# Remove user files
Remove-Item -Path "user\*.o", "user\*.d", "user\*.asm", "user\*.sym" -ErrorAction SilentlyContinue -Force
Remove-Item -Path "user\usys.S" -ErrorAction SilentlyContinue -Force
Remove-Item -Path "user\_*" -ErrorAction SilentlyContinue -Force

# Remove other files
Remove-Item -Path "fs.img" -ErrorAction SilentlyContinue -Force
Remove-Item -Path "mkfs\mkfs", "mkfs\mkfs.exe" -ErrorAction SilentlyContinue -Force
Remove-Item -Path ".gdbinit" -ErrorAction SilentlyContinue -Force

Write-Host "Clean complete!" -ForegroundColor Green
Write-Host ""
Write-Host "Note: To build XV6, you need to use WSL or a Linux environment:" -ForegroundColor Yellow
Write-Host "  1. Open WSL: wsl" -ForegroundColor White
Write-Host "  2. Navigate: cd /mnt/c/Users/artun/Desktop/Proyectos/Sistema\ Operativos/xv6-riscv" -ForegroundColor White
Write-Host "  3. Build: make qemu" -ForegroundColor White
