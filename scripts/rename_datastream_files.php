<?php

/**
 * Script to get the PID of an object with an identifer stored in its
 * dc.identifier field. Uses the Islandora REST interface.
 */

// No trailing slash.
$islandora_host = 'http://digital.lib.sfu.ca';

// One PID per line.
$input_ids_file = trim($argv[1]);
$input_ids = file($input_ids_file);

print "Input ID\tPID\n";
foreach ($input_ids as $id) {
  $id = trim($id);
  $id_escaped = urlencode($id);
  $request = $islandora_host . '/islandora/rest/v1/solr/dc.identifier:"' . $id_escaped . '"?fl=PID';
  $result = file_get_contents($request);
  $result = json_decode($result, TRUE);
  if ($result['response']['numFound'] == 1) {
    print "$id\t" . $result['response']['docs'][0]['PID'] . "\n";
  }
  elseif ($result['response']['numFound'] > 1) {
    print "$id\tmultiple objects\n";
  }
  else {
    print "$id\tnot found\n";
  }
}
