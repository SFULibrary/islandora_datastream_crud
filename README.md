# Islandora Datastream CRUD  [![Build Status](https://travis-ci.org/mjordan/islandora_datastream_crud.png?branch=7.x)](https://travis-ci.org/mjordan/islandora_datastream_crud)

Islandora Drush module for performing Create, Read, Update, and Delete operations on datastreams. If you are looking for a web-based tool to modify XML and text datastreams, check out [Islandora Find & Replace](http://www.contentmath.com/articles/2016/4/11/islandora-find-replace-admin-form-to-batch-update-datastreams).

## Requirements

* [Islandora](https://github.com/Islandora/islandora)
* [Islandora Solr Search](https://github.com/Islandora/islandora_solr_search)

## Usage

Commands are inspired by a simple Git workflow (fetch, push, delete).

* `drush islandora_datastream_crud_fetch_pids --user=admin --collection=islandora:sp_basic_image_collection --pid_file=/tmp/imagepids.txt`
* `drush islandora_datastream_crud_fetch_datastreams --user=admin --pid_file=/tmp/imagepids.txt --dsid=MODS --datastreams_directory=/tmp/imagemods`
* `drush islandora_datastream_crud_push_datastreams --user=admin --datastreams_source_directory=/tmp/imagemods_modified --datastreams_crud_log=/tmp/crud.log`
* `drush islandora_datastream_crud_delete_datastreams --user=admin --dsid=FOO --pid_file=/tmp/delete_foo_from_these_objects.txt`
* `drush islandora_datastream_crud_generate_derivatives --user=admin --source_dsid=OBJ --pid_file=/tmp/regenerate_derivatives_for_these_objects.txt`

## Fetching PIDs

The `islandora_datastream_crud_fetch_pids` command provides several options for specifying which objects you want datastreams from:

* `--namespace`: Lets you specify a namespace.
* `--collection`: Lets you specify a collection PID.
* `--content_model`: Lets you specify a content model PID.
* `--with_dsid`: Lets you specify the ID of a datastream that objects must have.
* `--without_dsid`: Lets you specify the ID of a datastream that objects must not have.
* `--solr_query`: A raw Solr query. For example, `--solr_query=*:*` will retrieve all the PIDs in your repository. `--solr_query=dc.title:foo` will retrieve all the PIDs of objects that have the string 'foo' in their DC title fields.

The `--collection`, `--content_model`, `--namespace`, `--with_dsid`, `--without_dsid`, and `--solr_query` options, if present, are ANDed together, so you can, for example, retrieve PIDs of objects that have a specific namespace within a collection. If the `--solr_query` option is used, it overrides `--content_model'`, `--namespace`, `--with_dsid`, `--without_dsid`, and `--collection`.

You typically save the fetched PIDs to a PID file, whose path is specified using the `--pid_file` option. See 'The PID file' section below for more information.

## General workflow

The general workflow when using this module is:

1. Fetch some PIDs from Islandora.
2. Fetch a specific datastream from each of the objects identified by your PIDs (this saves the datastream content in a set of files, one per datasteram).
3. Update or modify the fetched datastream files.
4. Ensure that the modified datastream files are what you want to push to your repository. This module provides a way to roll back or revert changes made as a result of issuing `islandora_datastream_crud_push_datastreams, but that should not prevent you from performing quality control before you push`.
5. Push the updated datasteam files back to the objects they belong to.

Steps 1, 2, and 5 involve running the appropriate drush command. Steps 3 and 4 deserve special explantion.

### Step 3

This module doesn't do anything in Step 3 (update or modify the fetched datastream files). That's up to you. For example, if you fetched all of the TIFF OBJ datastreams from a collection, you could correct the color in the TIFFs using Photoshop, and then push them to your repository. Two sample scripts that modify datastream files are included in the `scripts` directory:

* a PHP script that appends an element/fragment to a set of XML files
* a shell script that adds a label/watermark to a set of image files

You may not find a use for these two scripts, but they illustrate the kinds of things you may want to do to datastream files.

### Step 4

This is a real step. Skip it at your own peril, doom, and ruin.

* It is extremely important that you are sure the datastream files you have modified or prepared are ready to push to your repository.
* You should perform the same types of QA and checking on these files that you perform prior to doing a batch ingest (validate the MODS, etc.).
* It would be prudent to push a small number of test datastream files to your repository before pusing the entire set, and to push the datastream files in small subsets and perform QA on the modified datastreams in your repository before pushing more.

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

### Exporting datastreams

If you want to export a set of datastreams from our repository, the `islandora_datastream_crud_fetch_datastreams` command provides a simple way to do so.

### Triggering derivative generation

This module offers a command, `islandora_datastream_crud_generate_derivatives`, that will generate all the derivatives from the specified source datastream ID. For example, to generate or regenerate all the derivatives based on the OBJ datastream, you would issue the following command:

`drush islandora_datastream_crud_generate_derivatives --user=admin --source_dsid=OBJ --pid_file=/tmp/regenerate_derivatives_for_these_objects.txt`

Note that this command does not download datastream files from your repository; it generates the derivatives directly from the datastream identified by the `--source_dsid` option.

You can also trigger derivative generation/regeneration on objects if you push OBJ datastreams up. A plausible scenario where you may want to do this is if a batch ingest fails during the derivative generation phase. By fetching a list of PIDs using the `--without_dsid` option with the ID of a derivative datastream, you can then fetch those objects' OBJ datastreams and push them back up. Not the most efficient way to trigger datastream generation. You should use this option if you want to replace the source datastream; use `islandora_datastream_crud_generate_derivatives` if just want to regenerate derivatives from an existing source datastream. 


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

## Effects of pushing datastreams

Islandora reacts to the replacement of a datastream, the deletion of a datastream, and the addition of a new datastream in several ways:

* Datastreams are versioned in Islandora. Pushing datastreams results in the pushed file content becoming the latest version of the specified datastream. It is possible to revert to a previous version of a datastream using the 'revert' option within an object's datastream management tab, but this module does *not* provide a simple way to roll back or revert changes made as a result of issuing `islandora_datastream_crud_push_datastreams`. To be clear: If you push 10,000 MODS datastreams to your repository using this module and you discover that each one contains a small problem, you'll need to revert those 10,000 datastreams, either manually or using the method described above. Or, push a new set of MODS XML datastream files that do not have the same problem.
* Replacing MODS and other datastreams indexed in Solr triggers a reindexing of that object. Deleting datastreams that are indexed in Solr updates the index to remove the indexed content of the deleted datastreams.
* Replacing the OBJ datastream, or any other datastream from which other datastreams are derived, triggers derivative regeneration as defined by solution packs and other modules. If you do not want derivatives generated as a result of pushing datastreams to your repository, enable "Defer derivative generation during ingest" option in your site's Islandora > Configuration menu. If you enable this option, don't forget to disable after your have pushed your datastreams.
* `islandora_datastream_crud_push_datastreams` does not change the MIME type of the datastream unless the `--datastreams_mimetype` is present.
* `hook_islandora_datastream_modified()`, `hook_islandora_datastream_ingested()`, `hook_islandora_datastream_purged()`, and their content-model-specific variations are fired when datastreams are replaced, created, and deleted. The effects of this depend on what modules are enabled on your Islandora site.

In general, the behaviors described here are the same regardless of whether the datastream is replaced using the "Replace" link provided in each object's Manage > Datastreams tab (or using the "+ Add a datastream" link), or with this module. However, Islandora Datastream CRUD lets you replace the same datastream across a lot of objects at once, which amplifies the load on your Islandora stack compared to replacing a datastream on a single object.

# Maintainer

* [Mark Jordan](https://github.com/mjordan)

## Development and feedback

Pull requests are welcome, as are use cases and suggestions. Scripts that do the work of updating datastreams, especially for MODS datastreams, are also welcome.

## Wishlist

* A graphical user interface
* Use the Drupal Batch framework, like the Islandora Batch modules do
* Add automated tests
* The `--collection` option for `islandora_datastream_crud_fetch_pids` only retrieves immediate children of the specified collection. If this is a problem for you, and all of the objects in your collection use the same namespace, use the `--namespace` option to get your PIDs instead of the `--collection` option.
* Does not work with datastreams in the (R)edirect and (E)xternal Referenced control groups - use cases are welcome.

## License

* [GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt) (please review sections 15, 16, and 17 carefully before installing this module).
