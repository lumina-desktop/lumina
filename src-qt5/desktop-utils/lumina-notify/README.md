### lumina-notify

This is a simple application for launching QDialogs from shell.

Use:

lumina-notify arg1 arg2 arg3 arg4

* arg1 = Message Text
* arg2 = Button Text (Accept Role)
* arg3 = Button Text (Reject Role)
* arg4 = Window Title

For multiple word arguments encapsulate them with "s

***
Example usage in shell script
***

~~~~
#!/bin/csh
set a=`./lumina-notify "Did Jar Jar do anything wrong?" Yes No "Question"`
if ($a == 1) then
set b=`./lumina-notify "Are you sure?" Yes No "Are you Sure?"`
if ($b == 1) then
./lumina-notify "Please stop being a hater." Ok Ok "Haters gunna hate"
else
set c=`./lumina-notify "Thank you for changing your mind" OK OK "You're Awesome"`
endif
else
./lumina-notify "High Five for Darth Jar Jar" Sure OK "Respect"
endif`
~~~~
