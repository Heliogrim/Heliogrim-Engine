#Requires -Version 7
<#
.SYNOPSIS
Sorts and regroups #include / import lines the way CLion Nova's "Sort and regroup includes" does (see STYLE.md,
"Include layout"), for many files at once. CLion offers that action per file only.

.DESCRIPTION
A block is a maximal top-level run of `#include`, `import X;`, guarded import blocks
(`#if not defined(HG_MODULE_INTERFACE_UNIT)` .. `#endif`) and blank lines. Any other line is a barrier that ends it:
a comment such as `/**/`, a `#define`, an `#if`, code. Includes inside `#if` blocks are left alone. Each block is
rewritten as groups separated by one blank line, in this order:
  1. `"pch.h"`                                    followed by a `/**/` barrier
  2. lead headers (`Engine.Common/GLMConfig.hpp`, `Jolt/Jolt.h`) that configure the ones after them
  3. C++ vendor headers without a module (spdlog, Jolt, ...), in their written order: textual std must precede any
     `import std;`, which engine headers pull in; followed by a `/**/` barrier, so Nova keeps them first
  4. the main header of a .cpp (its first quoted include, named like the file)
  5. `<...>` includes
  6. `"..."` includes, relative paths (`./`, `../`) last
  7. imports; in headers as one `#if not defined(HG_MODULE_INTERFACE_UNIT)` block
Groups 5 and 6 sort per `/` segment, case-insensitively, a file before a directory at the same level, a shorter
segment first (`Engine.Assets/` before `Engine.Assets.Type/`). Duplicates are dropped; the EOL style, a BOM and
blank lines around a block are kept. A block of only system and third-party headers stays as written.

.EXAMPLE
pwsh tools/format-includes.ps1 -Path src                        # every source below src
pwsh tools/format-includes.ps1 -Path (git diff --name-only HEAD) # only changed files
pwsh tools/format-includes.ps1 -Path src -Check                 # list files that would change, exit 1 if any
pwsh tools/format-includes.ps1 -SelfTest
#>
param(
	[string[]] $Path,
	[switch] $Check,
	[switch] $SelfTest
)

$ErrorActionPreference = 'Stop'

$guardOpenRx = '^\s*#\s*if\s+(not\s+|!\s*)defined\s*\(\s*HG_MODULE_INTERFACE_UNIT\s*\)\s*$'
$includeRx = '^\s*#\s*include\s*([<"])([^>"]+)[>"]'
$importRx = '^\s*(export\s+)?import\s+([\w.:]+)\s*;\s*$'
$leadRx = '^(Engine\.Common/GLMConfig\.hpp|Jolt/Jolt\.h)$'
$vendorRx = '^((Jolt|assimp|spdlog|ryml|c4|spirv-tools|gli|tinyexr|crcpp|gtest|gmock)/|spirv/unified1/spirv\.hpp11$)'
$engineRx = '^(Engine|Editor|Heliogrim|Support)[^/]*/'

# Sort key of an include path; compared element-wise (see Compare-Include).
function Compare-Include([string] $a, [string] $b) {
	$ra = $a.StartsWith('./') -or $a.StartsWith('../')
	$rb = $b.StartsWith('./') -or $b.StartsWith('../')
	if ($ra -ne $rb) { return $ra ? 1 : -1 }
	$sa = $a.ToLowerInvariant().Split('/')
	$sb = $b.ToLowerInvariant().Split('/')
	$n = [Math]::Min($sa.Count, $sb.Count)
	for ($i = 0; $i -lt $n; $i++) {
		$fa = $i -eq $sa.Count - 1
		$fb = $i -eq $sb.Count - 1
		if ($sa[$i] -ceq $sb[$i]) { continue }
		# A file sorts before a directory on the same level.
		if ($fa -ne $fb) { return $fa ? -1 : 1 }
		return [string]::CompareOrdinal($sa[$i], $sb[$i])
	}
	return $sa.Count - $sb.Count
}

function Get-Sorted([object[]] $items, [scriptblock] $compare) {
	$list = [Collections.Generic.List[object]]::new()
	foreach ($i in $items) { $list.Add($i) }
	$list.Sort([Comparison[object]] $compare)
	return , $list.ToArray()
}

