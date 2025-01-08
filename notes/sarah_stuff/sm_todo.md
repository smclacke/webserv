

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



**discoveries/challenges**

1) 	
	spaces inbetween server blocks were causing different behaviours with images in browser

2) 
	when adding "return 301 /new-route; #redirects request from old route to new-route with a 301 status" to test.conf server 9999, images always has 404

3) 	
	how to translate curl post + delete requests to html for action buttons in browser

4) 	
	if rapid refresh on big image, server crashes, sometimes with "handleFile(): write to client failed", sometimes nothing - doesnt seem to be an issue with other pages...

5) 
	trying to add back the 'oldimagesdir' redirecting in new conf but being weird

6) 
	cgi - http 0.9 ???