<?php
// $Id$

/**
 * @file
 * This file contains the admin functions for the Media module.
 */

/**
 * Display the list or thumbnails media admin display.
 */
function media_admin($form, $form_state) {
  global $user;
  if (arg(3)) {
    $display = arg(3);
    if ($display != 'list' && $display != 'thumbnails') {
      exit(drupal_not_found());
    }
    // Save their preference.
    db_merge('media_list_type')
      ->key(array('uid' => $user->uid))
      ->fields(array(
          'type' => $display,
      ))
      ->execute();
  }
  else {
    $display = db_query("SELECT type FROM {media_list_type} WHERE uid = :uid", array(':uid' => $user->uid))->fetch();
    if (!$display) {
      $display = 'list';
    }
    else {
      $display = $display->type;
    }
  }

  $form['#attached'] = media_attached_files();
  $form['#attached']['js'][] = array(
    'data' => array('mediaAddFileForm' => drupal_render(drupal_get_form('media_add_files'))),
    'type' => 'setting',
  );

  if (isset($form_state['values']['operation']) && $form_state['values']['operation'] == 'delete') {
    return media_multiple_delete_confirm($form, $form_state, array_filter($form_state['values']['files']));
  }

  // Build the 'Update options' form.
  $form[] = media_admin_operations();

  switch ($display) {
    case 'list':
      $form['admin'] = media_admin_list();
      break;
    case 'thumbnails':
      $form['admin'] = media_admin_thumbnails();
      break;
  }
  return $form;
}

/**
 * Form builder: Builds the media list administration overview.
 */
function media_admin_list() {
  // Build the sortable table header.
  $header = array(
   'title' => array('data' => t('Title'), 'field' => 'f.filename'),
   'type' => array('data' => t('Type'), 'field' => 'f.filemime'),
   'size' => array('data' => t('Size'), 'field' => 'f.filesize'),
   'author' => array('data' => t('Author'), 'field' => 'u.name'),
   'timestamp' => array('data' => t('Updated'), 'field' => 'f.timestamp', 'sort' => 'asc'),
   'operations' => array('data' => t('Operations')),
  );

  $query = db_select('file', 'f')->extend('PagerDefault')->extend('TableSort');
  $query->join('users', 'u', 'f.uid = u.uid');

  $result = $query
   ->fields('f')
   ->fields('u', array('name'))
   ->limit(50)
   ->orderByHeader($header)
   ->execute();

  $destination = drupal_get_destination();
  $files = array();
  $options = array();

  foreach ($result as $file) {
   $options[$file->fid] = array(
     'title' => theme('media_link', array('file' => $file)),
     'type' =>  check_plain($file->filemime),
     'size' => t('@size KB', array('@size' => number_format($file->filesize / 1000, 0))),
     'author' => theme('username', array('account' => $file)),
     'timestamp' => format_date($file->timestamp, 'short'),
   );
   $options[$file->fid]['operations'] = l(t('edit'), 'media/' . $file->fid . '/edit', array('query' => $destination));
  }
  $form['files'] = array(
   '#type' => 'tableselect',
   '#header' => $header,
   '#options' => $options,
   '#empty' => t('No media available.'),
  );
  $form['pager'] = array('#markup' => theme('pager', array('tags' => NULL)));

  // Build the display switch.
  $form = media_admin_display_switch($form, array('active_display' => 'list'));

  return $form;
}

/**
 * Form builder: Builds the media thumbnails administration overview.
 */
function media_admin_thumbnails() {


  $query = db_select('file', 'f')->extend('PagerDefault');
  $query->join('users', 'u', 'f.uid = u.uid');

  $result = $query
   ->fields('f')
   ->fields('u', array('name'))
   ->limit(50)
   ->orderBy('f.timestamp', 'desc')
   ->execute();

  $destination = drupal_get_destination();
  $rows = array();
  $options = array();
  $form['files'] = array(
   '#tree' => TRUE,
   '#prefix' => '<div class ="media-display-thumbnails clearfix">',
   '#suffix' => '</div>',
  );
  if (count($result)) {
    foreach ($result as $file) {
     $form['files'][$file->fid] = array(
       '#type' => 'checkbox',
       '#title' => check_plain($file->filename),
       '#prefix' => '<div class="media-thumbnail">' . theme('media_admin_thumbnail', array('file' => $file)),
       '#suffix' => theme('media_admin_thumbnail_operations', array('file' => $file)) . '</div>',
     );
    }
  }

  $form['pager'] = array('#markup' => theme('pager', array('tags' => NULL)));

  // Build the display switch.
  $form = media_admin_display_switch($form, array('active_display' => 'thumbnails'));

  return $form;
}

