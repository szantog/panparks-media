*******************************************************************************

                         D R U P A L  M O D U L E

*******************************************************************************

Name: 		Media module
Authors: 	Robert Douglass -- Rob *AT* robshouse /dot/ net
		      Alan Evans -- alanevans *AT* tiscali /dot/ de
Last update:	19th February 2005
Drupal:		4.5.2, 4.6.0

Sponsored by Webs4.com

*******************************************************************************

Description
-----------

The media module recognises certain types of audio and video files (types set by the administrator) and analyses any metadata present in those files.  This information is stored in a metadata database table, and can be viewed, allowing users with the right permissions to either stream or download audio or video files.  The module also allows the creation of a new type of node - the playlist - from those files, which can then be played by clicking a play button, assuming the user has a media player installed.

Requirements
------------

Tested on Drupal versions 4.5.2 and 4.6.0

Tested using MySQL database only.

This module requires the GetID3 library pre version 2 (current version is at the time of writing 1.7.2), which is supplied in the media folder.  Updates to this library are available from sourceforge.net  The media module is not designed for operation with getid3 version 2 (currently beta)


Installation
------------

See INSTALL.txt

Getting Started
---------------

After following the installation instructions, the media file types that you specified in the admin section will be recognised automatically when you attach them to any node type that accepts attachments.  You may need to alter site settings under "admin > settings" concerning file upload/download, and "administer > uploads" to set the maximum upload size.

Playlists can be created by clicking on the multi-media (or the name that you may have chosen for this item) link in the menu and clicking the "create playlist" subtab.  Items are added to the paylist from the table by clicking "2pl" under "actions". 

Alternatively, you can create a playlist by clicking "create content".  Give the playlist a title to begin with and submit it.  On the confirmation page, there is, in addition to the normal edit and view tabs also a "add/change content" tab.  Clicking this tab takes you to the same table where you may select items to add to the playlist.  Your changes are only added to your playlist when "save" is clicked.

Notes
-----

TODO
----

In preparation

Planned Features
----------------

In preparation


