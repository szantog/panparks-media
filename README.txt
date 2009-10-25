// $Id$

/**
 *  @file
 *  README for the Media Module.
 */

The Media Module exposes an architecture and API that allows a unified approach
to the storage, display, and editing of files, external resources, and other
media on a site. In particular, it extends the core File module, allowing
editors to browse from all local and remote media when attaching files to an
instance.

To install the Media Module, please follow the instructions in INSTALL.txt.

By default, the Media Module will work automatically with any File Fields
attached to nodes or other object types. You may override specific fields if
you desire, by visiting the Content Type Structure administration page, and
opening the 'Media settings' tab. Here you will see a check box allowing the
specific override for that content type.

On the field settings management page, you may additionally attach any enabled
Media Streams to a File Field. For instance, the Public and/or Private streams
are automatically enabled by default (but this may be disabled if desired),
and streams provided by other Media modules, such as for Media: YouTube or
Media: Flickr may be attached. Media or files of any provided type may be
so attached.

Enabled File fields will now have a file browser available for editors to add
new media and other files to content. In addition to uploading new files to
the server, editors will be able to browse files already uploaded, and also
files stored remotely, such as from a third party media provider, like YouTube
or Flickr.
