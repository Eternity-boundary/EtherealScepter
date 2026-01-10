[xml]$proj = Get-Content "EtherealScepter.vcxproj"
function Set-Include($tag,$old,$new){$proj.SelectNodes("//$tag[@Include='$old']") | ForEach-Object { $_.Include = $new }}
function Remove-Include($tag,$include){$proj.SelectNodes("//$tag[@Include='$include']") | ForEach-Object { $_.ParentNode.RemoveChild($_) | Out-Null }}

Set-Include 'ClInclude' 'AddPortRuleDialog.xaml.h' 'src\Views\Dialogs\AddPortRuleDialog.xaml.h'
Set-Include 'ClInclude' 'DashboardPage.xaml.h' 'src\Views\DashboardPage.xaml.h'
Set-Include 'ClInclude' 'NetworkPage.xaml.h' 'src\Views\NetworkPage.xaml.h'
Set-Include 'ClInclude' 'App.xaml.h' 'src\App\App.xaml.h'
Set-Include 'ClInclude' 'MainWindow.xaml.h' 'src\Views\MainWindow.xaml.h'
Set-Include 'ClInclude' 'PortPage.xaml.h' 'src\Views\PortPage.xaml.h'
Set-Include 'ClInclude' 'UPnPPage.xaml.h' 'src\Views\UPnPPage.xaml.h'
Remove-Include 'ClInclude' 'src\App\App.xaml.h'
if (-not $proj.SelectSingleNode("//ClInclude[@Include='src\App\App.xaml.h']")) {
    $ig = $proj.SelectNodes('//ItemGroup') | Where-Object { $_.ClInclude }
    if ($ig.Count -gt 0) {
        $node = $proj.CreateElement('ClInclude', $proj.Project.NamespaceURI)
        $node.SetAttribute('Include','src\App\App.xaml.h')
        $dep = $proj.CreateElement('DependentUpon',$proj.Project.NamespaceURI)
        $dep.InnerText = 'App.xaml'
        $node.AppendChild($dep) | Out-Null
        $ig[0].AppendChild($node) | Out-Null
    }
}

Set-Include 'ApplicationDefinition' 'App.xaml' 'src\App\App.xaml'
Remove-Include 'Page' 'src\App\App.xaml'
Set-Include 'Page' 'AddPortRuleDialog.xaml' 'src\Views\Dialogs\AddPortRuleDialog.xaml'
Set-Include 'Page' 'DashboardPage.xaml' 'src\Views\DashboardPage.xaml'
Set-Include 'Page' 'MainWindow.xaml' 'src\Views\MainWindow.xaml'
Set-Include 'Page' 'NetworkPage.xaml' 'src\Views\NetworkPage.xaml'
Set-Include 'Page' 'PortPage.xaml' 'src\Views\PortPage.xaml'
Set-Include 'Page' 'UPnPPage.xaml' 'src\Views\UPnPPage.xaml'

Set-Include 'ClCompile' 'AddPortRuleDialog.xaml.cpp' 'src\Views\Dialogs\AddPortRuleDialog.xaml.cpp'
Set-Include 'ClCompile' 'DashboardPage.xaml.cpp' 'src\Views\DashboardPage.xaml.cpp'
Set-Include 'ClCompile' 'NetworkPage.xaml.cpp' 'src\Views\NetworkPage.xaml.cpp'
Set-Include 'ClCompile' 'App.xaml.cpp' 'src\App\App.xaml.cpp'
Set-Include 'ClCompile' 'MainWindow.xaml.cpp' 'src\Views\MainWindow.xaml.cpp'
Set-Include 'ClCompile' 'PortPage.xaml.cpp' 'src\Views\PortPage.xaml.cpp'
Set-Include 'ClCompile' 'UPnPPage.xaml.cpp' 'src\Views\UPnPPage.xaml.cpp'

Set-Include 'Midl' 'AddPortRuleDialog.idl' 'src\Views\Dialogs\AddPortRuleDialog.idl'
Set-Include 'Midl' 'DashboardPage.idl' 'src\Views\DashboardPage.idl'
Set-Include 'Midl' 'MainWindow.idl' 'src\Views\MainWindow.idl'
Set-Include 'Midl' 'NetworkPage.idl' 'src\Views\NetworkPage.idl'
Set-Include 'Midl' 'PortPage.idl' 'src\Views\PortPage.idl'
Set-Include 'Midl' 'UPnPPage.idl' 'src\Views\UPnPPage.idl'

$proj.Save('EtherealScepter.vcxproj')
