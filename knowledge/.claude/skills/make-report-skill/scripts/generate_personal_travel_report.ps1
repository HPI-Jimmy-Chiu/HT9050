param(
    [string]$StartDate,
    [string]$OutputPath,
    [switch]$UseTestEid,
    [string]$TestEid,
    [switch]$IncludeAiSummary
)

Set-StrictMode -Version 2.0
$ErrorActionPreference = 'Stop'

function Format-MarkdownCell {
    param(
        [AllowNull()]
        [object]$Value
    )

    if ($null -eq $Value) {
        return ''
    }

    $text = [string]$Value
    $text = $text -replace '\|', '\\|'
    $text = $text -replace "`r`n|`n|`r", '<br>'
    return $text.Trim()
}

function Get-OrderedPropertyValues {
    param(
        [AllowNull()]
        [object]$InputObject
    )

    $values = @()
    if ($null -eq $InputObject) {
        return $values
    }

    foreach ($property in $InputObject.PSObject.Properties) {
        $values += $property.Value
    }

    return $values
}

function Get-ValueAtIndex {
    param(
        [object[]]$Values,
        [int]$Index
    )

    if ($null -eq $Values) {
        return ''
    }

    if ($Index -lt 0 -or $Index -ge $Values.Count) {
        return ''
    }

    return $Values[$Index]
}

function Resolve-QueryStartDate {
    param(
        [string]$InputStartDate
    )

    $today = (Get-Date).Date
    $minDate = $today.AddDays(-6)

    if ([string]::IsNullOrWhiteSpace($InputStartDate)) {
        return $minDate
    }

    $parsed = [datetime]::ParseExact($InputStartDate, 'yyyyMMdd', $null)
    if ($parsed.Date -lt $minDate -or $parsed.Date -gt $today) {
        throw "startdate must be within the last 7 days. Allowed range: $($minDate.ToString('yyyyMMdd')) ~ $($today.ToString('yyyyMMdd'))"
    }

    return $parsed.Date
}

function Invoke-JsonApi {
    param(
        [string]$Uri,
        [string]$ApiKey,
        [string]$Body
    )

    $request = @{
        Method = 'Post'
        Uri = $Uri
        Headers = @{ apikey = $ApiKey }
        ContentType = 'application/json'
        Body = $Body
    }

    return Invoke-RestMethod @request
}

function Build-MarkdownReport {
    param(
        [string]$Mode,
        [string]$AdAccount,
        [string]$EmployeeName,
        [string]$EmployeeId,
        [datetime]$QueryStartDate,
        [object[]]$Rows,
        [switch]$ShowAiSummary
    )

    $builder = New-Object System.Text.StringBuilder
    $null = $builder.AppendLine('# Personal Travel Report')
    $null = $builder.AppendLine()
    $null = $builder.AppendLine('| Field | Value |')
    $null = $builder.AppendLine('| --- | --- |')
    $null = $builder.AppendLine("| Mode | $(Format-MarkdownCell $Mode) |")
    $null = $builder.AppendLine("| AD Account | $(Format-MarkdownCell $AdAccount) |")
    $null = $builder.AppendLine("| Employee Name | $(Format-MarkdownCell $EmployeeName) |")
    $null = $builder.AppendLine("| Employee ID | $(Format-MarkdownCell $EmployeeId) |")
    $null = $builder.AppendLine("| Query Start Date | $($QueryStartDate.ToString('yyyyMMdd')) |")
    $null = $builder.AppendLine("| Generated At | $((Get-Date).ToString('yyyy-MM-dd HH:mm:ss')) |")
    $null = $builder.AppendLine()

    if (-not $Rows -or $Rows.Count -eq 0) {
        $null = $builder.AppendLine('> API call succeeded, but no records were found within the last 7 days.')
        return $builder.ToString().TrimEnd()
    }

    if ($ShowAiSummary) {
        $null = $builder.AppendLine('| No | DocType | ReplyNo | Issue | Action | Owner | ReplacedParts | CustomerRequest | CreatedDate | AiSummary |')
        $null = $builder.AppendLine('| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |')
    }
    else {
        $null = $builder.AppendLine('| No | DocType | ReplyNo | Issue | Action | Owner | ReplacedParts | CustomerRequest | CreatedDate |')
        $null = $builder.AppendLine('| --- | --- | --- | --- | --- | --- | --- | --- | --- |')
    }

    foreach ($row in $Rows) {
        $values = Get-OrderedPropertyValues -InputObject $row
        $columns = @(
            (Format-MarkdownCell (Get-ValueAtIndex -Values $values -Index 0)),
            (Format-MarkdownCell (Get-ValueAtIndex -Values $values -Index 1)),
            (Format-MarkdownCell (Get-ValueAtIndex -Values $values -Index 2)),
            (Format-MarkdownCell (Get-ValueAtIndex -Values $values -Index 3)),
            (Format-MarkdownCell (Get-ValueAtIndex -Values $values -Index 4)),
            (Format-MarkdownCell (Get-ValueAtIndex -Values $values -Index 5)),
            (Format-MarkdownCell (Get-ValueAtIndex -Values $values -Index 6)),
            (Format-MarkdownCell (Get-ValueAtIndex -Values $values -Index 7)),
            (Format-MarkdownCell (Get-ValueAtIndex -Values $values -Index 8))
        )

        if ($ShowAiSummary) {
            $columns += (Format-MarkdownCell (Get-ValueAtIndex -Values $values -Index 9))
        }

        $null = $builder.AppendLine('| ' + ($columns -join ' | ') + ' |')
    }

    return $builder.ToString().TrimEnd()
}

