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

  $attached = array();
  $attached['library'][] = array('media', 'media_browser_page');

  $plugins = module_invoke_all('media_browser_plugins');
  foreach ($plugins as $name => $plugin) {
    // Add their JS and CSS
    if ($plugin['#attached']) {
      $attached = array_merge_recursive($attached, $plugin['#attached']);
    }
    unset($plugin['#attached']);
  }

  // Allow modules implementing hook_media_browser_callbacks() to add a set
  // of callbacks to the settings.
  // @TODO: Document the hook...
  $callbacks = module_invoke_all('media_browser_callbacks');
  drupal_alter('media_browser_callbacks_alter', $callbacks);

  // Allow modules implementing hook_media_browser_conditions to add new
  // conditions to load the library. For instance, they may only return media
  // belonging to the user, with array('uid' => $user->uid);
  $conditions = module_invoke_all('media_browser_conditions');

  // Allow modules implementing hook_media_browser_conditions_alter to alter
  // the set of browser library conditions.
  drupal_alter('media_browser_conditions_alter', $conditions);

  // Allow modules implementing hook_media_browser_conditions to filter streams
  // when loading the library. For instance, they may only return media
  // belonging to the public:// stream, with array('public://');
  $streams = module_invoke_all('media_browser_streams');

  // Allow modules implementing hook_media_browser_streams_alter to alter
  // the set of browser stream filters.
  drupal_alter('media_browser_conditions_streams', $streams);

  $settings = array(
    'viewType' => media_variable_get('browser_viewtype_default', 'thumbnails'),
    'callbacks' => $callbacks,
    'plugins' => $plugins,
    'conditions' => $conditions,
    'streams' => $streams,
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
 *
 * @see media_add_upload()
 * @see media_add_from_url()
 */
function media_media_browser_plugins() {
  $plugins = array();
  $path = drupal_get_path('module', 'media');
  include_once($path . '/media.admin.inc');

  $redirect = array('media/browser', array('query' => array('render' => 'media-popup')));

  $upload_form = drupal_get_form('media_add_upload', $redirect);
  $from_url_form = drupal_get_form('media_add_from_url', $redirect);

  $plugins['upload'] = array(
//    'tabs' => array(
//      'title' => 'Upload',
//      'url' => url('media/add/upload', array('query' => array('destination' => drupal_get_destination()))),
//    ),
    '#attached' => array(
      'js' => array($path . '/javascript/plugins/media.upload.js'),
    ),
    'settings' => array(
      'uploadForm' => drupal_render($upload_form),
    ),
  );

  $plugins['fromurl'] = array(
    '#attached' => array(
      'js' => array($path . '/javascript/plugins/media.fromurl.js'),
    ),
    'settings' => array(
      'fromUrlForm' => drupal_render($from_url_form),
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
 * Implements hook_media_browser_callbacks().
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
function media_browser_list() {
  if (isset($_GET['conditions'])) {
    $conditions = drupal_json_decode($_GET['conditions']);
  }
  $conditions = isset($conditions) ? $conditions : array();

  if (isset($_GET['streams'])) {
    $streams = drupal_json_decode($_GET['streams']);
  }

  // If we have been passed an array of streams, such as 'youtube://',
  // then only accept media from those streams. Unfortunately, entity_load
  // doesn't accept a filter of LIKE, so we need to preprocess that.
  // @TODO: This is completely unacceptable. 1) if we pass more than
  // one stream, it will do AND rather than OR right now (easy fix),
  // but 2) this will get rather large I suspect, unless we combine
  // with pagers, or use a db_transaction maybe, or just write our own query.
  if (!empty($streams)) {
    // First get the fid's to load. We have to do that first, because
    // entity_load doesn't accept a condition of LIKE.
    $fids = db_select('file', 'f')
      ->fields('f', array('fid'));

    // Filter on streams.
    foreach ($streams as $stream) {
      $fids->condition('uri', db_like($stream) . '%', 'LIKE');
    }

    // Add our conditions.
    foreach ($conditions as $field => $condition) {
      $fids->condition($field, $condition);
    }

    // Grab the fid's.
    $fids = $fids->execute()
      ->fetchCol();

    // Now load the desired media to display.
    $medias = entity_load('media', $fids);
  }
  else {
    // Load the media based on any required conditions.
    $medias = entity_load('media', FALSE, $conditions);
  }

  foreach ($medias as &$media) {
    // Generate a preview of the file
    // @todo: Should generate placeholders for audio
    // Add yet another wrapper so we can style it as a preview :(
    // Otherwise it isn't really possible to know because the user can pick anything for their preview mode.
    $preview = field_view_field('media', $media, 'file', 'media_preview');
    $preview['#show_names'] = TRUE;
    $preview['#theme_wrappers'][] = 'media_thumbnail';

    $media->preview = drupal_render($preview);
  }
  print drupal_json_encode($medias);
  die();
}

/**
 *  @TODO: Document.
 */
function media_include_browser_js() {
  drupal_add_library('media', 'media_browser');

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
