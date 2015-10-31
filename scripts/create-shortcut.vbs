If WScript.Arguments.Count < 2 Or WScript.Arguments.Count > 3 Then
    WScript.Echo "Expected two or three arguments; got " & WScript.Arguments.Count
    WScript.Echo "First argument is the file to create"
    WScript.Echo "Second is the command to link to"
    WScript.Echo "Third, if present, is the arguments to pass"
    WScript.Quit
End If

 Set shell = WScript.CreateObject("WScript.Shell")

 Set link = shell.CreateShortcut(WScript.Arguments(0))
link.TargetPath = WScript.Arguments(1)

 If WScript.Arguments.Count = 3 Then
    link.Arguments = WScript.Arguments(2)
End If

link.Save
