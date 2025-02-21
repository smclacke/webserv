curl -X POST http://localhost:2345 \
  -F "file1=@testfiles/text1.txt" \
  -F "file2=@testfiles/text2.txt"


curl -X DELETE http://localhost:2345/uploads/text2.txt

curl -X POST http://localhost:2345 \
     --header "Transfer-Encoding: chunked" \
     --data-binary @testfiles/text1.txt
