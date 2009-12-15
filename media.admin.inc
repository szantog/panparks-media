<?php
// $Id$

/**
 * @file
 * This file contains the admin functions for the Media module.
 */

/**
 * Provide per content type settings.
 *
 * @param array &form
 *  A form structured array.
 * @param string $type_name
 *  A string representing the content type.
 * @return
 *  Nothing is returned.
 */
function media_settings_content_type(&$form, $type_name = NULL) {
  $form['media'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('Media settings'),
    '#collapsible' => TRUE,
    '#group'       => 'additional_settings',
  );

  $type = node_type_get_type($type_name);

  // Master settings override.
  $form['media']['media_' . $type_name . '_override'] = array(
    '#title'         => t($type->name . ' overrides default values'),
    '#type'          => 'checkbox',
    '#description'   => t('Override the default settings for this content type.  The options below will only be used if this box is checked.'),
    '#default_value' => variable_get('media_' . $type_name . '_override', NULL),
  );
  $form['media']['media_' . $type_name . '_enabled'] = array(
    '#title'         => t('Enable Media resource browser'),
    '#type'          => 'checkbox',
    '#description'   => t('Enable the Media resource browser for this node type.'),
    '#default_value' => variable_get('media_' . $type_name . '_enabled', NULL),
  );

  // Extract the fields for this node type
  /* Commented out because _media_content_field_types is missing. This should all be removed?
  if (isset($type->fields)) {
    foreach ((array)$type->fields as $field_name => $field) {
      // Create the field identifier
      $form['media'][$field['field_name']] = array(
        '#type'        => 'fieldset',
        '#title'       => t('Field name: !name', array('!name' => $field['widget']['label'])),
        '#collapsible' => 'true',
      );

      // Build a form for each type of module that we have
      foreach (media_registration_kinds() as $kind) {
        // Get all the kinds that match this field
        if ($registrations = media_get_fields($field['type'], $kind)) {
          $options = array();

          foreach ($registrations as $id => $registration) {
            $options[$field['field_name'] . '--' . $id] = $registration['name'] . ': ' . $registration['description'];
          }

          $form['media'][$field['field_name']]['media_' . $type_name . '_' . $field['field_name'] . '_' . $kind] = array(
            '#title'         => t('Enable !kind options for this field', array('!kind' => $kind)),
            '#description'   => t('Choose which !kind options you would like to have enabled on this field', array('!kind' => $kind)),
            '#type'          => 'checkboxes',
            '#options'       => $options,
            '#default_value' => variable_get('media_' . $type_name . '_' . $field['field_name'] . '_' . $kind, array()),
          );
        }
      }
    }

    // if we didn't get any additional data, remove this field form
    // this is ugly but hey, sue me
    if (count($form['media'][$field['field_name']]) == 3) {
      unset($form['media'][$field['field_name']]);
    }
  }
  */
}

/**
 * Provide global settings.
 *
 * Global settings are currently just defaults for content types.
 *
 * @param $form
 *  A form structure.
 * @return
 *  A form structured array.
 */
function media_settings_global($form) {
  $form = array();

  $form['media_global_enabled'] = array(
    '#title' => t('Media resource browser'),
    '#type' => 'checkbox',
    '#description' => t('Enable or Disable the Media resource browser for all types, unless specifically set for a given type.'),
    '#default_value' => variable_get('media_global_enabled', 1),
  );

  $form['global'] = array(
    '#title' => t('Global Settings'),
    '#type' => 'fieldset',
    '#description' => t('These global settings apply when uploading files at e.g. admin/content/media and will be overridden by specific fields.'),
  );

  // Make the extension list a little more human-friendly by comma-separation.
  $extensions = str_replace(' ', ', ', variable_get('media_file_extensions', ''));
  $form['global']['media_file_extensions'] = array(
    '#type' => 'textfield',
    '#title' => t('Allowed file extensions'),
    '#default_value' => $extensions,
    '#description' => t('Separate extensions with a space or comma and do not include the leading dot. Leaving this blank will allow users to upload a file with any extension.'),
    '#element_validate' => array('_file_generic_settings_extensions'),
  );

  $form['global']['media_max_filesize'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum upload size'),
    '#default_value' => variable_get('media_max_filesize', ''),
    '#description' => t('Enter a value like "512" (bytes), "80 KB" (kilobytes) or "50 MB" (megabytes) in order to restrict the allowed file size. If left empty the file sizes will be limited only by PHP\'s maximum post and file upload sizes (current limit <strong>%limit</strong>).', array('%limit' => format_size(file_upload_max_size()))),
    '#element_validate' => array('_file_generic_settings_max_filesize'),
    '#size' => 10,
  );

  return system_settings_form($form);
}

