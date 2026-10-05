$ErrorActionPreference = 'Stop'

$root = $PSScriptRoot
$source = Join-Path $root 'dev.cpp'
$bin = Join-Path $root 'bin'
$output = Join-Path $bin 'dev.exe'
$compiler = Get-Command g++ -ErrorAction Stop

if (-not (Test-Path -LiteralPath $source -PathType Leaf)) {
    throw "Source file not found: $source"
}

New-Item -ItemType Directory -Path $bin -Force | Out-Null

Write-Host "Building dev.cpp -> $output"
& $compiler.Source -std=c++20 -Wall -Wextra -I $root $source -o $output
if ($LASTEXITCODE -ne 0) {
    throw "Compilation failed (g++ exit code $LASTEXITCODE)."
}

Write-Host "Built $output"
