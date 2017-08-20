<?php

/**
 * @file
 * Utilities used by tests for Islandora Datastream CRUD.
 */

/**
 * Verifies a datastream content's sha1 checksum.
 *
 * Requires that $this->params must be set and contains a single
 * key, 'sha1', and that its value be the sha1 checksum of the
 * datastream content.
 */
class ChecksumDatastreamValidator extends DatastreamValidator {

  /**
   * Asserts that the datastream has the expected sha1 checksum. 
   */
  protected function assertChecksumMatches() {
    $sha1 = trim(sha1($this->datastreamContent));
    if ($sha1 == $this->params['sha1']) {
      $this->addResult(TRUE, "SHA1 checksum for object {$this->object->id} / {$this->datastream} matches.");
    }
    else {
      $this->addResult(FALSE, "SHA1 checksum for object {$this->object->id} / {$this->datastream} does not match (expected {$this->params['sha1']}, got $sha1).");
    }
  }
}