/**
 * Build the operations portion of the file listing forms.
 */
function media_admin_operations() {

  // Build the 'Update options' form.
  $form['options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Update options'),
    '#prefix' => '<div class="container-inline">',
    '#suffix' => '</div>',
  );
  $options = array();
  foreach (module_invoke_all('media_operations') as $operation => $array) {
    $options[$operation] = $array['label'];
  }
  $form['options']['operation'] = array(
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => 'delete',
  );
  $options = array();
  $form['options']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
    '#submit' => array('media_admin_submit'),
    '#validate' => array('media_admin_validate'),
  );
  return $form;
}

/**
 * Build the display switch portion of the file listings form.
 */
function media_admin_display_switch($form = array(), $options = array()) {

  $path = drupal_get_path('module', 'media');

  $form_location = (isset($options['form_location'])) ? $options['form_location'] : 'admin/content/media';
  $active_display = (isset($options['active_display'])) ? $options['active_display'] : NULL;

  // Set up the default display types.
  $display_types = array();
  $display_types['list'] = array(
    'title' => t('List'),
    'description' => t('Display as a list.'),
    'icon' => $path . '/images/display-list.png',
    'icon_active' => $path . '/images/display-list-active.png',
  );
  $display_types['thumbnails'] = array(
    'title' => t('Thumbnails'),
    'description' => t('Display as thumbnails.'),
    'icon' => $path . '/images/display-thumbnails.png',
    'icon_active' => $path . '/images/display-thumbnails-active.png',
  );
  // Make it alterable so other modules can add additional displays.
  drupal_alter('media_display_types', $display_types);

  // Build the item list.
  $display_items = array();
  foreach ($display_types as $delta => $item) {
    $attributes = array('title' => $item['description']);
    // Set a seperate icon for the active item.
    if ($delta == $active_display) {
      $icon = $item['icon_active'];
      $attributes['class'][] = 'active';
    }
    else {
      $icon = $item['icon'];
    }

    $display_items[] = array(
      'data' => l(theme('image', array('path' => $icon, 'alt' => $item['title'])),
        $form_location . '/' . $delta,
        array(
          'html' => TRUE,
          'attributes' => $attributes,
        )),
    );
  }

  $form['display_type'] = array(
    '#type' => 'markup',
    '#markup' => theme('item_list', array(
       'items' => $display_items,
       'attributes' => array('class' => 'media-display-switch'),
      )
    ),
  );

  return $form;
}

/**
 * Validate media_admin_list form submissions.
 *
 * Check if any files have been selected to perform the chosen
 * 'Update option' on.
 */
function media_admin_validate($form, &$form_state) {
  $files = array_filter($form_state['values']['files']);
  if (count($files) == 0) {
    form_set_error('', t('No items selected.'));
  }
}

/**
 * Process media_admin_list form submissions.
 *
 * Execute the chosen 'Update option' on the selected files.
 */
function media_admin_submit($form, &$form_state) {
  $operations = module_invoke_all('media_operations');
  $operation = $operations[$form_state['values']['operation']];
  // Filter out unchecked nodes
  $files = array_filter($form_state['values']['files']);
  if ($function = $operation['callback']) {
    // Add in callback arguments if present.
    if (isset($operation['callback arguments'])) {
      $args = array_merge(array($files), $operation['callback arguments']);
    }
    else {
      $args = array($files);
    }
    call_user_func_array($function, $args);

    cache_clear_all();
  }
  else {
    // We need to rebuild the form to go to a second step. For example, to
    // show the confirmation form for the deletion of nodes.
    $form_state['rebuild'] = TRUE;
  }
}

/**
 * Confirm the request to delete files.
 */
function media_multiple_delete_confirm($form, &$form_state, $files) {
  $form['files'] = array('#prefix' => '<ul>', '#suffix' => '</ul>', '#tree' => TRUE);
  // array_filter returns only elements with TRUE values
  foreach ($files as $fid => $value) {
    $title = db_query('SELECT filename FROM {file} WHERE fid = :fid', array(':fid' => $fid))->fetchField();
    $form['files'][$fid] = array(
      '#type' => 'hidden',
      '#value' => $fid,
      '#prefix' => '<li>',
      '#suffix' => check_plain($title) . "</li>\n",
    );
  }
  $form['operation'] = array('#type' => 'hidden', '#value' => 'delete');
  $form['#submit'][] = 'media_multiple_delete_confirm_submit';

  $confirm_question = format_plural(count($files),
                                  'Are you sure you want to delete this item?',
                                  'Are you sure you want to delete these items?');

  return confirm_form($form,
                    $confirm_question,
                    $_GET['q'], t('This action cannot be undone.'),
                    t('Delete'), t('Cancel'));
}

