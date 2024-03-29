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
  $query_processor->solrParams['fl'] = 'PID';
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
  $filename_separator = drush_get_option('filename_separator', '_');
  if (!islandora_object_load($pid)) {
    drush_set_error('OBJECT_DOES_NOT_EXIST',
      dt('The specified object (!pid) does not exist or is not accessible.',
        array('!pid' => $pid)));
    return FALSE;
  }
  else {
    $object = islandora_object_load($pid);
    $normalized_pid = $pid;
    if ($filename_separator == '_') {
      $normalized_pid = preg_replace('/:/', '_', $pid);
    }
    else {
      $normalized_pid = $pid;
    }
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
      $ds_filename = $normalized_pid . $filename_separator . $dsid . '.' . $extension;
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
    else {
      drush_log(dt("Object !pid does not have a !dsid datastream",
        array('!dsid' => $dsid, '!pid' => $pid)), 'status');
      return FALSE;
    }
  }
}


/**
 * Ingests a datastream.
 *
 * @param string $ds_path
 *   The absolute path to the datastream content.
 * @param bool $regenerate_dc
 *   Whether or not to regenerate DC from pushed datastreams.
 *
 * @return string|bool
 *   The PID of the object the datastream was added to, FALSE otherwise.
 */
function islandora_datastream_crud_push_datastream($ds_path, $regenerate_dc = FALSE) {
  list($pid, $dsid) = islandora_datastream_crud_parse_dsfilename($ds_path);
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

  if (!$pid) {
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
    if (isset($object[$dsid])) {
      $object_had_ds = TRUE;
    }
    else {
      $object_had_ds = FALSE;
    }
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
    // If we're reverting, use the previous version's mimetype and label.
    if (drush_get_option('datastreams_revert')) {
      $ds->mimeType = $version_data[$ds_filename]['mimetype'];
      $ds->label = $version_data[$ds_filename]['label'];
    }

    try {
      // We only need to ingestDatastream() if it doesn't already exist.
      // setContentFromFile() updates the datastream content if it does exist.
      if (!$object_had_ds) {
        $object->ingestDatastream($ds);
      }
      drush_log(dt("!dsid datastream pushed to object !pid", array('!dsid' => $dsid, '!pid' => $pid)), 'ok');
      islandora_datastream_crud_write_crudlog($pid, $dsid, 'pushed');
      if ($regenerate_dc) {
        if (islandora_datastream_crud_regenerate_dc($object, $dsid)) {
          drush_log(dt("DC datastream for object !pid regenerated from !dsid", array('!dsid' => $dsid, '!pid' => $pid)), 'ok');
        }
        else {
          drush_log(dt("DC datastream for object !pid not regenerated from !dsid", array('!dsid' => $dsid, '!pid' => $pid)), 'error');
        }
      }
      return $pid;
    }
    catch (Exception $e) {
      drush_log(dt("!dsid datastream could not be pushed to object !pid; details below",
        array('!dsid' => $dsid, '!pid' => $pid)), 'error');
      islandora_datastream_crud_write_crudlog($object->id, $dsid, 'not pushed');
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
 * @param array $versions
 *   An array containing either versions or a range
 *   'versions' => array of version numbers
 *   'range' => array with starting and ending version numbers
 *
 * @return bool
 *   TRUE if the deletion was successful, FALSE if not.
 */
function islandora_datastream_crud_delete_datastream($pid, $dsid, $versions = array()) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  if (!islandora_is_valid_pid($pid)) {
    drush_set_error('PID_IS_INVALID', dt('The PID (!pid) is not valid.',
      array('!pid' => $pid)));
    return FALSE;
  }

  drupal_static_reset('islandora_get_tuque_connection');

  if (!islandora_object_load($pid)) {
    drush_set_error('COLLECTION_NOT_FOUND',
      dt('The specified object (!pid) does not exist or is not accessible.',
      array('!pid' => $pid)));
    return FALSE;
  }
  else {
    $object = islandora_object_load($pid);
    if (!isset($object[$dsid])) {
      drush_log(dt("!dsid datastream not found in object !pid", array('!dsid' => $dsid, '!pid' => $pid)), 'error');
      islandora_datastream_crud_write_crudlog($pid, $dsid, 'not found');
      return FALSE;
    }

    if($versions) {
      $datastream = $object[$dsid];
      if(!$datastream->versionable) {
        drush_log(dt("Datastream versions were specified, but !dsid datastream is not versionable in object !pid", array('!dsid' => $dsid, '!pid' => $pid)), 'error');
        islandora_datastream_crud_write_crudlog($pid, $dsid, 'datastream not versionable');
        return FALSE;
      }
      $max_version = $datastream->count() - 1;
      if (!empty($versions['range'])) {
        $versions['versions'] = [];
        $versions['range']['end'] = min($versions['range']['end'], $max_version);
        for ($i = $versions['range']['start']; $i <= $versions['range']['end']; $i++) {
          $versions['versions'][] = $i;
        }
      }
      if (!empty($versions['versions'])) {
        // Sort versions from high to low.
        arsort($versions['versions']);
        foreach ($versions['versions'] as $version) {
          if (isset($datastream[$version])) {
            try {
              unset($datastream[$version]);
              drush_log(dt("Version !version of !dsid datastream purged from object !pid", array('!version' => $version, '!dsid' => $dsid, '!pid' => $pid)), 'ok');
              islandora_datastream_crud_write_crudlog($pid, $dsid, 'version deleted');
              watchdog('islandora_datastream_crud', 'Version @version of @dsid datastream purged from object @pid', array(
                '@pid' => $pid,
                '@version' => $version,
                '@dsid' => $dsid,
              ), WATCHDOG_NOTICE);
            } catch (Exception $e) {
              drupal_set_message(t('Error deleting version %v of %s datastream from object %o %e', [
                '%v' => $version,
                '%s' => $dsid,
                '%o' => $object->label,
                '%e' => $e->getMessage(),
              ]), 'error');
              watchdog('islandora_datastream_crud', 'Error deleting version @version of @dsid datastream: @message ', array(
                '@pid' => $pid,
                '@version' => $version,
                '@dsid' => $dsid,
                '@message' => $e->getMessage(),
              ), WATCHDOG_ERROR);
            }
          }
        }
      }
      return TRUE;
    }
    else {
      try {
        if ($object->purgeDatastream($dsid)) {
          drush_log(dt("!dsid datastream purged from object !pid", array('!dsid' => $dsid, '!pid' => $pid)), 'ok');
          islandora_datastream_crud_write_crudlog($pid, $dsid, 'deleted');
          return TRUE;
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
}

/**
 * Updates one or more object properties.
 *
 * @param object $object
 *   The object to update.
 * @param array $props
 *   An associative array of the properties to update.
 */
function islandora_datastream_crud_update_object_properties($object, $props) {
  if (isset($props['state'])) {
    $pid = $object->id;
    if ($props['state'] != $object->state) {
      $object->state = $props['state'];
      drush_log(dt("State for object !pid updated to !state", array('!pid' => $object->id, '!state' => $props['state'])), 'ok');
      islandora_datastream_crud_write_crudlog($pid, $props['state'], 'Object state updated');
    }
    else {
      drush_log(dt("State for object !pid not updated since !state is both the new and old state",
        array('!pid' => $object->id, '!state' => $props['state'])), 'warning');
      islandora_datastream_crud_write_crudlog($pid, $props['state'], 'Object state not updated since old and new are the same');
    }
  }
  if (isset($props['owner'])) {
    if ($props['owner'] != $object->owner) {
      $object->owner = $props['owner'];
      drush_log(dt("Owner for object !pid updated to !owner", array('!pid' => $object->id, '!owner' => $props['owner'])), 'ok');
      islandora_datastream_crud_write_crudlog($pid, $props['owner'], 'Object owner updated');
    }
    else {
      drush_log(dt("Owner for object !pid not updated since !owner is both the new and old owner",
        array('!pid' => $object->id, '!owner' => $props['owner'])), 'warning');
      islandora_datastream_crud_write_crudlog($pid, $props['owner'], 'Object owner not updated since old and new are the same');
    }
  }

  if (drush_get_option('update_object_label')) {
    $dsid = drush_get_option('source_dsid', 'MODS');
    $xpath_exp = drush_get_option('source_xpath', '//mods:titleInfo/mods:title');
    $additional_namespace = drush_get_option('add_namespace', '');

    $ds_xml = $object[$dsid]->content;
    if ($label = islandora_datastream_crud_get_label_from_xml($xpath_exp, $ds_xml, $additional_namespace)) {
      if ($object->label != $label) {
        $object->label = $label;
        drush_log(dt("Label for object !pid updated to !label", array('!pid' => $object->id, '!label' => $label)), 'ok');
        islandora_datastream_crud_write_crudlog($pid, $label, 'Label updated');
      }
      else {
        drush_log(dt('Label for object !pid not updated since "!label" is the same',
          array('!pid' => $object->id, '!label' => $label)), 'warning');
        islandora_datastream_crud_write_crudlog($pid, $label, 'Label not updated since old and new are the same');
      }
    }
    else {
      drush_set_error('CANNOT_GET_LABEL_FROM_DATASTREAM', 'Cannot get label from datastream, object label not updated.');
    }
  }
}

/**
 * Extracts string value from a datastream using an XPath expression.
 *
 * @param string $xpath_exp
 *   The XPath expression.
 * @param string $xml
 *   A serialized XML document, e.g., coming from a MODS or DC
 *   datastream content.
 *
 * @return string|FALSE
 *   The string value of result from the XPath, or FALSE.
 */
function islandora_datastream_crud_get_label_from_xml($xpath_exp, $xml, $additional_namespace = '') {
  $concat_string = drush_get_option('concat_string', ' ');
  $dom = new DOMDocument();
  $dom->loadXML($xml);
  $xpath = new DOMXPath($dom);

  $namespaces = islandora_datastream_crud_get_namespaces($xml);

  if (strlen($additional_namespace)) {
    list($additional_prefix, $additional_uri) = explode(':', $additional_namespace, 2);
    $namespaces[trim($additional_prefix)] = trim($additional_uri);
  }

  foreach ($namespaces as $namespace_prefix => $namespace_uri) {
    $xpath->registerNamespace($namespace_prefix, $namespace_uri);
  }

  $elements = $xpath->query($xpath_exp);
  if ($elements->length) {
    $label = '';
    foreach ($elements as $element) {
      $label .= $concat_string . $element->nodeValue;
    }
    $label = ltrim($label, $concat_string);
  }
  else {
    return FALSE;
  }

  if (strlen($label)) {
    return trim($label);
  }
  else {
    return FALSE;
  }
}

/**
 * Regenerates DC from the specified datastream.
 *
 * @param object $object
 *   The Islandora object.
 * @param string $source_dsid
 *   The ID of the datastream to use as the source of the DC.
 *
 * @return bool
 *   TRUE if the DC was regenerated and replaced, FALSE otherwise.
 */
function islandora_datastream_crud_regenerate_dc($object, $source_dsid) {
  $xslt_path = drush_get_option(
    'dc_transform',
    drupal_get_path('module', 'xml_forms') . '/builder/transforms/mods_to_dc.xsl'
  );
  if (!file_exists($xslt_path)) {
    drush_log(dt('The specified XSLT stylesheet (!xslt_path) does not exist.',
      array('!xslt_path' => $xslt_path)), 'error');
    return FALSE;
  }

  if (!isset($object[$source_dsid])) {
    drush_log(dt('DC cannot be regenerated because the specified datastream (!dsid) does not exist.',
      array('!dsid' => $source_dsid)), 'error');
    return FALSE;
  }

  try {
    $xsl_doc = new DOMDocument();
    $xsl_doc->load(realpath($xslt_path));
    $xml_doc = new DOMDocument();
    $xml_doc->loadXML($object[$source_dsid]->content);
    $xslt_proc = new XSLTProcessor();
    $xslt_proc->importStylesheet($xsl_doc);
    $dc_xml = $xslt_proc->transformToXML($xml_doc);
    $object['DC']->setContentFromString($dc_xml);
  }
  catch (Exception $e) {
    drush_log(dt("DC cannot be regenerated for !pid from its !dsid datastream because of an XSLT problem; details below",
      array('!dsid' => $source_dsid, '!pid' => $object->id)), 'error');
    islandora_datastream_crud_write_crudlog($object->id, $source_dsid, 'DC not regenerated');
    drush_log($e->getMessage(), 'error');
    return FALSE;
  }

  return TRUE;
}

/**
 * Get the namespaces registered in a serialized XML document.
 *
 * @param string $xml
 *   The serialized XML document.
 *
 * @return array
 *   An associative array containing a prefix => URI pair
 *   for each registered namespace.
 */
function islandora_datastream_crud_get_namespaces($xml) {
  $simple_xml = new SimpleXMLElement($xml);
  $namespaces = $simple_xml->getNamespaces(TRUE);
  return $namespaces;
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
 * Converts a datastream filename to a PID/DSID pair.
 *
 * @param string $filepath
 *   The datastream filepath.
 *
 * @return array|bool
 *   The corresponding PID and DSID or FALSE.
 */
function islandora_datastream_crud_parse_dsfilename($filepath) {
  $filename_separator = drush_get_option('filename_separator', '_');
  $filename = pathinfo($filepath, PATHINFO_FILENAME);

  $filenames_to_skip = array(
    'version_data',
  );
  if (in_array($filename, $filenames_to_skip)) {
    return FALSE;
  }

  // The default case is namesapce_number_DSID.
  if (substr_count($filename, '_') == 2) {
    list($namespace, $number, $dsid) = explode('_', $filename);
    return array($namespace . ':' . $number, $dsid);
  }
  // Covering underscores in the DSID
  elseif (substr_count($filename, '_') == 3) {
    list($namespace, $number, $dsid) = explode('_', $filename, 3);
    return array($namespace . ':' . $number, $dsid);
  }

  else {
    list($pid, $dsid) = explode($filename_separator, $filename);
    return array($pid, $dsid);
  }
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

function islandora_datastream_crud_parse_versions_option($versions) {
  $result = [];
  $range_delimited = strpos($versions, '..');
  if($range_delimited === FALSE) {
    $versions = drupal_explode_tags($versions);
    foreach($versions as $version) {
      if(is_numeric($version)) {
        $result['versions'][] = (int) $version;
      }
      else {
        drush_set_error('VERSION_NUMBER_NOT_VALID', NULL, dt('Datastream version number !version is not valid.', array('!version' => $version)));
        drupal_exit();
      }
    }
  }
  else {
    preg_match("/(?<start>\S*)\.\.(?<end>\S*)/", $versions, $matches);
    $matches = array_filter($matches);
    if ((isset($matches['start']) && !is_numeric($matches['start'])) || (isset($matches['end']) && !is_numeric($matches['end']))) {
      drush_set_error('VERSION_RANGE_NOT_VALID', NULL, dt('Datastream version range includes an invalid start or end value.'));
      drupal_exit();
    }

    if (isset($matches['start']) || isset($matches['end'])) {
      if (isset($matches['start']) && isset($matches['end'])) {
        // Both ends of the range were provided.
        $result['range']['start'] = (int) min($matches['start'], $matches['end']);
        $result['range']['end'] = (int) max($matches['start'], $matches['end']);
      }
      else {
        // Only one end of the range was provided, so the "start" and "end" keys matter.
        $result['range']['start'] = !empty($matches['start']) ? (int) $matches['start'] : 0;
        $result['range']['end'] = !empty($matches['end']) ? (int) $matches['end'] : 999999;
      }
    }
    else {
      drush_set_error('VERSION_RANGE_NOT_VALID', NULL, dt('Datastream version range is not valid.'));
      drupal_exit();
    }
  }
  return $result;
}
