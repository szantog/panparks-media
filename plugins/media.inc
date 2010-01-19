<?php

/**
 * Implementation of WYSIWYG's hook_INCLUDE_plugin().
 */
function media_media_plugin() {
  include_once drupal_get_path('module', 'media') . '/media.browser.inc';
  media_include_browser_js();

  // Plugin definition
  $plugins['media'] = array(
    'title' => t('Media Browser'),
    'vendor url' => 'http://drupal.org/project/media',
    'icon path' => drupal_get_path('module','media').'/images/',
    'icon file' => 'wysiwyg-media.gif',
    'icon title' => t('Add media'),
    'js path' => drupal_get_path('module','media').'/javascript/',
    'js file' => 'wysiwyg-media.js',
    'settings' => array(),
   );

   return $plugins;
}