function Convert-Block([object[]] $entries, [bool] $isHeader, [bool] $mainAllowed, [string] $fileStem) {
	$pch = @(); $lead = @(); $vendor = @(); $main = @(); $angled = @(); $quoted = @(); $imports = @()
	$seen = @{}
	foreach ($e in $entries) {
		if ($e.Kind -eq 'import') {
			if (-not $seen.ContainsKey("import:$($e.Name)")) { $seen["import:$($e.Name)"] = 1; $imports += $e }
			continue
		}
		$k = "$($e.Delim)$($e.Path)"
		if ($seen.ContainsKey($k)) { continue }
		$seen[$k] = 1
		# `Foo.hpp` is the main header of `Foo.cpp` and `Foo.Structured.cpp`, not of `FooBar.cpp`.
		$stem = [IO.Path]::GetFileNameWithoutExtension($e.Path)
		$isMain = $mainAllowed -and -not $main.Count -and $e.Delim -eq '"' -and -not $e.Path.Contains('/') -and ($fileStem -eq $stem -or $fileStem.StartsWith("$stem."))
		if ($e.Delim -eq '"' -and $e.Path -eq 'pch.h') { $pch += $e }
		elseif ($e.Path -match $leadRx) { $lead += $e }
		elseif ($e.Path -match $vendorRx) { $vendor += $e }
		elseif ($isMain) { $main += $e }
		elseif ($e.Delim -eq '<') { $angled += $e }
		else { $quoted += $e }
	}
	$byPath = { param($x, $y) Compare-Include $x.Path $y.Path }
	$byName = { param($x, $y) [string]::CompareOrdinal($x.Name, $y.Name) }
	$rest = $main.Count + $angled.Count + $quoted.Count + $imports.Count
	$groups = @()
	if ($pch.Count) { $groups += , @($pch | ForEach-Object Line) }
	if ($pch.Count -and ($lead.Count + $vendor.Count + $rest)) { $groups += , @('/**/') }
	if ($lead.Count) { $groups += , @($lead | ForEach-Object Line) }
	# Vendor headers keep their order: one may need another first (tinyexr after zlib).
	if ($vendor.Count) { $groups += , @($vendor | ForEach-Object Line) }
	if (($lead.Count + $vendor.Count) -and $rest) { $groups += , @('/**/') }
	if ($main.Count) { $groups += , @($main | ForEach-Object Line) }
	if ($angled.Count) { $groups += , @((Get-Sorted $angled $byPath) | ForEach-Object Line) }
	if ($quoted.Count) { $groups += , @((Get-Sorted $quoted $byPath) | ForEach-Object Line) }
	if ($imports.Count) {
		$names = @((Get-Sorted $imports $byName) | ForEach-Object { "$($_.Export)import $($_.Name);" })
		$groups += , ($isHeader ? (@('#if not defined(HG_MODULE_INTERFACE_UNIT)') + $names + @('#endif')) : $names)
	}
	$out = @()
	for ($i = 0; $i -lt $groups.Count; $i++) {
		if ($i -gt 0) { $out += '' }
		$out += $groups[$i]
	}
	# A main header found means the rest is a regular block; a block of only pch/lead/vendor leaves main open.
	return @{ Lines = $out; HasRegular = [bool] $rest }
}

