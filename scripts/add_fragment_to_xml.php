<?php

/**
 * @file
 * Simple script to add an XML fragment to each file in a directory.
 *
 * Usage:
 * php add_fragment_to_xml.php /path/to/input/directory /path/to/output/directory
 */

// You will want to modify $fragment before running the script.
$fragment = "<note>It's so awesome being a note!</note>";

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
 * Get the input files. Supports recusing down directories in case the
 * input material is a set of Islandora Newspaper Batch ingest packages, etc.
 */
$directory_iterator = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($input_dir));
foreach ($directory_iterator as $filepath => $info) {
  // Adjust this regex if you need more precision.
  if (preg_match('/\.xml$/', $filepath)) {
    add_fragment($filepath, $fragment, $output_dir);
  }
}

/**
 * Adds the fragment to the end of the XML file, and saves the resulting
 * file in the output directory.
 */
function add_fragment($XML_path, $fragment, $output_dir) {
  print "Processing $XML_path...";
  $dom = new DOMDocument;
  $dom->preserveWhiteSpace = FALSE;
  $dom->formatOutput = TRUE;
  $dom->load($XML_path);

  $frag = $dom->createDocumentFragment();
  $frag->appendXML($fragment);
  $dom->documentElement->appendChild($frag);

  $output_path = $output_dir . DIRECTORY_SEPARATOR . basename($XML_path);
  $mods_xml = $dom->saveXML();
  file_put_contents($output_path, $mods_xml);
  print "saved it to $output_path" . PHP_EOL;
}
