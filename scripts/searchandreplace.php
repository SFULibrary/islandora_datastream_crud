<?php

/**
 * @file
 * Simple script to perform search and replace in text/XML files
 *
 * Usage:
 *
 * Edit $pattern and $replacement and run:
 *
 * php searchandreplace.php [inputdir] [outputdir]
 *
 * where [inputdir] is the directory full of files you want to modify
 * and [outputdir] is the directory you want to save them in.
 */

// You will want to modify $pattern and $replacement before running the script.
// $target_pattern is a PHP regular expression pattern.
$pattern = "/Gift\sfrom\sJane\-Anne\sManson\sdonation/";
$replacement = "Gift of Jane-Anne Manson";

// You may also need to adjust the list of file extensions. This is a pipe-separated
// list of, without leading periods.
$extensions = 'xml|txt';

// Do not change anything below this line.

$input_dir = trim($argv[1]);
$output_dir = trim($argv[2]);

// Check for the existence of all of the things...
if (!file_exists($input_dir)) {
  print "Sorry, can't find input directory $input_dir" . PHP_EOL;
  exit;
}
if (!file_exists($output_dir)) {
  print "Output directory $output_dir doesn't exist, creating it." . PHP_EOL;
  mkdir($output_dir);
}

/**
 * Get the input files. Supports recusing down directories.
 */
$directory_iterator = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($input_dir));
foreach ($directory_iterator as $filepath => $info) {
  if (preg_match('/\.(' . $extensions . ')$/', $filepath)) {
    searchandreplace($filepath, $pattern, $replacement, $output_dir);
  }
}

/**
 * Adds the fragment to the end of the XML file, and saves the resulting
 * file in the output directory.
 */
function searchandreplace($filepath, $pattern, $replacement, $output_dir) {
  print "Processing $filepath...";
  $file_contents = file_get_contents($filepath);
  $modified_file_contents = preg_replace($pattern, $replacement, $file_contents); 

  $output_path = $output_dir . DIRECTORY_SEPARATOR . basename($filepath);
  file_put_contents($output_path, $modified_file_contents);
  print "saving it to $output_path" . PHP_EOL;
}
