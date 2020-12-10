# Islandora Datastream CRUD  [![Build Status](https://travis-ci.org/SFULibrary/islandora_datastream_crud.png?branch=7.x)](https://travis-ci.org/SFULibrary/islandora_datastream_crud)

Islandora Drush module for performing Create, Read, Update, and Delete operations on datastreams. If you are looking for a web-based tool to modify XML and text datastreams, check out [Islandora Find & Replace](http://www.contentmath.com/articles/2016/4/11/islandora-find-replace-admin-form-to-batch-update-datastreams).

## Requirements

* [Islandora](https://github.com/Islandora/islandora)
* [Islandora Solr Search](https://github.com/Islandora/islandora_solr_search)

## Usage

Commands are inspired by a simple Git workflow (fetch, push, delete).

* `drush islandora_datastream_crud_fetch_pids --user=admin --collection=islandora:sp_basic_image_collection --pid_file=/tmp/imagepids.txt`
* `drush islandora_datastream_crud_fetch_datastreams --user=admin --pid_file=/tmp/imagepids.txt --dsid=MODS --datastreams_directory=/tmp/imagemods`
* `drush islandora_datastream_crud_push_datastreams --user=admin --datastreams_mimetype=image/jpeg --datastreams_source_directory=/tmp/imagemods_modified --datastreams_crud_log=/tmp/crud.log`
* `drush islandora_datastream_crud_delete_datastreams --user=admin --dsid=FOO --pid_file=/tmp/delete_foo_from_these_objects.txt`
* `drush islandora_datastream_crud_generate_derivatives --user=admin --source_dsid=OBJ --pid_file=/tmp/regenerate_derivatives_for_these_objects.txt`

Note that you can include Drush's `-y` option to bypass confirmation prompts. This is useful if you are running Datastream CRUD in a `nohup` or scripted environment.

If you would prefer to avoid using Drush, [Islandora Datastreams Input/Output](https://github.com/ulsdevteam/islandora_datastreams_io) provides a way to run Datastream CRUD from within Drupal's administrative user interface.

## Fetching PIDs

The `islandora_datastream_crud_fetch_pids` command provides several options for specifying which objects you want datastreams from:

* `--namespace`: Lets you specify a namespace.
* `--collection`: Lets you specify a collection PID.
* `--is_member_of`: Lets you specify relationships to another parent object PID such as a Newspaper or Book.
* `--content_model`: Lets you specify a content model PID.
* `--without_cmodel`: Excludes objects with a specified content model from the results.
* `--with_dsid`: Lets you specify the ID of a datastream that objects must have.
* `--without_dsid`: Lets you specify the ID of a datastream that objects must not have.
* `--solr_query`: A raw Solr query. For example, `--solr_query=*:*` will retrieve all the PIDs in your repository; `--solr_query=dc.title:foo` will retrieve all the PIDs of objects that have the string 'foo' in their DC title fields; `--solr_query="RELS_EXT_isMemberOf_uri_s:info\:fedora/dailyplanet\:1"`will retrieve all newspaper issues that are part of the newspaper "dailyplanet:1". For a more complex query, `--solr_query="RELS_EXT_isMemberOfCollection_uri_ms:info\:fedora\/ir\:citationCollection AND dc.title:citation AND -mods_genre_ms:Article"` will return all objects in the "ir:citationCollection" collection with a title containing the word "citation" but without the genre "Article".

These options generate a Solr query that is used to fetch the object PIDs. If multiple options are present, they are ANDed together, so you can, for example, retrieve PIDs of objects that have a specific `--namespace` within a particular `--collection`.

If the `--solr_query` option is used, it overrides all other options.

You typically save the fetched PIDs to a PID file, whose path is specified using the `--pid_file` option. See 'The PID file' section below for more information.

### Solr query examples

* Versioned datastreams:
  * To fetch PIDs only for objects that have datastreams with multiple versions, use the `fedora_datastream_(DSID)_ms` field. For example: `drush -u 1 islandora_datastream_crud_fetch_pids --pid_file=/home/user/verisoned_pids_to_delete.txt --solr_query="fedora_datastreams_mt:JP2 AND fedora_datastream_version_JP2_ID_ms:JP2.1"` finds all objects that have a JP2 datastream with a version 1 or higher.

## General workflow

The general workflow when using this module is:

1. Fetch some PIDs from Islandora.
2. Fetch a specific datastream from each of the objects identified by your PIDs (this saves the datastream content in a set of files, one per datastream).
3. Update or modify the fetched datastream files.
4. Ensure that the modified datastream files are what you want to push to your repository. This module provides a way to roll back or revert changes made as a result of issuing `islandora_datastream_crud_push_datastreams`, but that should not prevent you from performing quality control before you push.
5. Push the updated datasteam files back to the objects they belong to.

Steps 1, 2, and 5 involve running the appropriate drush command. Steps 3 and 4 deserve special explanation.

### Step 3

This module doesn't do anything in Step 3 (update or modify the fetched datastream files). That's up to you. For example, if you fetched all of the TIFF OBJ datastreams from a collection, you could correct the color in the TIFFs using Photoshop, and then push them to your repository. Three sample scripts that modify datastream files are included in the `scripts` directory:

* a PHP script that appends an element/fragment to a set of XML files
* a shell script that adds a label/watermark to a set of image files
* a PHP script that converts JPEG2000 files from those extracted from CONTENTdm to ones usable in Islandora 
* a PHP script that performs search and replace on a directory of files

You may not find a use for these three scripts, but they illustrate the kinds of things you may want to do to datastream files.

### Step 4

This is a real step. Skip it at your own peril, doom, and ruin.

* It is extremely important that you are sure the datastream files you have modified or prepared are ready to push to your repository.
* You should perform the same types of QA and checking on these files that you perform prior to doing a batch ingest (validate the MODS, etc.).
* It would be prudent to push a small number of test datastream files to your repository before pusing the entire set, and to push the datastream files in small subsets and perform QA on the modified datastreams in your repository before pushing more.

### Step 5
Push the updated datasteam files back to the objects they belong to.

* The islandora_datastream_crud_push_datastreams command does not require a pid list
* The directory of updated files to push is referenced with the argument `datastreams_source_directory`

As an example: you have pulled 100 MODS files from the MODS datastream of 100 records and updated the files to your satisfaction - the files are in a directory `clean_mods` pushing these back to their records may be done with this command: `drush islandors_datastream_crud_push_datastreams --user=admin --datastreams_source_directory=clean_mods`


## Specific workflows

### Creating new datastreams

A subset of the workflow outlined above adds new (i.e., previously nonexistent) datastreams to a set of objects. In this case, you wouldn't fetch datastream files from Islandora, you would prepare the new datastream files using some external process, and name them using the required filenames. You would then issue an `islandora_datastream_crud_push_datastreams` command to add them to the objects identified in the filenames. You would most likely also want to provide a label for your new datastreams with the `--datastreams_label` option.

You can add multiple datastreams to one object using a single command. Given a directory containing datastream files named using the same PID (in this example, "test:1300"), like this:

```
test_1300_DS1.txt
test_1300_DS2.txt
test_1300_DS3.txt
```
the following command would add three datastreams (DS1, DS2, and DS3, all with a MIME type of "text/plain" and a label of "My label") to the object:

`drush islandora_datastream_crud_push_datastreams --user=admin --datastreams_source_directory=/tmp/pushtest  --datastreams_mimetype=text/plain  --datastreams_label="My label"`. Notice that the `--pid_file` option is absent.

### Deleting datastreams

Another subset of the general workflow in which you do not fetch datastreams is to delete a datastream from a set of objects. To do this, you only need to specify the objects you want to delete the datastream from in the PID file, and the datastream ID you want to delete.

Optionally, you can also specify datastream version numbers to be deleted, either as a list of version numbers, or as a range of version numbers. This can be helpful, for example, if you want to delete all but the current version (0) of a datastream on many objects.

To specify a datastream version, use the `--versions=` parameter with one of the following values:

* `0` deletes version 0 (the current version of the datastream)
* `1` deletes version 1 (the second-newest version)
* `1..` deletes all versions from 1 and older
* `5..1` deletes all versions between 1 and 5
* `1,4,3` deletes those specific individual datastream versions

Example: `drush islandora_datastream_crud_delete_datastreams --user=admin --dsid=MODS --versions=1.. --pid_file=/tmp/ds_versions_delete.txt`

Note that you cannot delete the DC datastream. Fedora Commons requires that each object has a DC datastream.

### Exporting datastreams

If you want to export a set of datastreams from our repository, the `islandora_datastream_crud_fetch_datastreams` command provides a simple way to do so.

### Triggering derivative generation

> Before running the command described in this section, `islandora_datastream_crud_generate_derivatives`, with either the `--dest_dsids` or `--skip_dsids` options, you should enable "Defer derivative generation during ingest" option in your site's Islandora > Configuration menu. Doing this will ensure that additional datastreams are not generated unintentionally.

This module offers a command, `islandora_datastream_crud_generate_derivatives`, that will generate all the derivatives from the specified source datastream ID. For example, to generate or regenerate all the derivatives based on the OBJ datastream, you would issue the following command:

`drush islandora_datastream_crud_generate_derivatives --user=admin --source_dsid=OBJ --pid_file=/tmp/regenerate_derivatives_for_these_objects.txt`

Note that this command does not download datastream files from your repository; it generates the derivatives directly from the datastream identified by the `--source_dsid` option.

`islandora_datastream_crud_generate_derivatives` provides two options to let you limit what derivatives are generated:

* e.g., `--dest_dsids=TN`
* e.g., `--skip_dsids=OCR,TECHMD`

The former restricts the generated derivatives to a comma-separated list of DSIDs, and the latter generates all derivatives other than the ones specified in a comma-separated list. The two options are meant to be mutually exclusive and should not be used together.

You can also trigger derivative generation/regeneration on objects if you push OBJ datastreams up. A plausible scenario where you may want to do this is if a batch ingest fails during the derivative generation phase. By fetching a list of PIDs using the `--without_dsid` option with the ID of a derivative datastream, you can then fetch those objects' OBJ datastreams and push them back up. Not the most efficient way to trigger datastream generation. You should use this option if you want to replace the source datastream; use `islandora_datastream_crud_generate_derivatives` if just want to regenerate derivatives from an existing source datastream. Note that if you use this technique, you should disable (the normal state) "Defer derivative generation during ingest" option in your site's Islandora > Configuration menu.

### Updating DC datastreams by pushing other XML datastreams

Islandora's default behavior is to not regenerate an object's DC datastream when its MODS datastream is replaced. Islandora Datastream CRUD lets you override this behavior by regenerating your objects' DC datastreams when pushing MODS or other XML datastreams. When you push datastream files that end in '.xml', you will be prompted to confirm that you want to regenerate your DC:

`Do you want to update each object's DC datastream using the new MODS? (y/n)`

If you respond `y`, the DC datastream of each object represented by a pushed MODS datastream will be regenerated; if you reply n, the MODS files will be pushed without regenerating the corresponding DC datastreams, following Islandora's default behavior. The MODS-to-DC XLST stylesheet provided by the Islandora XML Forms module will be used to generate the new DC datastreams unless you provide an alternative path to a stylesheet using the `--dc_transform` option.

If you are pushing non-MODS XML datastream files and want to regenerate the corresponding objects' DC datastreams from the pushed files, you will need to provide your own XSLT transform by specifying its absolute path with the `--dc_transform` parameter, e.g.,:

`--dc_transform=/path/to/my/custom/ddi_to_dc_stylesheet.xsl`

If you are not running Islandora Datastream CRUD interactively but from within a script, provide the standard Drush option `-y` to answer "yes" to all Drush prompts. If you do not want to regenerate your DC datastreams, include the option `--update_dc=0`.

### Reverting datastream versions

This module provides basic support for reverting the content and some attributes (mimetype and label) of a datastream to those of an earlier version. The process has two parts, 1) fetching the version of the datastream you want to revert to and 2) pushing the datastream content and attributes back. Reverting datastream versions requires that you use two special Drush options, shown here at the end of the two commands they apply to: 

* `drush islandora_datastream_crud_fetch_datastreams --user=admin --pid_file=/tmp/imagepids.txt --dsid=MODS --datastreams_directory=/tmp/imagemods --datasteams_version=1`
* `drush islandora_datastream_crud_push_datastreams --user=admin --datastreams_source_directory=/tmp/imagemods_modified --datastreams_crud_log=/tmp/crud.log --datastreams_revert`

`--datastreams_version` indicates the version number of the datastream you want to revert to. 0 is the current version (the default, so you wouldn't normally specify it), 1 is the previous version, 2 is the version before that, etc. `--datastreams_revert` takes no value, but its presence signals `islandora_datastream_crud_push_datastreams` to read a special data file called 'version_data.log' (generated by the `--datastreams_version` option) that contains the fetched datastream version's mimetype and label. These values are reinstated along with the earlier version's content to become the latest version of the datastream. version_data.log also contains a timestamp for each datastream that indicates when the version was created, but that information is not used in the reversion process; it is included solely to provide a way of uniquely identifying the datastream version (just like in the Islandora web interface for reverting versions).

The 'reversion' is not complete - it restores the content of the previous version of the datastream but doesn't restore all of the attributes of the previous version, just its mimetype and label, under the assumption that these two attributes are the most likely to change between versions. This approach mirrors how reverting datastreams works within Islandora's web-based datastream management tools.

If a version number is specificied that does not exist for a given object's datasteam, the datastream is not retrieved but a message to that effect is displayed to the user. For example, a command that fetches version 2 of the MODS datastreams might result in messages like this:

```
MODS datastream for object islandora:1305 retrieved
MODS datastream for object islandora:1306 retrieved
Datastream MODS not retrieved for object islandora:1307
```

This is OK - it just means that you will be able to use the specificed datastream version to replace the current one for objects islandora:1305 and islandora:1306, but not islandora:1307. The command to revert those datastreams will simply skip reverting the datastream for islandora:1307.

## The PID file

The PID file generated by `islandora_datastream_crud_fetch_pids`, and used by the other three commands, simply lists one PID per line, like this:

```
islandora:10
islandora:11
example:5782
# This line will be ignored.
// So will this one.
islandora:948
someothernamespace:1
someothernamespace:2
```

After you generate the PID file, you are free to add additional PIDS or delete PIDS. You don't even need to generate a PID file using the `islandora_datastream_crud_fetch_pids` command as long as your file contains one PID per line.

Lines in the PID file beginning with `#` or `//` are ignored.

## The datastream files

`islandora_datastream_crud_fetch_datastreams` will write the content of the fetched datastreams into the location specified in `--datastreams_directory` with filenames containing the object's PID and the datastream ID in the form `namespace_restofthepid_dsid.ext`. The colon in the PID is replaced with an underscore, and the datastream ID is separated from the PID with another underscore. For example, the MODS datastream from object islandora:11 would be saved as `islandora_11_MODS.xml`.


If `--datastreams_extension` is present, filenames are given its value as their extension. If it is absent, Islandora will assign an extension based on the datastream's MIME type.

As mentioned above, datastream files do not need to be created using `islandora_datastream_crud_fetch_datastreams`. Any files conforming to the expected filenaming pattern will replace existing datastream content using the `islandora_datastream_crud_push_datastreams` command.

### Dealing with underscores in PIDs or datastream IDs

As described in the previous section, by default Islandora Datastream CRUD uses underscores (`_`) in filenames to separate the two parts of each object's PID and the datastream ID from each other. This causes problems if your PIDs or your datastream IDs contain underscores.

If your PIDs contain underscores, or your datastream IDs contain underscores, you should use the `--filename_separator` option with `islandora_datastream_crud_fetch_datastreams` and `islandora_datastream_crud_push_datastreams` so that your PIDs and DSIDs are unambiguously added to (in the case of fetching datastreams) or parsed from (in the case of pushing datastreams) their filenames. Any character other than `:`, `*`, and `/` can be used with this option. For example:

* `drush islandora_datastream_crud_fetch_datastreams --user=admin --pid_file=/tmp/imagepids.txt --dsid=MODS --datastreams_directory=/tmp/imagemods --filename_separator=^`
* `drush islandora_datastream_crud_push_datastreams --user=admin --datastreams_source_directory=/tmp/imagemods_modified --filename_separator=^`

If you use this option, the PID in each filename will contain the standard colon (`:`) and the character specified will be used to separate the PID from the datastream ID. For example, `--filename_separator=^` will produce datastream filenames such as

```
islandora:1^MODS.xml
islandora:2^MODS.xml
islandora:3^MODS.xml
```

PIDs containing underscores, such as `my_namespace:200` and datastream IDs containing underscores, such as `MY_CUSTOM_DS`, will be preserved, e.g. `my_namespace:200^MY_CUSTOM_DS.txt`.

If you do not specify the `--filename_separator` option, Datastream CRUD will use underscores in the datastream filenames.

## Effects of pushing datastreams

Islandora reacts to the replacement of a datastream, the deletion of a datastream, and the addition of a new datastream in several ways:

* Datastreams are versioned in Islandora. Pushing datastreams results in the pushed file content becoming the latest version of the specified datastream. It is possible to revert to a previous version of a datastream using the 'revert' option within an object's datastream management tab, but this module does *not* provide a simple way to roll back or revert changes made as a result of issuing `islandora_datastream_crud_push_datastreams`. To be clear: If you push 10,000 MODS datastreams to your repository using this module and you discover that each one contains a small problem, you'll need to revert those 10,000 datastreams, either manually or using the method described above. Or, push a new set of MODS XML datastream files that do not have the same problem.
  * If you want to push datastreams and turn off all normal derivative creation that would result from pushing those datastreams, include the `--no_derivs` option in the `islandora_datastream_crud_push_datastreams` command. This will disable all derivative generation.
* Pushing datastreams triggers a reindexing of that object in Solr. This applies to all datastreams, not just MODS. Deleting datastreams that are indexed in Solr updates the index to remove the indexed content of the deleted datastreams.
* Replacing the OBJ datastream, or any other datastream from which other datastreams are derived, triggers derivative regeneration as defined by solution packs and other modules. If you do not want derivatives generated as a result of pushing datastreams to your repository, enable "Defer derivative generation during ingest" option in your site's Islandora > Configuration menu. If you enable this option, don't forget to disable after your have pushed your datastreams. Alternatively, you can include the `--no_derivs` option in the `islandora_datastream_crud_push_datastreams` command.
* `islandora_datastream_crud_push_datastreams` does not change the MIME type of the datastream unless the `--datastreams_mimetype` is present. Also, and very importantly, it does not assign a default MIME type when adding the datastream. This means that you must include the `--datastreams_mimetype` option if you are pushing datastreams that do not already exist in the target objects.
* `hook_islandora_datastream_modified()`, `hook_islandora_datastream_ingested()`, `hook_islandora_datastream_purged()`, and their content-model-specific variations are fired when datastreams are replaced, created, and deleted. The effects of this depend on what modules are enabled on your Islandora site.

In general, the behaviors described here are the same regardless of whether the datastream is replaced using the "Replace" link provided in each object's Manage > Datastreams tab (or using the "+ Add a datastream" link), or with this module. However, Islandora Datastream CRUD lets you replace the same datastream across a lot of objects at once, which amplifies the load on your Islandora stack compared to replacing a datastream on a single object. To determine what the potential effect on your site might be, you should update datastreams in small sets before moving on to updating large numbers of datastreams at once.

## Modifying object properties

Even though this module is called Islandora *Datastream* CRUD, it can also modify object properties. It can do this via the `islandora_datastream_crud_update_object_properties` command, which can take the following options:
  * `--pid_file`: Required. Absolute path to the file that lists PIDs for objects whose properties you want to update.
  * `--owner`: Optional. The owner you want to assign to objects identified in `--pid_file`.
  * `--state`: Optional. The state you want to assign to objects identified in `--pid_file`. Must be one of A (for 'active'), I (for 'inactive'), or D for 'deleted').
  * `--update_object_label`: Optional. This option does not take a value, but it is accompanied by the following two other options:
    * `--source_dsid`: Optional; default is 'MODS'. Can be any XML datastream ID.
    * `--source_xpath`: Optional; default is "//mods:titleInfo/title". An XPath expression identifying the value you want to use as the object label. Note that if your XPath expression should be wrapped in double quotes (`"expression"`) so that Drush interacts with the shell properly, e.g., `--source_xpath="//mods:titleInfo[not(@type='alternative')]/mods:title"`.
    * `--add_namespace`: Optional. Use this to register a namespace that may not be explicitly declared in your XML file using `namespace_prefix:namespace_uri` format, e.g. `--add_namespace=mods:http://www.loc.gov/mods/v3`.
  * `--datastreams_crud_log`: Optional. Absolute path to a log file. If present, the PID, old value of the property, and new value of the property will be written to the specified file.

Updating object properties such as owner, state, and label will fire all of the 'hook_islandora_object_modified()' and related implementations enabled on your site. The effects of this are the same as if you updated an object's label or state within its Manage > Properties tab. However, Islandora Datastream CRUD lets you update the properties of a lot of objects at once, which amplifies the load on your Islandora stack compared to updating a property of a single object. TL;DR is it's proobably best to update object properties in small sets and observe any effects the update may have before moving on to updating large numbers of objects at once.

# Controlling load on your Islandora server

Many of Datastream CRUD's operations are fairly resource intensive. If your system experiences stress or crashes while running Datastream CRUD, you may want to use the `--pause` option, which is available in the `islandora_datastream_crud_fetch_datastreams`, `islandora_datastream_crud_push_datastreams`, `islandora_datastream_crud_delete_datastreams`, and `islandora_datastream_crud_generate_derivatives` commands. This option takes a number of seconds as its value, e.g. `--pause=2`, using PHP's `sleep()` function to pause execution for the indicated number of seconds betwen processing objects.

# Maintainer

* [Mark Jordan](https://github.com/mjordan)

## Development and feedback

Pull requests are welcome, as are use cases and suggestions. Please open an issue before creating a pull request.

Scripts that do the work of updating datastreams, especially for MODS datastreams, are also welcome.

## Wishlist

* The `--collection` option for `islandora_datastream_crud_fetch_pids` only retrieves immediate children of the specified collection. If this is a problem for you, and all of the objects in your collection use the same namespace, use the `--namespace` option to get your PIDs instead of the `--collection` option.
* Does not work with datastreams in the (R)edirect and (E)xternal Referenced control groups - use cases are welcome.

## Known Issues

* In sites that use extensive caching (e.g. Cloudflare page rules), objects' Solr documents might not get updated with changes made by Islandora Datastream CRUD (e.g. generating derivatives will not update the `fedora_datastreams_ms` Solr field, which CRUD uses for its `fetch_pids` searches). This could lead to such problems as fetching PIDs using a `--without_dsid` parameter incorrectly fetching PIDs that actually have that DSID. 
  * The only known solution to this problem is to disable page caching before using Islandora Datastream CRUD.

## License

* [GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt) (please review sections 15, 16, and 17 carefully before installing this module).