/**
 * Attempt to delete files and notify the user of the result.
 */
function media_multiple_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $results = array();
    $files = array_keys($form_state['values']['files']);
    foreach ($files as $fid) {
      $file = file_load($fid);
      $files[$fid] = $file;
      $results[$fid] = file_delete($file);
    }
    // The result of file_delete can be an array if the file is in use, or TRUE/FALSE.
    foreach ($results as $fid => $result) {
      if (is_array($result)) {
        drupal_set_message(t('The file @title is in use and cannot be deleted.', array('@title' => $files[$fid]->filename)), 'warning');
      }
      elseif (!$result) {
        drupal_set_message(t('The file @title was not deleted due to an error.', array('@title' => $files[$fid]->filename)), 'error');
      }
      else {
        $message = t('File @title was deleted', array('@title' => $files[$fid]->filename));
        watchdog('media', $message);
        drupal_set_message($message);
      }
    }
  }
}


/**
 * Implement hook_media_operations().
 * @todo: shouldn't this be using actions or something?
 */
function media_media_operations() {
  $operations = array(
    'delete' => array(
      'label' => t('Delete'),
      'callback' => NULL,
    ),
  );
  return $operations;
}

/**
 * Display the list or thumbnails media admin display.
 */
function media_add_files($form, $form_state) {
  $validators = array(
   'file_validate_extensions' => array(variable_get('media_file_extensions', '')),
   'file_validate_size' => array(parse_size(variable_get('media_max_filesize', ''))),
  );


  // A blank set of allowed file extensions means no need to validate.
  if (!$validators['file_validate_extensions'][0]) {
    unset($validators['file_validate_extensions']);
  }
  // Use the PHP limit for filesize if no variable was set.
  if (!$validators['file_validate_size']) {
    $validators['file_validate_size'] = file_upload_max_size();
  }

  if ($validators['file_validate_size'][0] == 0) {
    unset($validators['file_validate_size']);
  }

  $form['#validators'] = $validators;

  $form['upload'] = array(
    '#type' => 'file',
    '#title' => t('Upload a new file'),
    '#description' => theme('file_upload_help', array('description' => '', 'upload_validators' => $validators)),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}

/**
 * Validate the generic file upload with the global media settings.
 */
function media_add_files_validate($form, &$form_state) {

  // Save the file as a temporary file.
  $file = file_save_upload('upload', $form['#validators']);
  if ($file === FALSE) {
    form_set_error('upload', t("Failed to upload the file."));
  }
  elseif ($file !== NULL) {
    $form_state['values']['upload'] = $file;
  }
}

/**
 * Upload a file.
 */
function media_add_files_submit($form, &$form_state) {
  $scheme = variable_get('file_default_scheme', 'public') . '://';
  $file = $form_state['values']['upload'];
  $destination = file_stream_wrapper_uri_normalize($scheme . $file->filename);

  $defaults = array (
    'display' => TRUE,
  );

  if ($file = file_move($file, $destination, FILE_EXISTS_REPLACE)) {
    drupal_set_message(t('The file @name was uploaded', array('@name' => $file->filename)));
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
    drupal_set_message(t('An error occurred and no file was uploaded.'), 'error');
  }
  $form_state['redirect'] = 'admin/content/media';
}

/**
 * Displays the content type admin overview page.
 */
function media_admin_type_list() {
  $types = media_type_get_types();
  $field_ui = module_exists('field_ui');
  $header = array(t('Name'), array('data' => t('Operations'), 'colspan' => $field_ui ? '4' : '2'));
  $rows = array();

  foreach ($types as $key => $info) {
    $type = $types[$key];
    $type_url_str = str_replace('_', '-', $key);
    $row = array();
    $label = $info->label;

    // Set the name column.
    $row[] = array('data' => t($info->label));

    // Set the edit column.
    $row[] = array('data' => l(t('settings'), 'admin/structure/media/manage/' . $type_url_str));

    if ($field_ui) {
      // Manage fields.
      $row[] = array('data' => l(t('manage fields'), 'admin/structure/media/manage/' . $type_url_str . '/fields'));

      // Display fields.
      $row[] = array('data' => l(t('manage display'), 'admin/structure/media/manage/' . $type_url_str . '/display'));
    }

    // Set the delete column.
    if (empty($type->base)) {
      $row[] = array('data' => l(t('delete'), 'admin/structure/media/manage/' . $type_url_str . '/delete'));
    }
    else {
      $row[] = array('data' => '');
    }

    $rows[] = $row;

  }

  $build['media_type_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows
  );

  return $build;
}

/**
 *  The administration form for managing media types.
 */
function media_admin_type_manage_form($form, &$form_state, $media_type) {
  $form = array();
  $form['media_type'] = array(
    '#type' => 'value',
    '#value' => $media_type->machine_name,
  );
  $form['type'] = array(
    '#type' => 'fieldset',
    '#title' => t('@media media type', array('@media' => $media_type->label)),
  );

  // If this Media type is handled by us, then we can put in some default
  // options. Otherwise, we leave it to the implementing module to form_alter.
  if ($media_type->type_callback == 'media_is_type') {
    // Options for match_type.
    $options = array(
      'all' => t('All'),
      'any' => t('Any'),
      '0' => t('Other'),
    );
    if ($media_type->type_callback_args['match_type'] && isset($options[$media_type->type_callback_args['match_type']])) {
      $default_value = $media_type->type_callback_args['match_type'];
      $other_default_value = '';
    }
    else {
      $default_value = 0;
      $other_default_value = $media_type->type_callback_args['match_type'];
    }
    $form['type']['match_type'] = array(
      '#type' => 'radios',
      '#title' => t('Match type'),
      '#options' => $options,
      '#default_value' => $default_value,
    );
    $form['type']['match_type_other'] = array(
      '#type' => 'textfield',
      '#title' => t('Other match type value'),
      '#default_value' => $other_default_value,
      '#attached' => array(
        'js' => array(drupal_get_path('module', 'media') . '/javascript/media-types-admin.js'),
      ),
    );

    // Options for allowed Streams.
    $options = array('public' => t('Public files'), 'private' => t('Private files'));
    foreach(file_get_stream_wrappers() as $stream => $wrapper) {
      $options[$stream] = $wrapper['name'];
    }
    unset($options['temporary']);
    $default_value = array();
    if (isset($media_type->type_callback_args['streams'])) {
      foreach ($media_type->type_callback_args['streams'] as $stream) {
        $default_value[$stream] = $stream;
      }
    }
    $form['type']['streams'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Allowed streams'),
      '#options' => $options,
      '#default_value' => $default_value,
    );

    // Options for allowed mimetypes & extensions.
    $default_value = isset($media_type->type_callback_args['mimetypes']) ? implode(' ', $media_type->type_callback_args['mimetypes']) : '';
    $form['type']['mimetypes'] = array(
      '#type' => 'textfield',
      '#title' => t('Allowed mimetypes'),
      '#description' => t('You may optionally enter one or more allowed file mimetypes for this Media type, if appropriate, separating each with a space. You may use a regular expression for matching, such as %image_match (which would match any mimetype beginning with %image) or %any_match, which would match any file mimetype.', array('%image_match' => '/^image/', '%image' => t('image'), '%any_match' => '/.*/')),
      '#default_value' => check_plain($default_value),
    );
    $default_value = isset($media_type->type_callback_args['extensions']) ? implode(' ', $media_type->type_callback_args['extensions']) : '';
    $form['type']['extensions'] = array(
      '#type' => 'textfield',
      '#title' => t('Allowed extensions'),
      '#description' => t('You may optionally enter one or more allowed file extensions for this Media type, if appropriate, separating each with a space (and no dots).'),
      '#default_value' => check_plain($default_value),
    );
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 100,
  );
  return $form;
}

function media_admin_type_manage_form_submit($form, &$form_state) {
  $media_type = media_type_load($form_state['values']['media_type']);
  if ($form_state['values']['match_type']) {
    $media_type->type_callback_args['match_type'] = $form_state['values']['match_type'];
  }
  else {
    $media_type->type_callback_args['match_type'] = $form_state['values']['match_type_other'];
  }
  $media_type->type_callback_args['streams'] = array();
  foreach ($form_state['values']['streams'] as $stream) {
    if ($stream) {
      $media_type->type_callback_args['streams'][] = $stream;
    }
  }
  $media_type->type_callback_args['mimetypes'] = explode(' ', $form_state['values']['mimetypes']);
  $media_type->type_callback_args['extensions'] = explode(' ', $form_state['values']['extensions']);

  media_type_save($media_type);
  drupal_set_message(t('The @label media type has been saved.', array('@label' => $media_type->label)));
}

/* *********************/
/*   BROWSER RELATED
/* *********************/


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


