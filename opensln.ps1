param([Switch]$build)
Set-Location $PSScriptRoot

function LoadConfig([String]$path)
{
    if (Test-Path -Path $path -PathType Leaf)
    {
        $raw = Get-Content $path
        return ConvertFrom-Json([String]::Join("", $raw)) 
    }
    return $null
}

function VSGetUserVersionOrLatest([String]$vsWherePath, [Int]$userVer)
{
    $raw = (& $vsWherePath -format Json -prerelease)
    $allvs = ConvertFrom-Json([String]::Join("", $raw))
    $verToUse = -1

    for ($i = 0; $i -lt $allvs.count; $i++)
    {
        $catVersion = [int]$allvs[$i].catalog.productLineVersion
        $localVer = [int]$allvs[$i].installationVersion.Split('.')[0]

        $verToUse = $i
        if($catVersion -eq $userVer)
        {
            break
        }
    }

    if ($verToUse -eq -1)
    {
        return $null
    }

    return $allvs[$verToUse]
}

function SetVSVars([String]$path)
{
    $dir = (Get-Item $path).Directory
    pushd $dir
    cmd.exe /c "vcvarsall.bat x86_x64 -vcvars_ver=14.29&set" |
    foreach {
        if ($_ -match "=")
        {
            $v = $_.Split("="); 
            Set-Item -Force -Path "ENV:\$($v[0])" -Value "$($v[1])"
        }
    }
    popd
}

$userConfigPath = "./user.json"
$vsvarsall = "\VC\Auxiliary\Build\vcvarsall.bat"
$premakePath = "./External/Premake5/"
$projectName = "Unstable"
$vsWhere = "C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe"
$vs = $null
$config = LoadConfig($userConfigPath)

$vsVersion = (VSGetUserVersionOrLatest $vsWhere $config.VSVersion)

if ($vsVersion -eq $null)
{
    Write-Error "No version of Visual Studio installed"
    return -1
}

if ($env:DevEnvDir.Length -eq 0)
{
    $vsvarsallPath = $vsVersion.installationPath + $vsvarsall
    SetVSVars($vsvarsallPath)
}

& "$($premakePath)premake5.exe" vs2019

if ($build)
{
    msbuild.exe "$($projectName).sln"
    return
}

#sln gets generated above this, but if we just want to generate then exit here
if ($generate)
{
    return
}

devenv.exe "$($projectName).sln"