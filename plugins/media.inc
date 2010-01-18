<?php 

/**
 * Implementation of hook_INCLUDE_plugin()
 */

function media_media_plugin() {
  // Not elegant to include js and css files here, but till we figure out
  // elegant way, it atleast assures that they are included only when the
  // plugin is included and not on all form_alter, page_alter, hook_init.
  drupal_add_js(drupal_get_path('module','media').'/javascript/util/json2.js');
  drupal_add_js(drupal_get_path('module','media').'/javascript/util/ba-debug.min.js');
  drupal_add_js(drupal_get_path('module','media').'/javascript/media.jquery.js');
  drupal_add_library('system','ui');
  drupal_add_library('system','ui.dialog');
  drupal_add_css(drupal_get_path('module','media').'/media-browser.css');
  // copied directly from media module to get Drupal.settings.media object
  $settings = array(
    'browserUrl' => url('media/browser',
      array('query' => array('render' => 'media-browser'))),
  // @Todo: rip this down and make a callback API structure with tokens, etc
  // this is temporary.
    'blacklist' => array('src','fid'),
  );
  drupal_add_js(array('media' => $settings), 'setting');
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