function Convert-Text([string] $text, [string] $path) {
	$eol = $text.Contains("`r`n") ? "`r`n" : "`n"
	$endsWithEol = $text.EndsWith("`n")
	$lines = $text -split '\r?\n'
	if ($endsWithEol) { $lines = $lines[0..($lines.Count - 2)] }
	$isHeader = $path -match '\.(h|hpp|hh|hxx|inl)$'
	$fileStem = [IO.Path]::GetFileNameWithoutExtension($path)
	$out = [Collections.Generic.List[string]]::new()
	$mainOpen = -not $isHeader
	# A classic include guard (`#ifndef X` + `#define X` as the first directives) counts as top level.
	$baseDepth = 0
	$directives = @($lines | Where-Object { $_ -match '^\s*#' } | Select-Object -First 2)
	if ($directives.Count -eq 2 -and $directives[0] -match '^\s*#\s*ifndef\s+(\w+)' -and $directives[1] -match "^\s*#\s*define\s+$($Matches[1])\b") { $baseDepth = 1 }
	$depth = 0
	$i = 0
	while ($i -lt $lines.Count) {
		if ($depth -ne $baseDepth) {
			if ($lines[$i] -match '^\s*#\s*if') { $depth++ } elseif ($lines[$i] -match '^\s*#\s*endif') { $depth-- }
			$out.Add($lines[$i]); $i++; continue
		}
		$j = $i; $entries = @(); $lastContent = -1
		while ($j -lt $lines.Count) {
			$l = $lines[$j]
			if ($l -match '^\s*$') { $j++; continue }
			if ($l -match $includeRx) {
				$entries += [pscustomobject]@{ Kind = 'include'; Delim = $Matches[1]; Path = $Matches[2]; Line = $l.Trim() }
				$lastContent = $j; $j++; continue
			}
			if ($l -match $importRx) {
				$entries += [pscustomobject]@{ Kind = 'import'; Export = $Matches[1] ? 'export ' : ''; Name = $Matches[2] }
				$lastContent = $j; $j++; continue
			}
			if ($l -match $guardOpenRx) {
				$k = $j + 1; $names = @()
				while ($k -lt $lines.Count -and $lines[$k] -match $importRx) {
					$names += [pscustomobject]@{ Kind = 'import'; Export = $Matches[1] ? 'export ' : ''; Name = $Matches[2] }; $k++
				}
				if ($names.Count -and $k -lt $lines.Count -and $lines[$k] -match '^\s*#\s*endif') {
					$entries += $names; $lastContent = $k; $j = $k + 1; continue
				}
			}
			break
		}
		if ($lastContent -lt 0) {
			if ($lines[$i] -match '^\s*#\s*if') { $depth++ } elseif ($lines[$i] -match '^\s*#\s*endif') { $depth-- }
			$out.Add($lines[$i]); $i++; continue
		}
		# A block of only system and third-party headers stays as written: their order can be load-bearing.
		if (-not ($entries | Where-Object { $_.Kind -eq 'import' -or $_.Delim -eq '"' -or $_.Path -match $engineRx })) {
			for ($k = $i; $k -le $lastContent; $k++) { $out.Add($lines[$k]) }
			$i = $lastContent + 1; continue
		}
		$lead = 0; while ($lines[$i + $lead] -match '^\s*$') { $lead++ }
		for ($b = 0; $b -lt $lead; $b++) { $out.Add('') }
		# A block glued to a preceding directive (`#endif`) gets one separating blank line.
		if ($lead -eq 0 -and $out.Count -gt 0 -and $out[$out.Count - 1] -match '^\s*#\s*endif') { $out.Add('') }
		$r = Convert-Block $entries $isHeader $mainOpen $fileStem
		foreach ($c in $r.Lines) { $out.Add($c) }
		if ($r.HasRegular) { $mainOpen = $false }
		$i = $lastContent + 1
	}
	$result = $out -join $eol
	if ($endsWithEol) { $result += $eol }
	return $result
}

