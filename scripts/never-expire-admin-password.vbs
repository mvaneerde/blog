' LDAP doesn't work for controlling local users
' (unless you're a domain controller, of course)
'
' have to use WinNT provider instead

Const ADS_UF_DONT_EXPIRE_PASSWD = &H10000

' hardcoding "Admin" username
Dim admin: Set admin = GetObject("WinNT://localhost/Admin,user")

WScript.Echo "Admin's userFlags are 0x" & Hex(admin.userFlags)

If Not admin.userFlags And ADS_UF_DONT_EXPIRE_PASSWD Then
    WScript.Echo "Setting local admin account to never expire password"
    admin.userFlags = (admin.userFlags Or ADS_UF_DONT_EXPIRE_PASSWD)

    ' Save
    admin.SetInfo
End If
