#!/usr/bin/env php
<?php

// Read input from STDIN
$input_text = file_get_contents("php://stdin");

// Capitalize all letters
$capitalized_text = strtoupper($input_text);

// Send HTTP headers
header("Content-Type: text/plain");

while(true)
{

}

// Output the capitalized text
echo $capitalized_text;

?>