/**
 * Display the list or thumbnails media admin display.
 */
function media_admin($form, $form_state, $display = NULL) {
  $path = drupal_get_path('module', 'media');

  if (!$display) {
    drupal_goto('admin/content/media/list');
  }

  // Load our CSS and JS.
  drupal_add_css($path . '/media.css');
  drupal_add_js($path . '/javascript/media-admin.js');

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
      // $form['#theme] = 'media_admin_thumbnails';
      break;
  }
  return $form;
}

/**
 * Form builder: Builds the media list administration overview.
 */
function media_admin_list() {
  $path = drupal_get_path('module', 'media');

  // Build the sortable table header.
  $header = array(
   'title' => array('data' => t('Title'), 'field' => 'f.filename'),
   'type' => array('data' => t('Type'), 'field' => 'f.filemime'),
   'size' => array('data' => t('Size'), 'field' => 'f.filesize'),
   'author' => array('data' => t('Author'), 'field' => 'u.name'),
   'timestamp' => array('data' => t('Updated'), 'field' => 'f.timestamp', 'sort' => 'desc'),
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
     'title' => theme('file_link', array('file' => $file)),
     'type' =>  check_plain($file->filemime),
     'size' => t('@size KB', array('@size' => number_format($file->filesize / 1000, 0))),
     'author' => theme('username', array('account' => $file)),
     'timestamp' => format_date($file->timestamp, 'short'),
   );
   $options[$file->fid]['operations'] = l(t('edit'), 'admin/content/media/' . $file->fid . '/edit', array('query' => $destination));
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
  $path = drupal_get_path('module', 'media');

  $query = db_select('file', 'f')->extend('PagerDefault');
  $query->join('users', 'u', 'f.uid = u.uid');

  $result = $query
   ->fields('f')
   ->fields('u', array('name'))
   ->limit(50)
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
    // Set a seperate icon for the active item.
    if ($delta == $active_display) {
      $icon = $item['icon_active'];
    }
    else {
      $icon = $item['icon'];
    }

    $display_items[] = array(
      'data' => l(theme('image', array('path' => $icon, 'alt' => $item['title'])),
        $form_location . '/' . $delta,
        array(
          'html' => TRUE,
          'attributes' => array('title' => $item['description']),
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
 * Menu callback; presents the Media editing form, or redirects to delete confirmation.
 */
function media_page_edit($file) {
  drupal_set_title(t('Edit @title', array('@title' => drupal_set_title($file->filename))));
  return drupal_get_form('media_edit', $file);
}

/**
 * Form builder: Builds the edit file form.
 */
function media_edit($form, $form_state, $file) {

  drupal_set_title($file->filename);

  field_attach_form('media', $file, $form, $form_state);

  $form['preview'] = array(
    '#type' => 'markup',
    '#markup' => 'PREVIEW GOES HERE',
    '#prefix' => '<div style="padding:10px; border:3px solid #CCC; width:200px; height:200px">',
    '#suffix' => '</div>',
    '#weight' => -10,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );



  $form['fid'] = array(
    '#type' => 'hidden',
    '#value' => $file->fid,
  );

  $form['#redirect'] = 'admin/content/media';
  return $form;
}

function media_edit_validate($form, &$form_state) {
  $media = media_load($form_state['values']['fid']);
  field_attach_form_validate('media', $media, $form, $form_state);
}

/**
 * Process media_edit form submissions.
 */
function media_edit_submit($form, &$form_state) {
  $media = media_load($form_state['values']['fid']);
  field_attach_submit('media', $media, $form, $form_state);
  media_save($media);
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

  if ($file = file_move($file, $destination, FILE_EXISTS_REPLACE)) {
    file_save($file);
    drupal_set_message(t('The file @name was uploaded', array('@name' => $file->filename)));
    $form_state['redirect'] = url('admin/content/media/' . $file->fid . '/edit');
    field_attach_insert('media', $file);
  }
  else {
    drupal_set_message(t('An error occurred and no file was uploaded.'), 'error');
  }
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
    $row[] = array('data' => l(t('edit'), 'admin/structure/media/manage/' . $type_url_str));

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

function media_admin_type_manage_form($form, &$form_state, $media_type) {
  $form = array();
  $form['media_type'] = array(
    '#type' => 'fieldset',
    '#title' => t('@media media type', array('@media' => $media_type->label)),
  );
  return $form;
}
