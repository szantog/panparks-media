<?php
// $Id$

/**
 *  @file
 *  @TODO Document.
 */

/**
 *  @TODO: Document this.
 */
function media_browser() {
  $rows = array();
  $path = drupal_get_path('module', 'media');
  $attached = array(
    'js' => array(
      'misc/ui/ui.core.js',
      'misc/ui/ui.tabs.js',
      $path . '/javascript/media.core.js',
      $path . '/javascript/media.pluginbase.js',
      $path . '/javascript/media-browser.js',
      $path . '/javascript/util/json2.js',
      $path . '/javascript/util/ba-debug.min.js',
    ),
    'css' => array(
      '/misc/ui/ui.all.css',
      $path . '/media.browser.css'
    ),
  );

  $plugins = module_invoke_all('media_browser_plugins');
  foreach ($plugins as $name => $plugin) {
    // Add their JS and CSS
    if ($plugin['#attached']) {
      $attached = array_merge_recursive($attached, $plugin['#attached']);
    }

  }

  $callbacks = module_invoke_all('media_browser_callbacks');
  drupal_alter('media_browser_callbacks_alter', $callbacks);
  $settings = array(
    'callbacks' => $callbacks,
    'plugins' => $plugins,
  );

  drupal_add_js(array('media' => array('browser' => $settings)), 'setting');

//  drupal_add_js(array('mediaBrowser' => $settings), 'setting');
  // Here we will return the basic structure of the browser.
  $build['media_browser'] = array(
    '#prefix' => '<div id="media-browser">',
    '#suffix' => '</div>',
    '#attached' => $attached,
  );

  $build['media_browser']['tabs'] = array(
    '#type' => 'markup',
    '#prefix' => '<div id="media-browser-tabs">',
    '#suffix' => '</div>',
    '#markup' => '<ul></ul>',
  );

  return $build;
}

/**
 * Implement hook_media_browser_plugins
 * @return unknown_type
 */
function media_media_browser_plugins() {
  $plugins = array();
  $path = drupal_get_path('module', 'media');

  $plugins['upload'] = array(
    '#attached' => array(
      'js' => array($path . '/javascript/plugins/media.upload.js'),
    ),
  );

  $plugins['fromurl'] = array(
    '#attached' => array(
      'js' => array($path . '/javascript/plugins/media.fromurl.js'),
      ),
  );

  $plugins['library'] = array(
    '#attached' => array(
      'js' => array(
        $path . '/javascript/plugins/media.library.js',
      ),
      'css' => array(
        $path . '/javascript/plugins/media.library.css',
      ),
    ),
    'settings' => array('viewMode' => 'thumbnails'),
  );
  return $plugins;
}

/**
 *  @TODO: Document.
 */
function media_media_browser_callbacks() {
  return array (
    'getMedia' => array(
      'url' => url('media/browser/list'),
      'token' => drupal_get_token('media/browser/list'),
    ),
  );
}

/**
 * AJAX Callback for a list of media with some basic filters.
 *
 * @return unknown_type
 */
function media_browser_list($conditions = array()) {
  $medias = entity_load('media', FALSE, $conditions);

  foreach ($medias as &$media) {
    // Generate a preview of the file
    // @todo: Should generate placeholders for audio/video
    $media->preview = drupal_render(field_view_field('media', $media, 'file', 'media_preview'));
  }
  print drupal_json_encode($medias);
  die();
}

/**
 *  @TODO: Document.
 */
function media_include_browser_js() {
  drupal_add_library('media', 'media_browser');

  drupal_add_library('system','ui');
  drupal_add_library('system','ui.dialog');
  // copied directly from media module to get Drupal.settings.media object
  $settings = array(
    'browserUrl' => url('media/browser',
      array('query' => array('render' => 'media-popup'))),
    'styleSelectorUrl' => url('media/-media_id-/format-form',
      array('query' => array('render' => 'media-popup'))),
  // @Todo: rip this down and make a callback API structure with tokens, etc
  // this is temporary.
  // Adding src to blacklist; fid and view_mode we capture outside attribs so adding
  // them too to the blacklist.
    'blacklist' => array('src','fid','view_mode'),
  );

  drupal_add_js(array('media' => $settings), 'setting');
}
