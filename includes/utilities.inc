<?php

/**
 * @file
 * Utilities.
 */

/**
 * Queries Islandora's Solr index to get a list of PIDs.
 *
 * The object is the one corresponding to the alias and pointer.
 *
 * @param string $query
 *   The query to send to Solr.
 *
 * @return array
 *   An array containing all of the PIDs matching $query.
 */
function islandora_datastream_crud_query_solr($query) {
  $pids = array();
  $query_processor = new IslandoraSolrQueryProcessor();
  $query_processor->solrQuery = $query;
  // Some silly number.
  $query_processor->solrLimit = '1000000';
  $query_processor->executeQuery(FALSE);
  if ($query_processor->islandoraSolrResult['response']['numFound'] > 0) {
    drush_log(dt('!num matching objects found.',
      array('!num' => $query_processor->islandoraSolrResult['response']['numFound'])), 'ok');
    foreach ($query_processor->islandoraSolrResult['response']['objects'] as $object) {
      $pids[] = $object['solr_doc']['PID'];
    }
  }
  return $pids;
}

/**
 * Retrieves the content of a datastream and saves it to a file.
 *
 * @param string $pid
 *   The PID of the current Islandora object.
 * @param string $dsid
 *   The DSID of the datastream to be retrieved.
 * @param string $dir
 *   The directory to save the datastream content in.
 *
 * @return bool
 *   TRUE on successful saving of the content, FALSE otherwise.
 */
function islandora_datastream_crud_retrieve_datastream($pid, $dsid, $dir) {
  if (!islandora_object_load($pid)) {
    drush_set_error('IBD_COLLECTION_NOT_FOUND',
      dt('The specified object (!pid) does not exist or is not accessible.',
      array('!pid' => $pid)));
    return FALSE;
  }
  else {
    $object = islandora_object_load($pid);
    $normalized_pid = preg_replace('/:/', '_', $pid);
    if ($object[$dsid]) {
      $ds = $object[$dsid];
      // If no datastream version number is specified, Tuque gives us
      // version 0.
      if (drush_get_option('datastreams_version')) {
        $version_number = drush_get_option('datastreams_version');
        // We need to make sure that the version number does not
        // exceed the number of versions.
        if ($ds->count() > $version_number) {
          $ds = $ds[$version_number];
        }
        else {
          return FALSE;
        }
      }
      if (drush_get_option('datastreams_extension')) {
        $extension = drush_get_option('datastreams_extension');
      }
      else {
        module_load_include('inc', 'islandora', 'includes/mimetype.utils');
        $extension = islandora_get_extension_for_mimetype($ds->mimetype);
      }
      $ds_filename = $normalized_pid . '_' . $dsid . '.' . $extension;
      // If a version was specified, we need to record some information about
      // the version that was retrieved in case we want to revert to that
      // version later.
      if (drush_get_option('datastreams_version')) {
        islandora_datastream_crud_write_version_data_log($dir, $ds_filename, $ds);
      }
      $path = $dir . DIRECTORY_SEPARATOR . $ds_filename;
      $ds->getContent($path);
      return TRUE;
    }
  }
}


/**
 * Ingests a datastream.
 *
 * @param string $ds_path
 *   The absolute path to the datastream content.
 *
 * @return string|bool
 *   The PID of the object the datastream was added to, FALSE otherwise.
 */
function islandora_datastream_crud_push_datastream($ds_path) {
  if (drush_get_option('datastreams_revert')) {
    $version_data_path = drush_get_option('datastreams_source_directory') .
      DIRECTORY_SEPARATOR . 'version_data.log';
    if (!file_exists($version_data_path)) {
      drush_set_error('VERSION_DATA_NOT_FOUND', dt('The version data file ' .
        '(!version_data_path) cannot be found.',
        array('!version_data_path' => $version_data_path)));
      exit;
    }
  }
  $ds_filename = pathinfo($ds_path, PATHINFO_FILENAME);
  list($namespace, $number, $dsid) = explode('_', $ds_filename);
  if (!$pid = islandora_datastream_crud_dsfilename_to_pid($ds_filename)) {
    return FALSE;
  }
  module_load_include('inc', 'islandora', 'includes/utilities');

  if (!islandora_is_valid_pid($pid)) {
    drush_set_error('PID_IS_INVALID', dt('The PID (!pid) is not valid.',
      array('!pid' => $pid)));
    return FALSE;
  }

  drupal_static_reset('islandora_get_tuque_connection');

  if (!islandora_object_load($pid)) {
    drush_set_error('OBJECT_DOES_NOT_EXIST',
      dt('The specified object (!pid) does not exist or is not accessible.',
      array('!pid' => $pid)));
    return FALSE;
  }
  else {
    $object = islandora_object_load($pid);
    if (drush_get_option('datastreams_revert')) {
      $version_data = islandora_datastream_crud_read_version_data_log($version_data_path);
    }
    $ds = isset($object[$dsid]) ? $object[$dsid] : $object->constructDatastream($dsid);
    $ds->setContentFromFile($ds_path);
    if (drush_get_option('datastreams_mimetype')) {
      $ds->mimetype = drush_get_option('datastreams_mimetype');
    }
    if (drush_get_option('datastreams_label')) {
      $ds->label = drush_get_option('datastreams_label');
    }
    // If we're reverting, use the previous version's mimetypeand label.
    if (drush_get_option('datastreams_revert')) {
      $ds->mimeType = $version_data[$ds_filename]['mimetype'];
      $ds->label = $version_data[$ds_filename]['label'];
    }
    try {
      $object->ingestDatastream($ds);
      drush_log(dt("!dsid datastream pushed to object !pid", array('!dsid' => $dsid, '!pid' => $pid)), 'ok');
      islandora_datastream_crud_write_crudlog($pid, $dsid, 'pushed');
      return $pid;
    }
    catch (Exception $e) {
      drush_log(dt("!dsid datastream could not be pushed to object !pid; details below",
        array('!dsid' => $dsid, '!pid' => $pid)), 'error');
      islandora_datastream_crud_write_crudlog($pid, $dsid, 'not pushed');
      drush_log($e->getMessage(), 'error');
      return FALSE;
    }
  }
}

