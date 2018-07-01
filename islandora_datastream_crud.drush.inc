<?php

/**
 * @file
 * Drush file for the Islandora Datastream CRUD module.
 */

/**
 * Implements hook_drush_command().
 */
function islandora_datastream_crud_drush_command() {
  $items = array();
  $items['islandora_datastream_crud_fetch_pids'] = array(
    'aliases' => array('idcrudfp'),
    'description' => 'Fetch PIDs from Islandora for the objects whose datastreams you want to update. The `--collection`, `--content_model`, `--namespace`, and `--solr_query` options, if present, are ANDed together.',
    'examples' => array(
      'drush islandora_datastream_crud_fetch_pids --user=admin --collection=islandora:sp_basic_image_collection --pid_file=/tmp/imagepids.txt',
      'drush islandora_datastream_crud_fetch_pids --user=admin --content_model=islandora:sp_basic_image --collection=islandora:sp_basic_image_collection --pid_file=/tmp/imagepids.txt',
    ),
    'options' => array(
      'content_model' => array(
        'description' => 'The content model of the objects you want to update.',
      ),
      'namespace' => array(
        'description' => 'The namespace of the objects you want to update.',
      ),
      'collection' => array(
        'description' => 'The PID of the collection from which to select the objects.',
      ),
      'is_member_of' => array(
        'description' => 'The PID of the parent with the isMemberOf relationship.',
      ),
      'with_dsid' => array(
        'description' => 'Limit to objects that have a datastream with the specified datastream ID.',
      ),
      'without_dsid' => array(
        'description' => 'Limit to objects that do not have a datastream with the specified datastream ID.',
      ),
      'solr_query' => array(
        'description' => 'A raw Solr query. If the --solr_query option is present, it overrides --content_model, --namespace, and --collection.',
      ),
      'pid_file' => array(
        'description' => 'Absolute path to the file where you want the PID list to be saved. If absent, PIDs will be written to STDOUT.',
      ),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );
  $items['islandora_datastream_crud_fetch_datastreams'] = array(
    'aliases' => array('idcrudfd'),
    'description' => 'Fetch files from Islandora corresponding to the datastreams you want to update.',
    'examples' => array(
      'drush islandora_datastream_crud_fetch_datastreams --user=admin --pid_file=/tmp/imagepids.txt --dsid=MODS --datastreams_directory=/tmp/imagemods',
      'drush islandora_datastream_crud_fetch_datastreams --user=admin --pid_file=/tmp/imagepids.txt --dsid=OBJ --datastreams_directory=/tmp/imagecollection_objs',
    ),
    'options' => array(
      'dsid' => array(
        'description' => 'Datastream ID.',
        'required' => TRUE,
      ),
      'pid_file' => array(
        'description' => 'Absolute path to the file that lists PIDs for objects you want to fetch datastreams from.',
        'required' => TRUE,
      ),
      'datastreams_directory' => array(
        'description' => 'Absolute path to the directory where you want the datastream files to be saved.',
        'required' => TRUE,
      ),
      'datastreams_extension' => array(
        'description' => "Optional. The file extension, without the period, to give the datastream files. If absent, Islandora will assign an extension based on the datastream's MIME type.",
      ),
      'datastreams_version' => array(
        'description' => 'Optional. The version number of the datastream. 0 is the current version (the default, so you wouldn\'t normally specify it), 1 is the previous version, 2 is the version before that, etc.',
      ),
      'pause' => array(
        'description' => 'Optional. Number of seconds to pause before fetching the next datastream.',
      ),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );
  $items['islandora_datastream_crud_push_datastreams'] = array(
    'aliases' => array('idcrudpd'),
    'description' => 'Add or replace datastrems. This operation writes datastreams to objects in your repository.',
    'examples' => array(
      'drush islandora_datastream_crud_push_datastreams --user=admin --datastreams_source_directory=/tmp/imagecollection_objs',
      'drush islandora_datastream_crud_push_datastreams --user=admin --no_derivs --datastreams_source_directory=/tmp/imagecollection_objs',
      'drush islandora_datastream_crud_push_datastreams --user=admin --datastreams_source_directory=/tmp/imagecollection_objs --pid_file=/tmp/wanted_pids.txt',
      'drush islandora_datastream_crud_push_datastreams --user=admin --datastreams_directory=/tmp/my_new_datastreams --pid_file=/tmp/pids_for_target_objects.txt --datastream_label="My new datastream" --datastream_mimetype=application/foo',
    ),
    'options' => array(
      'datastreams_source_directory' => array(
        'description' => 'Absolute path to the directory where updated datastream files are stored.',
        'required' => TRUE,
      ),
      'pid_file' => array(
        'description' => 'Optional. Absolute path to the file that lists PIDs for objects you want to push datastreams to. If absent, all of the datastreams in the source directory will be pushed',
      ),
      'datastreams_mimetype' => array(
        'description' => 'Optional. The MIME type to apply to the new datastreams. If absent, the existing MiME type is used.',
      ),
      'datastreams_label' => array(
        'description' => 'Optional. The label to apply to the new datastreams. Enclose in quotation marks. If absent and the datastream exists, the original label is retained. If absent and the datastream is being created, no label is applied.',
      ),
      'datastreams_crud_log' => array(
        'description' => 'Optional. Absolute path to the CRUD log. If present, the PID, DSID, and a message will be written to the specified file.' ,
      ),
      'pause' => array(
        'description' => 'Optional. Number of seconds to pause before pushing the next datastream.',
      ),
      'datastreams_revert' => array(
        'description' => 'Optional. Include if the push is to revert datastream versions.',
        'value' => 'optional',
      ),
      'update_object_label' => array(
        'description' => 'Optional. Include if the push of DC datastream is to update each object\'s label derived from DC title element. Will be deprecated at end of 2017 as described in issue 38.',
        'value' => 'optional',
      ),
      'update_dc' => array(
        'description' => 'Optional. Set to 0 (zero) if you do not want to update your DC datastreams from the MODS or other XML datastreams ou are pushing. Default is 1, but you will be prompted to confirm the DC update.',
      ),
      'dc_transform' => array(
        'description' => 'Optional. Absolute path to the XSLT stylesheet to use for regenerating DC from MODS or other XML datastream.',
      ),
      'no_derivs' => array(
        'description' => 'Optional. Turns off all derivative generation that would otherwise result from pushing datastreams.',
        'value' => 'optional',
      ),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );

  $items['islandora_datastream_crud_delete_datastreams'] = array(
    'aliases' => array('idcruddd'),
    'description' => 'Delete datastreams.',
    'examples' => array(
      'drush islandora_datastream_crud_delete_datastreams --user=admin --dsid=FOO --pid_file=/tmp/delete_foo_from_these_objects.txt',
    ),
    'options' => array(
      'pid_file' => array(
        'description' => 'Absolute path to the file that lists PIDs for objects you want to delete datastreams from.',
        'required' => TRUE,
      ),
      'dsid' => array(
        'description' => 'Datastream ID.',
        'required' => TRUE,
      ),
      'datastreams_crud_log' => array(
        'description' => 'Optional. Absolute path to the CRUD log. If present, the PID, DSID, and a message will be written to the specified file.',
      ),
      'pause' => array(
        'description' => 'Optional. Number of seconds to pause before deleting the next datastream.',
      ),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );

  $items['islandora_datastream_crud_generate_derivatives'] = array(
    'aliases' => array('idcrudgd'),
    'description' => 'Generate or regenerate derivative datastreams from a source DSID.',
    'examples' => array(
      'drush islandora_datastream_crud_generate_derivatives --user=admin --source_dsid=OBJ --pid_file=/tmp/regenerate_derivatives_for_these_objects.txt',
    ),
    'options' => array(
      'pid_file' => array(
        'description' => 'Absolute path to the file that lists PIDs for objects you want to generate derivatives for.',
        'required' => TRUE,
      ),
      'source_dsid' => array(
        'description' => 'The source datastream ID.',
        'required' => TRUE,
      ),
      'dest_dsids' => array(
        'description' => 'Optional. A comma-separated list of datastream IDs to generate. If absent, all derivatives will be generated.',
      ),
      'skip_dsids' => array(
        'description' => 'Optional. A comma-separated list of datastream IDs to skip. If absent, all derivatives will be generated.',
      ),
      'datastreams_crud_log' => array(
        'description' => 'Optional. Absolute path to the CRUD log. If present, the PID, DSID, and a message will be written to the specified file.',
      ),
      'pause' => array(
        'description' => 'Optional. Number of seconds to pause between objects.',
      ),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );

  $items['islandora_datastream_crud_update_object_properties'] = array(
    'aliases' => array('idcrudop'),
    'description' => 'Update object properties.',
    'examples' => array(
      'drush islandora_datastream_crud_update_object_properties --user=admin --pid_file=/tmp/update_these_objects.txt',
    ),
    'options' => array(
      'pid_file' => array(
        'description' => 'Absolute path to the file that lists PIDs for objects whose properties you want to update.',
        'required' => TRUE,
      ),
      'owner' => array(
        'description' => "The object's owner.",
      ),
      'state' => array(
        'description' => "The object's state (A/I/D).",
      ),
      'update_object_label' => array(
        'description' => "Include if you want to update the object's label from either a MODS or DC element.",
        'value' => 'optional' ,
      ),
      'source_dsid' => array(
        'description' => "Tells the 'update_object_label' option which datastream to use as the source of the label.",
      ),
      'source_xpath' => array(
        'description' => "Xpath expression that tells the 'update_object_label' option where to find the label value. Must be wrapped in double quotation marks.",
      ),
      'concat_string' => array(
        'description' => "String used to join repeated values retrieved by --source_xpath. Default is a single space.",
      ),
      'datastreams_crud_log' => array(
        'description' => 'Optional. Absolute path to the CRUD log. If present, the PID, old value of the property, and new value of the property will be written to the specified file.',
      ),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );

  return $items;
}

/**
 * Fetches a list of PIDs from Solr.
 */
function drush_islandora_datastream_crud_fetch_pids() {
  if (!module_exists('islandora_solr')) {
    drush_set_error('SOLR_SEARCH_NOT_AVAILABLE',
      dt('Sorry, Islandora Solr Search not enabled.'));
  }

  // If applicable, check that the specified parent object exists and
  // is accessible; if it's not, exit with an error.
  if (drush_get_option('is_member_of')) {
    if (!islandora_object_load(drush_get_option('is_member_of'))) {
      drush_set_error('PARENT_NOT_FOUND',
        dt('The specified parent object (!is_member_of) is not found or is not accessible.',
        array('!is_member_of' => drush_get_option('is_member_of'))));
    }
  }

  // If applicable, check that the specified collection object exists and
  // is accessible; if it's not, exit with an error. Also confirm that the
  // object has the expected content model.
  if (drush_get_option('collection')) {
    if (!islandora_object_load(drush_get_option('collection'))) {
      drush_set_error('COLLECTION_NOT_FOUND',
        dt('The specified collection object (!collection) is not found or is not accessible.',
        array('!collection' => drush_get_option('collection'))));
    }
    else {
      $collection_object = islandora_object_load(drush_get_option('collection'));
      $collection_cmodels = $collection_object->models;
      if ($collection_cmodels[0] != 'islandora:collectionCModel') {
        drush_set_error('OBJECT_IS_NOT_COLLECTION',
          dt('The specified collection object (!collection) does not have a content model of islandora:collectionCModel.',
          array('!collection' => drush_get_option('collection'))));
      }
    }
  }

  // Build the Solr query.
  $query = '';
  $query_parts = array();
  if (drush_get_option('is_member_of')) {
    $query_parts[] = 'RELS_EXT_isMemberOf_uri_t' . ':"' . drush_get_option('is_member_of') . '"';
  }
  if (drush_get_option('collection')) {
    $query_parts[] = 'RELS_EXT_isMemberOfCollection_uri_t' . ':"' . drush_get_option('collection') . '"';
  }
  if (drush_get_option('content_model')) {
    $query_parts[] = 'RELS_EXT_hasModel_uri_t' . ':' . '"info:fedora/' . drush_get_option('content_model') . '"';
  }

  if (count($query_parts) > 1) {
    $query = implode(' AND ', $query_parts);
  }
  else {
    $query = $query_parts[0];
  }

  if (drush_get_option('namespace')) {
    // Other options are present.
    if (strlen($query)) {
      $query .= ' AND PID:' . drush_get_option('namespace') . '\:*';
    }
    else {
      // Only option is --namespace.
      $query = 'PID:' . drush_get_option('namespace') . '\:*';
    }
  }

  if (drush_get_option('with_dsid')) {
    // Other options are present.
    if (strlen($query)) {
      $query .= ' AND fedora_datastreams_mt:' . drush_get_option('with_dsid');
    }
    else {
      // Only option is --with_dsid.
      $query = 'fedora_datastreams_mt:' . drush_get_option('with_dsid');
    }
  }

  if (drush_get_option('without_dsid')) {
    // Other options are present.
    if (strlen($query)) {
      $query .= ' AND -fedora_datastreams_mt:' . drush_get_option('without_dsid');
    }
    else {
      // Only option is --without_dsid.
      $query = '-fedora_datastreams_mt:' . drush_get_option('without_dsid');
    }
  }

  // --query overrides all other options.
  if (drush_get_option('solr_query')) {
    $query = drush_get_option('solr_query');
  }

  // Get the PIDs from the Solr query.
  module_load_include('inc', 'islandora_datastream_crud', 'includes/utilities');
  $pids = islandora_datastream_crud_query_solr($query);

  if (!count($pids) || !$pids) {
    drush_set_error('NO_PIDS_FOUND',
      dt('Sorry, no PIDS were found.'));
  }
  else {
    foreach ($pids as $pid) {
      if (drush_get_option('pid_file')) {
        file_put_contents(drush_get_option('pid_file'), $pid . PHP_EOL, FILE_APPEND);
      }
      else {
        drush_print($pid);
      }
    }
    if (drush_get_option('pid_file')) {
      drush_log(dt("PIDS written to file !pid_file", array('!pid_file' => drush_get_option('pid_file'))), 'ok');
    }
  }
}

/**
 * Iterates through the list of PIDs and fetches the content of a datastream.
 *
 * This is the 'Read' function in CRUD.
 */
function drush_islandora_datastream_crud_fetch_datastreams() {
  module_load_include('inc', 'islandora_datastream_crud', 'includes/utilities');
  $dsid = (drush_get_option('dsid'));
  $datastreams_directory = drush_get_option('datastreams_directory');
  $pid_file = (drush_get_option('pid_file'));

  if (file_exists($datastreams_directory)) {
    if (!drush_confirm(dt("Datastreams directory !dir already exists. Any " .
      "datastream files from the same object PID/datastream ID combination " .
      "that already exist in it will be overwritten. Do you want to want to " .
      "continue?",
      array('!dir' => $datastreams_directory)))) {
      drush_user_abort(dt('Exiting, no datastreams fetched.'));
      exit;
    }
  }
  else {
    if (drush_confirm(dt("Datastreams directory !dir does not exist. Do you " .
      "want to create it and continue?",
      array('!dir' => $datastreams_directory)))) {
      mkdir($datastreams_directory);
    }
    else {
      drush_user_abort(dt('Exiting, no datastreams fetched.'));
      exit;
    }
  }

  $pids = islandora_datastream_crud_read_pid_file(drush_get_option('pid_file'));
  if (!count($pids)) {
    drush_set_error('NO_PIDS_IN_PID_FILE',
    dt('The specified PID file (!pid_file) contains no PIDS.',
      array('!pid_file' => drush_get_option('pid_file'))));
  }
  foreach ($pids as $pid) {
    if (drush_get_option('pause')) {
      sleep(drush_get_option('pause'));
    }
    if (islandora_datastream_crud_retrieve_datastream($pid, $dsid, $datastreams_directory)) {
      drush_log(dt("!dsid datastream for object !pid retrieved", array('!dsid' => $dsid, '!pid' => $pid)), 'ok');
    }
    else {
      drush_log(dt("Datastream !dsid not retrieved for object !pid",
        array('!dsid' => $dsid, '!pid' => $pid)), 'warning');
    }
  }
}

/**
 * Ingests the datastream files into the repository.
 *
 * This is the 'Update' function in CRUD, and
 * is also part of the 'Create' function in CRUD.
 */
function drush_islandora_datastream_crud_push_datastreams() {
  if (drush_get_option('datastreams_revert')) {
    if (!drush_confirm(dt("You are about to push datastreams to objects " .
      "in your repository. This will revert old versions of the datastreams. " .
      "Do you want to want to continue?",
      array()))) {
      drush_user_abort(dt('Exiting, no datastreams pushed.'));
      exit;
    }
  }
  else {
    if (!drush_confirm(dt("You are about to push datastreams to objects " .
      "in your repository. This will create new versions of the datastreams, or " .
      "create new datastreams if none exist. Do you want to want to continue?",
      array()))) {
      drush_user_abort(dt('Exiting, no datastreams pushed.'));
      exit;
    }
  }

  $source_directory = drush_get_option('datastreams_source_directory');
  if (!file_exists($source_directory)) {
    drush_set_error('DATASTREAM_SOURCE_DIRECTORY_NOT_FOUND',
    dt('The specified datastream source directory (!dir) does not exist.',
      array('!dir' => $source_directory)));
  }

  module_load_include('inc', 'islandora_datastream_crud', 'includes/utilities');
  // If --pid_file was specified, only push to objects that have a PID there.
  if (drush_get_option('pid_file')) {
    $pids = islandora_datastream_crud_read_pid_file(drush_get_option('pid_file'));
    if (!count($pids)) {
      drush_set_error('NO_PIDS_IN_PID_FILE',
      dt('The specified PID file (!pid_file) contains no PIDS.',
        array('!pid_file' => $pid_file)));
    }
  }

  // We only provide the option to generate DC from datastream files that
  // end in .xml. We need to get one such fileaname so we can determine
  // which DSID we are generating the DC from.
  $regenerate_dc = FALSE;
  $xml_files = array_slice(array_filter(glob($source_directory .
    DIRECTORY_SEPARATOR . '*.xml'), 'is_file'), 0, 1);
  if (count($xml_files) === 1) {
    // Note: https://github.com/mjordan/islandora_datastream_crud/issues/33
    // may make separator not be an _.
    list($namespace, $restofthepid, $dsid) = explode('_', pathinfo($xml_files[0], PATHINFO_FILENAME));

    if (drush_get_option('update_dc', 1)) {
      if (drush_confirm(dt("Do you want to update each object's DC datastream using the new !dsid?",
        array('!dsid' => $dsid)))) {
        $regenerate_dc = TRUE;
      }
    }
  }

  foreach (glob("$source_directory/*") as $filepath) {
    if (drush_get_option('pause')) {
      sleep(drush_get_option('pause'));
    }
    if (drush_get_option('pid_file')) {
      if ($pid = islandora_datastream_crud_dsfilename_to_pid($filepath)) {
        if (in_array($pid, $pids)) {
          islandora_datastream_crud_push_datastream($filepath, $regenerate_dc);
        }
      }
      else {
        continue;
      }
    }
    else {
      islandora_datastream_crud_push_datastream($filepath, $regenerate_dc);
    }
  }
}


/**
 * Deletes datastreams from objects.
 *
 * This is the 'Delete' function in CRUD.
 */
function drush_islandora_datastream_crud_delete_datastreams() {
  $dsid = drush_get_option('dsid');
  if ($dsid == 'DC') {
    drush_user_abort(dt('DC is a required datastream and cannot be deleted.'));
    exit;
  }
  if (!drush_confirm(dt("You are about to delete (purge) the !dsid " .
    "datastream from a set of objects in your repository. This action " .
    "cannot be undone. Do you want to want to continue?",
    array('!dsid' => drush_get_option('dsid'))))) {
    drush_user_abort(dt('Exiting, no datastreams deleted.'));
    exit;
  }

  module_load_include('inc', 'islandora_datastream_crud', 'includes/utilities');
  $pids = islandora_datastream_crud_read_pid_file(drush_get_option('pid_file'));
  if (!count($pids)) {
    drush_set_error('NO_PIDS_IN_PID_FILE',
    dt('The specified PID file (!pid_file) contains no PIDS.',
      array('!pid_file' => $pid_file)));
  }
  foreach ($pids as $pid) {
    if (drush_get_option('pause')) {
      sleep(drush_get_option('pause'));
    }
    islandora_datastream_crud_delete_datastream($pid, $dsid);
  }
}

/**
 * Regenerates derivative datastreams.
 */
function drush_islandora_datastream_crud_generate_derivatives() {
  module_load_include('inc', 'islandora_datastream_crud', 'includes/utilities');
  $pids = islandora_datastream_crud_read_pid_file(drush_get_option('pid_file'));
  $dsid = drush_get_option('source_dsid');
  if (!drush_confirm(dt("You are about to regenerate derivatives from the " .
    "!dsid datastream for !num object(s) in your repository. This action " .
    "cannot be undone. Do you want to want to continue?",
    array('!num' => count($pids), '!dsid' => $dsid)))) {
    drush_user_abort(dt('Exiting, no derivatives generated.'));
    exit;
  }

  module_load_include('inc', 'islandora', 'includes/derivatives');
  if (!count($pids)) {
    drush_set_error('NO_PIDS_IN_PID_FILE',
    dt('The specified PID file (!pid_file) contains no PIDS.',
      array('!pid_file' => $pid_file)));
  }
  foreach ($pids as $pid) {
    if (drush_get_option('pause')) {
      sleep(drush_get_option('pause'));
    }
    // Load the object.
    if (!$object = islandora_object_load($pid)) {
      drush_set_error('IBD_OBJECT_NOT_FOUND',
        dt('The specified object (!pid) does not exist or is not accessible.',
          array('!pid' => $pid)));
      continue;
    }
    // Regenerate derivatives.
    drush_log(dt('Please be patient, generating derivatives from the !dsid datastream for !pid',
      array('!dsid' => $dsid, '!pid' => $pid)), 'ok');
    $options = array('force' => TRUE, 'source_dsid' => $dsid);
    $messages = islandora_do_derivatives($object, $options);
    // Print the output from the derivative generation functions.
    foreach ($messages as $message) {
      if ($message['success']) {
        foreach ($message['messages'] as $ds_message) {
          drush_log(strip_tags(dt($ds_message['message'], $ds_message['message_sub'])), 'ok');
        }
      }
      else {
        foreach ($message['messages'] as $ds_message) {
          drush_set_error('DERIVATIVE_NOT_GENERATED', strip_tags(dt($ds_message['message'], $ds_message['message_sub'])));
        }
      }
    }
  }
}

/**
 * Updates object properties.
 */
function drush_islandora_datastream_crud_update_object_properties() {
  module_load_include('inc', 'islandora_datastream_crud', 'includes/utilities');
  $pids = islandora_datastream_crud_read_pid_file(drush_get_option('pid_file'));

  $confirmation_message = '';
  $props = array();
  if (drush_get_option('owner')) {
    $props['owner'] = drush_get_option('owner');
    $confirmation_message = 'owner, ';
  }
  if (drush_get_option('state')) {
    $props['state'] = drush_get_option('state');
    $confirmation_message .= 'state, ';
  }

  if (drush_get_option('update_object_label')) {
    $confirmation_message .= 'label';
  }
  $confirmation_message = rtrim($confirmation_message, " ,");

  if (!drush_confirm(dt("You are about to change the following properties" .
    " of !num objects in your repository: !properties. This action" .
    " cannot be undone. Do you want to want to continue?",
    array('!num' => count($pids), '!properties' => $confirmation_message)))) {
    drush_user_abort(dt('Exiting, no objects updated.'));
    exit;
  }

  if (!count($pids)) {
    drush_set_error('NO_PIDS_IN_PID_FILE',
    dt('The specified PID file (!pid_file) contains no PIDS.',
      array('!pid_file' => $pid_file)));
  }
  foreach ($pids as $pid) {
    if (drush_get_option('pause')) {
      sleep(drush_get_option('pause'));
    }
    // Load the object.
    if (!$object = islandora_object_load($pid)) {
      drush_set_error('IBD_OBJECT_NOT_FOUND',
        dt('The specified object (!pid) does not exist or is not accessible.',
          array('!pid' => $pid)));
      continue;
    }

    // Update the object's properties.
    islandora_datastream_crud_update_object_properties($object, $props);
  }
}

/**
 * Validate function for islandora_datastream_crud_update_object_properties().
 */
function drush_islandora_datastream_crud_update_object_properties_validate() {
  if (drush_get_option('state') &&
    !in_array(drush_get_option('state'), array('A', 'I', 'D'))) {
    drush_set_error('INVALID_OBJECT_STATE_PROVIDED', NULL, dt("Value of the --state option must be one of 'A', 'I', or 'D'. "));
  }
}

/**
 * Implements hook_islandora_derivative_alter().
 */
function islandora_datastream_crud_islandora_derivative_alter(&$derivatives, AbstractObject $object) {
  if (drush_get_option('no_derivs', FALSE)) {
    $derivatives = array();
  }
  if (drush_get_option('dest_dsids', FALSE)) {
    $dsid_list = drush_get_option('dest_dsids');
    $dsid_list = rtrim($dsid_list, ',');
    $dsids = explode(',', $dsid_list);
    foreach ($dsids as &$dsid) {
      $dsid = trim($dsid);
    }
    foreach ($derivatives as $key => $derivative) {
      if (!in_array($derivative['destination_dsid'], $dsids)) {
        unset($derivatives[$key]);
      }
    }
  }
  if (drush_get_option('skip_dsids', FALSE)) {
    $dsid_list = drush_get_option('skip_dsids');
    $dsid_list = rtrim($dsid_list, ',');
    $dsids = explode(',', $dsid_list);
    foreach ($dsids as &$dsid) {
      $dsid = trim($dsid);
    }
    foreach ($derivatives as $key => $derivative) {
      if (in_array($derivative['destination_dsid'], $dsids)) {
        unset($derivatives[$key]);
      }
    }
  }
}
