$ErrorActionPreference = 'Stop'

$root = $PSScriptRoot
$src = Join-Path $root 'src'
$bin = Join-Path $root 'bin'

if (-not (Test-Path -LiteralPath $src -PathType Container)) {
    throw "Source directory not found: $src"
}

$compiler = Get-Command g++ -ErrorAction Stop
$sources = @(Get-ChildItem -LiteralPath $src -File -Filter '*.cpp')

if ($sources.Count -ne 4) {
    throw "Expected exactly 4 .cpp files in '$src'; found $($sources.Count)."
}

$invalid = @($sources | Where-Object { $_.Name -cnotmatch '-script\.cpp$' })
if ($invalid.Count -gt 0) {
    throw "These source files must end exactly in '-script.cpp': $($invalid.Name -join ', ')"
}

New-Item -ItemType Directory -Path $bin -Force | Out-Null

$failed = $false
foreach ($source in $sources) {
    $exeName = $source.Name -replace '-script\.cpp$', '.exe'
    $output = Join-Path $bin $exeName

    Write-Host "Building $($source.Name) -> $exeName"
    & $compiler.Source -std=c++20 -Wall -Wextra -I $root -I $src $source.FullName -o $output

    if ($LASTEXITCODE -ne 0) {
        Write-Error "Compilation failed for $($source.Name) (g++ exit code $LASTEXITCODE)." -ErrorAction Continue
        $failed = $true
    }
}

if ($failed) {
    throw 'One or more builds failed; PATH was not changed.'
}

Write-Host "Built executables in $bin"
$answer = Read-Host "Add '$bin' to your user PATH? [y/N]"

if ($answer -match '^(y|yes)$') {
    $userPath = [Environment]::GetEnvironmentVariable('Path', 'User')
    $entries = @($userPath -split ';' | Where-Object { $_.Trim() })
    $alreadyAdded = @($entries | Where-Object {
        [string]::Equals($_.Trim().TrimEnd('\'), $bin.TrimEnd('\'), [StringComparison]::OrdinalIgnoreCase)
    }).Count -gt 0

    if (-not $alreadyAdded) {
        [Environment]::SetEnvironmentVariable('Path', (@($entries) + $bin) -join ';', 'User')
        if ($env:Path) { $env:Path += ";$bin" } else { $env:Path = $bin }
        Write-Host 'Added to user PATH. New terminals will also pick it up.'
    } else {
        Write-Host 'That folder is already on user PATH.'
    }
}