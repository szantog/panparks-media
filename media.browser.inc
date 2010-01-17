<?php

/**
 * @file
 * Media browser.
 */

function media_browser_library() {
  $rows = array();
  $path = drupal_get_path('module', 'media');

  $callbacks = module_invoke_all('media_browser_callbacks');
  drupal_alter('media_browser_callbacks_alter', $callbacks);
  $settings = array(
    'callbacks' => $callbacks,
  );

  drupal_add_js(array('mediaBrowser' => $settings), 'setting');
  // Here we will return the basic structure of the browser.
  $build['media_browser'] = array(
    '#type' => 'markup',
    '#prefix' => '<div id="media-browser-library">',
    '#suffix' => '</div>',
  );

  $build['media_browser']['center'] = array(
    '#theme' => 'table',
    '#attributes' => array('id' => 'media-browser-list'),
    '#header' => array('Preview', 'Info'),
    '#rows' => $rows,
    '#prefix' => t('Pick something!'),
  );

  $build['media_browser']['info'] = array (
    '#type' => 'markup',
    '#markup' => '<div id="media-browser-info"></div>',
  );

  return $build;
}

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

  //@todo: this should wrap inside of the parent.
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

function media_browser_include_plugin($name) {
  $path = drupal_get_path('module', 'media');
  drupal_add_js($path . '/javascript/plugins/' . $name . '.js');
}

/**
 *
 * @param unknown_type $form
 * @param unknown_type $media
 * @return unknown_type
 */
function media_browser_format_form($form, $form_state, $media) {
  $form = array();
  $options = array();
  foreach (media_field_view_modes('media') as $key => $mode) {
    //@TODO: Get the file formatter, and display that here too.
    $options[$key] = $mode['label'];
    $formats[$key] = field_view_field('media', $media, 'file', $key);
    $formats[$key]['#prefix'] = '<div id="' . $key . '"/>';
    $formats[$key]['#suffix'] = '</div>';
  }

  $form['preview'] = array(
    '#type' => 'markup',
    '#title' => basename($media->uri),
    '#markup' => drupal_render(field_view_field('media', $media, 'file', 'media_preview'))
  );

  $form['formats'] = array(
    '#type' => 'radios',
    '#title' => 'Format',
    '#options' => $options,
  );

  $form['formats_rendered'] = array(
    '#prefix' => '<div id="formats-rendered" style="display:none">',
    '#suffix' => '</div>',
  );

  $form['formats_rendered'] += $formats;

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'select',
    '#ajax' => array(
      'callback' => 'media_browser_format_select',
    ),
  );

  return $form;
}

function media_browser_format_select($form, &$form_state) {
  dpr($form_state);
  return "blah blah";
}

function media_browser_format($media) {
  print drupal_render(drupal_get_form('media_browser_format_form', $media));
  die();
}

function media_add_fromurl($form, &$form_state = array()) {
  $form['url'] = array(
    '#type' => 'textfield',
    '#title' => 'URL',
    '#description' => 'Input any url which Drupal can handle',
  );

  $form['url'] = array(
    '#type' => 'textfield',
    '#title' => 'URL',
    '#description' => 'Input any url which Drupal can handle',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Submit',
  );

  return $form;
}

function media_add_fromurl_validate($form, &$form_state) {
  // How do we validate this?
}

/**
 * Upload a file from a URL
 */
function media_add_fromurl_submit($form, &$form_state) {
  $url = $form_state['values']['url'];
  $scheme = variable_get('file_default_scheme', 'public') . '://';
  $destination = file_stream_wrapper_uri_normalize($scheme . basename($url));

  $defaults = array (
    'display' => TRUE,
  );

  copy($url, $destination);

  $file = file_uri_to_object($destination);
  file_save($file);

  if ($file) {
    $form_state['redirect'] = 'media/' . $file->fid . '/edit';
    field_attach_submit('media', $file, $form, $form_state);
    // Make a copy of the file object to use as the media object
    // (file is a field of media and the base table). This is weird.
    $media = clone($file);
    $file->file = array();
    $file->file[LANGUAGE_NONE] = array();
    $file->file[LANGUAGE_NONE][0] = (array)$file + $defaults;
    $file->is_new = TRUE;
    field_attach_insert('media', $file);
  }
  else {
    drupal_set_message(t('An error occurred and no file was saved.'), 'error');
  }
  $form_state['redirect'] = 'admin/content/media';
}


