# AI(W906-HTDESIGNER) 20260929: build dist\<name>-<version>.vsix without npm / vsce.
# A .vsix is a zip: [Content_Types].xml + extension.vsixmanifest + extension\<files>.
# Only the files the extension needs at run time go in (no test\, no dist\).
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$pkg = Get-Content (Join-Path $here 'package.json') -Raw -Encoding utf8 | ConvertFrom-Json
$dist = Join-Path $here 'dist'
New-Item -ItemType Directory -Force $dist | Out-Null
$out = Join-Path $dist ("{0}-{1}.vsix" -f $pkg.name, $pkg.version)
if (Test-Path $out) { Remove-Item $out -Force }

$files = @('package.json', 'extension.js', 'README.md', 'CHEATSHEET.md', 'CHANGELOG.md')
$files += Get-ChildItem (Join-Path $here 'lib') -File -Filter *.js | ForEach-Object { 'lib/' + $_.Name }
$files += Get-ChildItem (Join-Path $here 'media') -File | ForEach-Object { 'media/' + $_.Name }
# (1008: the C++ code templates, Ctrl+J -- package.json contributes snippets/cpp.json)
if (Test-Path (Join-Path $here 'snippets')) { $files += Get-ChildItem (Join-Path $here 'snippets') -File -Filter *.json | ForEach-Object { 'snippets/' + $_.Name } }

function Esc([string]$s) { return [System.Security.SecurityElement]::Escape($s) }
$manifest = @"
<?xml version="1.0" encoding="utf-8"?>
<PackageManifest Version="2.0.0" xmlns="http://schemas.microsoft.com/developer/vsx-schema/2011" xmlns:d="http://schemas.microsoft.com/developer/vsx-schema-design/2011">
  <Metadata>
    <Identity Language="en-US" Id="$(Esc $pkg.name)" Version="$(Esc $pkg.version)" Publisher="$(Esc $pkg.publisher)" />
    <DisplayName>$(Esc $pkg.displayName)</DisplayName>
    <Description xml:space="preserve">$(Esc $pkg.description)</Description>
    <Tags></Tags>
    <Categories>$(Esc ($pkg.categories -join ','))</Categories>
    <GalleryFlags>Public</GalleryFlags>
    <Properties>
      <Property Id="Microsoft.VisualStudio.Code.Engine" Value="$(Esc $pkg.engines.vscode)" />
      <Property Id="Microsoft.VisualStudio.Code.ExtensionDependencies" Value="" />
      <Property Id="Microsoft.VisualStudio.Code.ExtensionPack" Value="" />
      <Property Id="Microsoft.VisualStudio.Code.ExtensionKind" Value="workspace" />
      <Property Id="Microsoft.VisualStudio.Code.LocalizedLanguages" Value="" />
      <Property Id="Microsoft.VisualStudio.Services.GitHubFlavoredMarkdown" Value="true" />
    </Properties>
  </Metadata>
  <Installation>
    <InstallationTarget Id="Microsoft.VisualStudio.Code" />
  </Installation>
  <Dependencies />
  <Assets>
    <Asset Type="Microsoft.VisualStudio.Code.Manifest" Path="extension/package.json" Addressable="true" />
    <Asset Type="Microsoft.VisualStudio.Services.Content.Details" Path="extension/README.md" Addressable="true" />
  </Assets>
</PackageManifest>
"@
$types = '<?xml version="1.0" encoding="utf-8"?><Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">' +
  '<Default Extension=".json" ContentType="application/json" /><Default Extension=".js" ContentType="application/javascript" />' +
  '<Default Extension=".md" ContentType="text/markdown" /><Default Extension=".css" ContentType="text/css" />' +
  '<Default Extension=".svg" ContentType="image/svg+xml" /><Default Extension=".vsixmanifest" ContentType="text/xml" /></Types>'

Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem
$utf8 = New-Object System.Text.UTF8Encoding($false)
$zip = [System.IO.Compression.ZipFile]::Open($out, [System.IO.Compression.ZipArchiveMode]::Create)
try {
  function Add-Entry([string]$name, [byte[]]$bytes) {
    $e = $zip.CreateEntry($name, [System.IO.Compression.CompressionLevel]::Optimal)
    $s = $e.Open()
    try { $s.Write($bytes, 0, $bytes.Length) } finally { $s.Dispose() }
  }
  Add-Entry '[Content_Types].xml' $utf8.GetBytes($types)
  Add-Entry 'extension.vsixmanifest' $utf8.GetBytes($manifest)
  foreach ($f in $files) {
    $src = Join-Path $here ($f -replace '/', '\')
    Add-Entry ('extension/' + $f) ([IO.File]::ReadAllBytes($src))
  }
} finally {
  $zip.Dispose()
}
Write-Host ("built {0} ({1} files, {2:N0} bytes)" -f $out, ($files.Count + 2), (Get-Item $out).Length)
Write-Host "install: code --install-extension `"$out`" --force"
