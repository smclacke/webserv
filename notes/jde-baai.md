curl -X POST http://localhost:2345 \
  -F "file1=@testfiles/text1.txt" \
  -F "file2=@testfiles/text2.txt"


curl -X DELETE http://localhost:2345/uploads/text2.txt

curl -X POST http://localhost:2345 \
     --header "Transfer-Encoding: chunked" \
     --data-binary @testfiles/text1.txt


----


Problem list:

-formdata. Check why the chunked header curl request formdata just returns OK.
If its not cgi shouldn’t it return not allowed or something?
-test CGI with input
-create a page with a cgi form
-create a second www directory where only GET is required
-cleanup config files.