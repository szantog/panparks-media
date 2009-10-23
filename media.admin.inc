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

  $type = _media_content_field_types($type_name);

  // Master settings override.
  $form['media']['media_'. $type_name .'_override'] = array(
    '#title'         => t($type->name .' overrides default values'),
    '#type'          => 'checkbox',
    '#description'   => t('Override the default settings for this content type.  The options below will only be used if this box is checked.'),
    '#default_value' => variable_get('media_'. $type_name .'_override', NULL),
  );
  $form['media']['media_'. $type_name .'_enabled'] = array(
    '#title'         => t('Enable Media resource browser'),
    '#type'          => 'checkbox',
    '#description'   => t('Enable the Media resource browser for this node type.'),
    '#default_value' => variable_get('media_'. $type_name .'_enabled', NULL),
  );

  // Extract the fields for this node type
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
            $options[$field['field_name'] .'--'. $id] = $registration['name'] .': '. $registration['description'];
          }

          $form['media'][$field['field_name']]['media_'. $type_name .'_'. $field['field_name'] .'_'. $kind] = array(
            '#title'         => t('Enable !kind options for this field', array('!kind' => $kind)),
            '#description'   => t('Choose which !kind options you would like to have enabled on this field', array('!kind' => $kind)),
            '#type'          => 'checkboxes',
            '#options'       => $options,
            '#default_value' => variable_get('media_'. $type_name .'_'. $field['field_name'] .'_'. $kind, array()),
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
  $settings_form = array();

  $settings_form['media_global_enabled'] = array(
    '#title'         => t('Media resource browser'),
    '#type'          => 'checkbox',
    '#description'   => t('Enable or Disable the Media resource browser for all types, unless specifically set for a given type.'),
    '#default_value' => variable_get('media_global_enabled', 1),
  );

  return system_settings_form($settings_form);
}

/**
 * Menu callback.  Display the list or thumbnails media admin display.
 */
function media_admin($display = 'list') {

  // Load our css.
  drupal_add_css(drupal_get_path('module', 'media') . '/media.css');
  
  switch ($display) {
    case 'list':
      return drupal_get_form('media_admin_list');
    case 'thumbnails':
      return drupal_get_form('media_admin_thumbnails');
  }
}

/**
 * Form builder: Builds the media list administration overview.
 */
function media_admin_list($form) {
  $path = drupal_get_path('module', 'media');
  
  // Build the 'Update options' form.
  $form = media_admin_operations($form);
  
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
   $options[$file->fid]['operations'] = l(t('edit'), 'media/file/' . $file->fid . '/edit', array('query' => $destination));
  }
  $form['files'] = array(
   '#type' => 'tableselect',
   '#header' => $header,
   '#options' => $options,
   '#empty' => t('No media available.'),
  );
  $form['pager'] = array('#markup' => theme('pager', array('tags' => NULL)));

  // Build the display switch.
  $display_types = array();
  $display_types[] = array(
    'data' => l(theme('image',
        array(
          'path' => $path . '/images/display-list.png',
          'alt' => 'List',
        )
      ),
      'admin/content/media/list',
      array(
        'html' => TRUE,
        'attributes' => array('title' => 'Display as a list.'),
      )
    ),
    'class' => array('media-display-list'),
  );
  $display_types[] = array(
    'data' => l(theme('image',
        array(
          'path' => $path . '/images/display-thumbnails.png',
          'alt' => 'Thumbnails',
        )
      ),
      'admin/content/media/thumbnails',
      array(
        'html' => TRUE,
        'attributes' => array('title' => 'Display as thumbnails.'),
      )
    ),
    'class' => array('media-display-thumbnails'),
  );
  drupal_alter('media_display_types', $display_types);
  $form['display_type'] = array(
    '#type' => 'markup',
    '#markup' => theme('item_list', array(
       'items' => $display_types,
       'attributes' => array('class' => 'media-display-switch'),
      )
    ),
  );

  return $form;
}

/**
 * Form builder: Builds the media thumbnails administration overview.
 */
function media_admin_thumbnails($form) {
  $path = drupal_get_path('module', 'media');
  
  // Build the 'Update options' form.
  $form = media_admin_operations($form);

  $query = db_select('file', 'f')->extend('PagerDefault')->extend('TableSort');
  $query->join('users', 'u', 'f.uid = u.uid');

  $result = $query
   ->fields('f')
   ->fields('u', array('name'))
   ->limit(50)
   ->execute();

  $destination = drupal_get_destination();
  $rows = array();
  $options = array();
  foreach ($result as $file) {
   $options[$file->fid] = theme('media_admin_thumbnail', array('file' => $file));
  }
  
  $form['files'] = array(
   '#type' => 'checkboxes',
   '#options' => $options,
  );
  
  $form['pager'] = array('#markup' => theme('pager', array('tags' => NULL)));

  // Build the display switch.
  $display_types = array();
  $display_types[] = array(
    'data' => l(theme('image',
        array(
          'path' => $path . '/images/display-list.png',
          'alt' => 'List',
        )
      ),
      'admin/content/media/list',
      array(
        'html' => TRUE,
        'attributes' => array('title' => 'Display as a list.'),
      )
    ),
    'class' => array('media-display-list'),
  );
  $display_types[] = array(
    'data' => l(theme('image',
        array(
          'path' => $path . '/images/display-thumbnails.png',
          'alt' => 'Thumbnails',
        )
      ),
      'admin/content/media/thumbnails',
      array(
        'html' => TRUE,
        'attributes' => array('title' => 'Display as thumbnails.'),
      )
    ),
    'class' => array('media-display-thumbnails'),
  );
  drupal_alter('media_display_types', $display_types);
  $form['display_type'] = array(
    '#type' => 'markup',
    '#markup' => theme('item_list', array(
       'items' => $display_types,
       'attributes' => array('class' => 'media-display-switch'),
      )
    ),
  );

  return $form;
}

function media_admin_operations($form = array()) {
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
      $args = array($nodes);
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
