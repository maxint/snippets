url="http://doc-server/login.asp?RedirectURL=/Default.asp" '网址
name="xhh2113"  '用户名
password="xhh2113"  '密码
set ie=createobject("internetexplorer.application")
ie.navigate url
ie.Visible = true 'true 为显示窗口  false 隐藏

do while ie.busy or ie.readystate<>4
wscript.sleep 500
loop

wscript.sleep 500
for i=0 to ie.document.all.length-1	'寻找user name的id
if left(ie.document.all(i).id, 8)="userName"  Then
ie.document.getelementbyid(ie.document.all(i).id).value=name
exit for
end if
next
wscript.sleep 200
ie.document.getelementbyid("password").value=password
wscript.sleep 200

for i=0 to ie.document.all.length-1
if ie.document.all(i).tagname="INPUT" Then
if ie.document.all(i).value="Check" then ie.document.all(i).click
end if
next