if ($UseTestEid -and [string]::IsNullOrWhiteSpace($TestEid)) {
    throw 'Test mode requires -TestEid.'
}

if (-not $UseTestEid -and -not [string]::IsNullOrWhiteSpace($TestEid)) {
    throw 'If you want to use a specific eid, you must also pass -UseTestEid.'
}

$queryStartDate = Resolve-QueryStartDate -InputStartDate $StartDate
$adAccount = "$env:USERDOMAIN\$env:USERNAME"
$mode = 'Normal mode (current AD account)'
$employeeName = ''

if ($UseTestEid) {
    $employeeId = $TestEid
    $employeeName = 'TEST-MODE'
    $mode = 'Test mode (specified eid, not AD flow)'
}
else {
    $employeeBody = @{
        uid = $env:USERNAME
        uname = ''
        uorg = ''
        udial = ''
        uextension = ''
    } | ConvertTo-Json -Compress

    $employeeResponse = Invoke-JsonApi -Uri 'https://aiapim.honprec.com/api/hpi/hr/getEmployeeInfo' -ApiKey '48dfdba900b30d6e9446399b39ecba9d' -Body $employeeBody
    if (-not $employeeResponse.data -or @($employeeResponse.data).Count -eq 0) {
        throw "No employee data was returned for AD account: $adAccount"
    }

    $employeeRow = @($employeeResponse.data)[0]
    $employeeValues = Get-OrderedPropertyValues -InputObject $employeeRow
    $employeeName = [string](Get-ValueAtIndex -Values $employeeValues -Index 2)
    $employeeId = [string](Get-ValueAtIndex -Values $employeeValues -Index 3)

    if ([string]::IsNullOrWhiteSpace($employeeId)) {
        throw 'HR API did not return an employee ID.'
    }
}

$crmBody = @{
    startdate = $queryStartDate.ToString('yyyyMMdd')
    eid = $employeeId
} | ConvertTo-Json -Compress

$crmResponse = Invoke-JsonApi -Uri 'https://aiapim.honprec.com/api/hpi/crm/getCRMDataById' -ApiKey '48dfdba900b30d6e9446399b39ecba9d' -Body $crmBody
$rows = @()
if ($crmResponse.data) {
    $rows = @($crmResponse.data)
}

$markdown = Build-MarkdownReport -Mode $mode -AdAccount $adAccount -EmployeeName $employeeName -EmployeeId $employeeId -QueryStartDate $queryStartDate -Rows $rows -ShowAiSummary:$IncludeAiSummary

if (-not [string]::IsNullOrWhiteSpace($OutputPath)) {
    $directory = Split-Path -Parent $OutputPath
    if (-not [string]::IsNullOrWhiteSpace($directory)) {
        New-Item -ItemType Directory -Path $directory -Force | Out-Null
    }

    $utf8Bom = New-Object System.Text.UTF8Encoding($true)
    [System.IO.File]::WriteAllText($OutputPath, $markdown + [Environment]::NewLine, $utf8Bom)
}

Write-Output $markdown