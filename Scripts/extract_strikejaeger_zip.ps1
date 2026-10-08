$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.IO.Compression.FileSystem
$archives = @(Get-ChildItem -LiteralPath 'C:\Users\Huhuhu\Downloads' -Filter '*miHoYo*2027*.zip')
if ($archives.Count -ne 1) { throw 'Expected exactly one miHoYo 2027 archive' }
$dest = 'E:\mihoyou\KianaCombatDemo\SourceAssets\StrikeJaeger\Animation_RootBone'
New-Item -ItemType Directory -Force -Path $dest | Out-Null
$archive = [IO.Compression.ZipFile]::OpenRead($archives[0].FullName)
try {
 foreach ($entry in $archive.Entries) {
  if ($entry.FullName -like 'StrikeJaegerResources/Animation_RootBone/*.fbx') {
   $target = Join-Path $dest $entry.Name
   if (-not (Test-Path -LiteralPath $target)) {
    [IO.Compression.ZipFileExtensions]::ExtractToFile($entry, $target, $false)
   }
  }
 }
} finally { $archive.Dispose() }
Write-Output 'Enemy RootBone sources ready; existing files preserved.'