/**
 * Deletes a datastream from an object.
 *
 * @param string $pid
 *   The PID of the object to delete the datastream from.
 * @param string $dsid
 *   The ID of the datastream to delete.
 *
 * @return bool
 *   TRUE if the deletion was successful, FALSE if not.
 */
function islandora_datastream_crud_delete_datastream($pid, $dsid) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  if (!islandora_is_valid_pid($pid)) {
    drush_set_error('PID_IS_INVALID', dt('The PID (!pid) is not valid.',
      array('!pid' => $pid)));
    return FALSE;
  }

  drupal_static_reset('islandora_get_tuque_connection');

  if (!islandora_object_load($pid)) {
    drush_set_error('IBD_COLLECTION_NOT_FOUND',
      dt('The specified object (!pid) does not exist or is not accessible.',
      array('!pid' => $pid)));
    return FALSE;
  }
  else {
    $object = islandora_object_load($pid);
    try {
      if ($object->purgeDatastream($dsid)) {
        drush_log(dt("!dsid datastream purged from object !pid", array('!dsid' => $dsid, '!pid' => $pid)), 'ok');
        islandora_datastream_crud_write_crudlog($pid, $dsid, 'deleted');
        return TRUE;
      }
      else {
        drush_log(dt("!dsid datastream not purged from object !pid", array('!dsid' => $dsid, '!pid' => $pid)), 'error');
        islandora_datastream_crud_write_crudlog($pid, $dsid, 'not deleted');
        return FALSE;
      }
    }
    catch (Exception $e) {
      drush_log(dt("Error purging !dsid datastream from object !pid; details below.",
        array('!dsid' => $dsid, '!pid' => $pid)), 'error');
      drush_log($e->getMessage(), 'error');
      islandora_datastream_crud_write_crudlog($pid, $dsid, 'not deleted');
      return FALSE;
    }
  }
}

/**
 * Returns a list of PIDs from a PID file.
 *
 * @param string $pid_file_path
 *   The absolute path to the PID file.
 *
 * @return array
 *   A list of PIDs.
 */
function islandora_datastream_crud_read_pid_file($pid_file_path) {
  if (!file_exists($pid_file_path)) {
    drush_set_error('PID_FILE_NOT_FOUND',
    dt('The specified PID file (!pid_file) does not exist.',
      array('!pid_file' => $pid_file_path)));
    drupal_exit();
  }

  $pids = array();
  $lines = file($pid_file_path);
  foreach ($lines as $pid) {
    $pid = trim($pid);
    // Skip commented out rows.
    if (!preg_match('!(#|//)!', $pid)) {
      $pids[] = $pid;
    }
  }
  return $pids;
}

/**
 * Converts a datastream filename to a PID.
 *
 * @param string $filepath
 *   The datastream filepath.
 *
 * @return string
 *   The corresponding PID.
 */
function islandora_datastream_crud_dsfilename_to_pid($filepath) {
  $filename = pathinfo($filepath, PATHINFO_FILENAME);
  $filenames_to_skip = array(
    'version_data',
  );
  if (in_array($filename, $filenames_to_skip)) {
    return FALSE;
  }
  list($namespace, $restofthepid, $dsid) = explode('_', $filename);
  return $namespace . ':' . $restofthepid;
}

/**
 * Writes the CRUD log.
 *
 * @param string $pid
 *   The PID of the object being operated on.
 * @param string $dsid
 *   The datastream ID.
 * @param string $message
 *   The message to write to the log.
 */
function islandora_datastream_crud_write_crudlog($pid, $dsid, $message) {
  if (drush_get_option('datastreams_crud_log')) {
    $crud_log_path = drush_get_option('datastreams_crud_log');
    $entry = $pid . ',' . $dsid . ',' . $message;
    file_put_contents($crud_log_path, $entry . PHP_EOL, FILE_APPEND);
  }
}

/**
 * Writes attributes of a datastream version to a log.
 *
 * @param string $dir
 *   The directory where the datastream content is being saved.
 * @param string $ds_filename
 *   The filename given to the datastream content.
 * @param object $ds
 *   The datastream object.
 */
function islandora_datastream_crud_write_version_data_log($dir, $ds_filename, $ds) {
  $version_log_path = $dir . DIRECTORY_SEPARATOR . 'version_data.log';
  $entry = $ds_filename . ',' . $ds->createdDate . ',' . $ds->mimetype .
    ',' . $ds->label;
  file_put_contents($version_log_path, $entry . PHP_EOL, FILE_APPEND);
}

/**
 * Reads attributes of a datastream version from the data file (log).
 *
 * @param string $path
 *   The path to the version data file.
 *
 * @return array
 *   Array containing version info.
 */
function islandora_datastream_crud_read_version_data_log($path) {
  if (file_exists($path)) {
    $version_data_rows = file($path);
    static $version_data = array();
    if (count($version_data) == 0) {
      foreach ($version_data_rows as $ds_version_data) {
        list($filename, $created_date, $mimetype, $label) = explode(',', $ds_version_data, 4);
        // For now we're only interested in the new version's
        // mimetype and label.
        $version_data[$filename]['mimetype'] = $mimetype;
        $version_data[$filename]['label'] = $label;
      }
    }
    return $version_data;
  }
  else {
    return FALSE;
  }
}