if ($SelfTest) {
	$fails = 0
	function Assert-Eq($name, $actual, $expected) {
		if ($actual -cne $expected) { Write-Host "FAIL $name`n--- expected`n$expected`n--- actual`n$actual"; $script:fails++ }
		else { Write-Host "ok   $name" }
	}
	function Join-Lines([string[]] $l) { ($l -join "`n") + "`n" }
	$cases = [ordered]@{
		'header: groups, guarded imports' = @('X/SceneBase.hpp',
			@('#pragma once', '#include "B.hpp"', '#include <Engine.Reflect/Inherit/InheritBase.hpp>', '#include "../A.hpp"', '#include <Engine.Common/__macro.hpp>',
			  '#if not defined(HG_MODULE_INTERFACE_UNIT)', 'import std;', '#endif', '#if not defined(HG_MODULE_INTERFACE_UNIT)', 'import Engine.Common;', '#endif', '', 'namespace hg {}'),
			@('#pragma once', '#include <Engine.Common/__macro.hpp>', '#include <Engine.Reflect/Inherit/InheritBase.hpp>', '', '#include "B.hpp"', '#include "../A.hpp"', '',
			  '#if not defined(HG_MODULE_INTERFACE_UNIT)', 'import Engine.Common;', 'import std;', '#endif', '', 'namespace hg {}'))
		'segment sort' = @('X/A.hpp',
			@('#pragma once', '#include <Engine.Assets.Type/Asset.hpp>', '#include <Engine.Assets/Assets.hpp>', '#include <Engine.Assets/AssetTypeId.hpp>',
			  '#include <Editor.Action/Action/Import/AutoImportAction.hpp>', '#include <Editor.Action/ActionManager.hpp>', '#include <Engine.Common/__macro.hpp>',
			  '#include <Engine.Common/Sal.hpp>', '#include <cstdlib>', '#include <Engine.Assets.Type/AssetConcept.hpp>'),
			@('#pragma once', '#include <cstdlib>', '#include <Editor.Action/ActionManager.hpp>', '#include <Editor.Action/Action/Import/AutoImportAction.hpp>',
			  '#include <Engine.Assets/Assets.hpp>', '#include <Engine.Assets/AssetTypeId.hpp>', '#include <Engine.Assets.Type/Asset.hpp>',
			  '#include <Engine.Assets.Type/AssetConcept.hpp>', '#include <Engine.Common/__macro.hpp>', '#include <Engine.Common/Sal.hpp>'))
		'cpp: main first, vendor barrier' = @('X/Engine.cpp',
			@('#include <Engine.Logging/Logger.hpp>', '#include "Engine.hpp"', '#include <spdlog/spdlog.h>', '#include "EngineState.hpp"', 'import std;', '', 'using namespace hg;'),
			@('#include <spdlog/spdlog.h>', '', '/**/', '', '#include "Engine.hpp"', '', '#include <Engine.Logging/Logger.hpp>', '', '#include "EngineState.hpp"', '', 'import std;', '', 'using namespace hg;'))
		'pch then barrier' = @('X/Test.cpp',
			@('#include "pch.h"', '#include <Engine.B/B.hpp>', '#include <Engine.A/A.hpp>', 'import Engine.Common;'),
			@('#include "pch.h"', '', '/**/', '', '#include <Engine.A/A.hpp>', '#include <Engine.B/B.hpp>', '', 'import Engine.Common;'))
		'barriers keep blocks apart' = @('X/K.cpp',
			@('/* gli */', '#include <Engine.Common/GLMConfig.hpp>', '#include <gli/texture.hpp>', '#include <Engine.X/Y.hpp>', '', '/**/', '#include <Engine.B/B.hpp>',
			  '#define FOO', '#include <Engine.A/A.hpp>'),
			@('/* gli */', '#include <Engine.Common/GLMConfig.hpp>', '', '#include <gli/texture.hpp>', '', '/**/', '', '#include <Engine.X/Y.hpp>', '', '/**/',
			  '#include <Engine.B/B.hpp>', '#define FOO', '#include <Engine.A/A.hpp>'))
		'conditional untouched, dedup' = @('X/Foo.Structured.cpp',
			@('#include "Foo.hpp"', '#include "Foo.hpp"', '#ifdef _WIN32', '#include <b.h>', '#include <a.h>', '#endif', 'import std;'),
			@('#include "Foo.hpp"', '#ifdef _WIN32', '#include <b.h>', '#include <a.h>', '#endif', '', 'import std;'))
		'third-party block untouched' = @('X/__tinyexr.cpp',
			@('#define TINYEXR_IMPLEMENTATION', '#include <zlib/zlib.h>', '#include <tinyexr/tinyexr.h>'),
			@('#define TINYEXR_IMPLEMENTATION', '#include <zlib/zlib.h>', '#include <tinyexr/tinyexr.h>'))
		'include guard is top level' = @('X/Foo.hpp',
			@('#ifndef FOO_HPP', '#define FOO_HPP', '#include "b.hpp"', '#include <Engine.A/A.hpp>', '#endif'),
			@('#ifndef FOO_HPP', '#define FOO_HPP', '#include <Engine.A/A.hpp>', '', '#include "b.hpp"', '#endif'))
	}
	foreach ($k in $cases.Keys) {
		$c = $cases[$k]
		$out = Convert-Text (Join-Lines $c[1]) $c[0]
		Assert-Eq $k $out (Join-Lines $c[2])
		Assert-Eq "$k (idempotent)" (Convert-Text $out $c[0]) $out
	}
	$crlf = "#include `"pch.h`"`r`n#include <Engine.B/B.hpp>`r`n#include <Engine.A/A.hpp>`r`n"
	Assert-Eq 'crlf kept' (Convert-Text $crlf 'X/T.cpp') "#include `"pch.h`"`r`n`r`n/**/`r`n`r`n#include <Engine.A/A.hpp>`r`n#include <Engine.B/B.hpp>`r`n"
	exit ($fails ? 1 : 0)
}

if (-not $Path) { throw 'Need -Path (files or directories) or -SelfTest.' }
$files = foreach ($p in $Path) {
	if (Test-Path -LiteralPath $p -PathType Container) {
		Get-ChildItem -LiteralPath $p -Recurse -File -Include *.hpp, *.h, *.cpp, *.inl | ForEach-Object FullName
	} elseif ($p -match '\.(hpp|h|cpp|inl)$' -and (Test-Path -LiteralPath $p)) { (Resolve-Path -LiteralPath $p).Path }
}
$changed = 0
foreach ($file in $files) {
	$bytes = [IO.File]::ReadAllBytes($file)
	$hasBom = $bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF
	$skip = $hasBom ? 3 : 0
	$text = [Text.UTF8Encoding]::new($false, $true).GetString($bytes, $skip, $bytes.Length - $skip)
	$new = Convert-Text $text $file
	if ($new -cne $text) {
		$changed++
		if ($Check) { Write-Host "would change $file" }
		else { [IO.File]::WriteAllText($file, $new, [Text.UTF8Encoding]::new($hasBom)) }
	}
}
Write-Host "changed=$changed of $(@($files).Count)"
if ($Check -and $changed) { exit 1 }
