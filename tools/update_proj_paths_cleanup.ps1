[xml]$proj = Get-Content "EtherealScepter.vcxproj"
$ns = New-Object System.Xml.XmlNamespaceManager($proj.NameTable)
$ns.AddNamespace('msb', $proj.Project.NamespaceURI)

function Remove-Includes($tag, $list){
    foreach($item in $list){
        $proj.SelectNodes("//msb:$tag[@Include='$item']", $ns) | ForEach-Object { $_.ParentNode.RemoveChild($_) | Out-Null }
    }
}

$oldClIncludes = 'AddPortRuleDialog.xaml.h','DashboardPage.xaml.h','NetworkPage.xaml.h','App.xaml.h','MainWindow.xaml.h','PortPage.xaml.h','UPnPPage.xaml.h','src\\App\\App.xaml.h'
$oldClCompiles = 'AddPortRuleDialog.xaml.cpp','DashboardPage.xaml.cpp','NetworkPage.xaml.cpp','App.xaml.cpp','MainWindow.xaml.cpp','PortPage.xaml.cpp','UPnPPage.xaml.cpp'
$oldPages = 'AddPortRuleDialog.xaml','DashboardPage.xaml','MainWindow.xaml','NetworkPage.xaml','PortPage.xaml','UPnPPage.xaml','App.xaml','src\\App\\App.xaml'
$oldMidl = 'AddPortRuleDialog.idl','DashboardPage.idl','MainWindow.idl','NetworkPage.idl','PortPage.idl','UPnPPage.idl'

Remove-Includes 'ClInclude' $oldClIncludes
Remove-Includes 'ClCompile' $oldClCompiles
Remove-Includes 'Page' $oldPages
Remove-Includes 'ApplicationDefinition' 'App.xaml'
Remove-Includes 'Midl' $oldMidl

function Ensure-Include($tag,$include,$dependent){
    if (-not $proj.SelectSingleNode("//msb:$tag[@Include='$include']", $ns)) {
        $ig = $proj.SelectNodes('//msb:ItemGroup', $ns) | Where-Object { $_.SelectNodes("msb:$tag", $ns).Count -gt 0 }
        if ($ig.Count -eq 0){ $ig = $proj.SelectNodes('//msb:ItemGroup', $ns) }
        if ($ig.Count -gt 0){
            $node = $proj.CreateElement($tag,$proj.Project.NamespaceURI)
            $node.SetAttribute('Include',$include)
            if ($dependent){
                $dep = $proj.CreateElement('DependentUpon',$proj.Project.NamespaceURI)
                $dep.InnerText = $dependent
                $node.AppendChild($dep) | Out-Null
            }
            $ig[0].AppendChild($node) | Out-Null
        }
    }
}

Ensure-Include 'ApplicationDefinition' 'src\\App\\App.xaml' $null
Ensure-Include 'Page' 'src\\Views\\Dialogs\\AddPortRuleDialog.xaml' $null
Ensure-Include 'Page' 'src\\Views\\DashboardPage.xaml' $null
Ensure-Include 'Page' 'src\\Views\\MainWindow.xaml' $null
Ensure-Include 'Page' 'src\\Views\\NetworkPage.xaml' $null
Ensure-Include 'Page' 'src\\Views\\PortPage.xaml' $null
Ensure-Include 'Page' 'src\\Views\\UPnPPage.xaml' $null

Ensure-Include 'ClInclude' 'src\\App\\App.xaml.h' 'App.xaml'
Ensure-Include 'ClInclude' 'src\\Views\\Dialogs\\AddPortRuleDialog.xaml.h' 'AddPortRuleDialog.xaml'
Ensure-Include 'ClInclude' 'src\\Views\\DashboardPage.xaml.h' 'DashboardPage.xaml'
Ensure-Include 'ClInclude' 'src\\Views\\MainWindow.xaml.h' 'MainWindow.xaml'
Ensure-Include 'ClInclude' 'src\\Views\\NetworkPage.xaml.h' 'NetworkPage.xaml'
Ensure-Include 'ClInclude' 'src\\Views\\PortPage.xaml.h' 'PortPage.xaml'
Ensure-Include 'ClInclude' 'src\\Views\\UPnPPage.xaml.h' 'UPnPPage.xaml'

Ensure-Include 'ClCompile' 'src\\App\\App.xaml.cpp' 'App.xaml'
Ensure-Include 'ClCompile' 'src\\Views\\Dialogs\\AddPortRuleDialog.xaml.cpp' 'AddPortRuleDialog.xaml'
Ensure-Include 'ClCompile' 'src\\Views\\DashboardPage.xaml.cpp' 'DashboardPage.xaml'
Ensure-Include 'ClCompile' 'src\\Views\\MainWindow.xaml.cpp' 'MainWindow.xaml'
Ensure-Include 'ClCompile' 'src\\Views\\NetworkPage.xaml.cpp' 'NetworkPage.xaml'
Ensure-Include 'ClCompile' 'src\\Views\\PortPage.xaml.cpp' 'PortPage.xaml'
Ensure-Include 'ClCompile' 'src\\Views\\UPnPPage.xaml.cpp' 'UPnPPage.xaml'

Ensure-Include 'Midl' 'src\\Views\\Dialogs\\AddPortRuleDialog.idl' $null
Ensure-Include 'Midl' 'src\\Views\\DashboardPage.idl' $null
Ensure-Include 'Midl' 'src\\Views\\MainWindow.idl' $null
Ensure-Include 'Midl' 'src\\Views\\NetworkPage.idl' $null
Ensure-Include 'Midl' 'src\\Views\\PortPage.idl' $null
Ensure-Include 'Midl' 'src\\Views\\UPnPPage.idl' $null

$proj.Save('EtherealScepter.vcxproj')
