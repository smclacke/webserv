

**TODOTODOTODO**

1) 
	any @todos in files

2) 
	add descriptions/briefs

3) 	
	the big to do from jdebaai


-------------------

**testing**

BASIC

1) 	(test.conf) - is this supposed to happen? - also cat.conf giving 404 on 9999 images
	triple check redirects, 9999 was doing something weird but 4242 was ok?

2) 	
	cgi timeout checking + test while(1) in the .cgi

3) 
	sometimes when things dont work, prog exits without error message

4) 
	rapid siege and browser testing somehow triggered CGI and then crashed everything

5) 	
	memory still having issues.. related to images? if i keep clicking images, it goes up, then run siege with c10 and i get like +300

6) 	
	if rapid refresh, server crashes, sometimes with "handleFile(): write to client failed", sometimes nothing



**discoveries**

1) 	OR NOT?? wtf
	if no space between server end bracket and next server block, server does weird shit
		still kind of works but the basic testing 1 was caused by there being no space
		and it throwing 404 for images
		-- v unclear what is happening here but something changed when i changed the spaces? or im high



**dont_know_what_to_call_this**

1) 
	when adding "return 301 /new-route; #redirects request from old route to new-route wiht a 301 status" to test.conf server 9999, images always has 404
