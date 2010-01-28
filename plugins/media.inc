<?php
// $Id$

/**
 *  @file
 *  Define the WYSIWYG browser plugin.
 */

/**
 * Implementation of WYSIWYG's hook_INCLUDE_plugin().
 */
function media_media_plugin() {
  // Include the required browser JS.
  module_load_include('inc', 'media', 'media.browser');
  media_include_browser_js();

  // Plugin definition
  $plugins['media'] = array(
    'title' => t(media_variable_get('wysiwyg_title')),
    'vendor url' => 'http://drupal.org/project/media',
    'icon path' => drupal_get_path('module','media').'/images/',
    'icon file' => 'wysiwyg-media.gif',
    'icon title' => t(media_variable_get('wysiwyg_icon_title')),
    'js path' => drupal_get_path('module','media').'/javascript/',
    'js file' => 'wysiwyg-media.js',
    'settings' => array(),
   );

   return $plugins;
}
