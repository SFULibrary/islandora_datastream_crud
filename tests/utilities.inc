<?php

/**
 * @file
 * Utilities used by tests for Islandora Datastream CRUD.
 */

/**
 * Verifies a datastream content's sha1 checksum.
 *
 * Requires that $this->params must be set and contains a single key, 'sha1'.
 * This value is the sha1 checksum of the datastream content as a file, not
 * a string, since ->datastreamContent contains the datastream as stored on
 * disk. Therefore, to generate the checksum to use in $this->params['sha1'],
 * use the Linux sha1sum command-line utility (or equivalent) against the
 * datastream file. Counterintuitively, in the comparison below, we use PHP's
 * sha1() function, not sha1_file(), to generate the checksum of the content
 * of ->datastreamContent.
 */
class ChecksumDatastreamValidator extends DatastreamValidator {

  /**
   * Asserts that the datastream content has the expected sha1 checksum.
   */
  protected function assertChecksumMatches() {
    $sha1 = sha1($this->datastreamContent);
    if ($sha1 == $this->params['sha1']) {
      $this->addResult(TRUE, "SHA1 checksum for object {$this->object->id} / {$this->datastream} matches.");
    }
    else {
      $this->addResult(FALSE, "SHA1 checksum for object {$this->object->id} / {$this->datastream} does not match (expected {$this->params['sha1']}, got $sha1).");
    }
  }
}
