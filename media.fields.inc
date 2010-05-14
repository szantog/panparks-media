<?php
// $Id$

/**
 * @file: Provides a "Multimedia asset" field to the fields API
 */

/**
 * Implement hook_field_info().
 */
function media_field_info() {
  return array(
    'media' => array(
      'label' => t('Multimedia asset'),
      'description' => t('This field stores a reference to a multimedia asset.'),
      'settings' => array(),
      'instance_settings' => array(),
      'default_widget' => 'media_generic',
      'default_formatter' => 'media_preview',
    ),
  );
}

/**
 * Implements hook_field_schema().
 */
function media_field_schema($field) {
  return array(
    'columns' => array(
      'fid' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => FALSE,
      ),
      'title' => array(
        'type' => 'varchar',
        'length' => 255,
        'not null' => FALSE,
      ),
      'data' => array(
        'type' => 'text',
        'not null' => FALSE,
        'size' => 'big',
        'serialize' => TRUE,
        //'description' => 'Used for storing additional information.  Can be harnessed by widgets',
      ),
    ),
    'indexes' => array(
      'fid' => array('fid'),
    ),
  );
}

/**
 * Implements hook_field_is_empty().
 */
function media_field_is_empty($item, $field) {
  if (!is_array($item) || (empty($item['fid']))) {
    return TRUE;
  }
  return FALSE;
}

/**
 *  Implements hook_field_formatter_prepare_view().
 */
function media_field_formatter_prepare_view($obj_type, $objects, $field, $instances, $langcode, &$items, $displays) {
  // Load the file items here?
  foreach ($items as $delta => $item) {
    foreach ($item as $file_delta => $file) {
      $file = (array)$file;
      $items[$delta][$file_delta] = media_load($file['fid']);
    }
  }
}

/**
 * Implement hook_field_widget_info().
 */
function media_field_widget_info() {
  return array(
    'media_generic' => array(
      'label' => t('File (media)'),
      'field types' => array('media'),
      'settings' => array(
        'progress_indicator' => 'throbber',
        'allowed_types' => array('image'),
        'allowed_schemes' => array('public', 'private'),
      ),
      'behaviors' => array(
        'multiple values' => FIELD_BEHAVIOR_DEFAULT,
        'default value' => FIELD_BEHAVIOR_NONE,
      ),
    ),
  );
}

/**
 * Implement hook_field_formatter_info
 */
function media_field_formatter_info() {
  $formatters = array();
  $build_modes = media_field_view_modes('media');
  foreach ($build_modes as $key => $mode) {
    $formatters[$key] = array(
      'label' => t($mode['label']),
      'field types' => array('media'),
    );
  }
  return $formatters;
}


/**
 * Used to Implement hook_field_view_modes().
 */
function media_field_view_modes($obj_type) {
  $modes = array();
  if ($obj_type == 'media') {
    $modes = array(
      'media_preview' => array('label' => t('Preview')),
      'media_small' => array('label' => t('Small')),
      'media_large' => array('label' => t('Large')),
      'media_original' => array('label' => t('Original')),
    );
  }
  return $modes;
}

function media_field_ui_view_modes_tabs() {
  $modes = array(
    'basic' => array(
      'view modes' => array_keys(media_field_view_modes('media')),
    ),
  );
  return $modes;
}

/**
 * Implements hook_field_prepare_view().
 */
function media_field_prepare_view($obj_type, $objects, $field, $instances, $langcode, &$items) {
  // @TODO: Do we wish to allow default files here?
}

/**
 * Implement hook_field_formatter_view
 */
function media_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
  $element = array();

  // An array of media items to pass into entity rendering functions.
  $media_items = array();

  foreach ($items as $delta => $item) {
    $media_items[$item->fid] = $item;
  }

  field_attach_prepare_view('media', $media_items, $display['type']);
  entity_prepare_view('media', $media_items);

  foreach ($items as $delta => $item) {
    // @TODO:
    // This works fine for Media styles, just not the default file types.
    // Find out why this doesn't automatically already route through file.
    // We're using a filefield in theory, created during the media_type
    // instantiation at media_type_configure_fields().
    // Other fields are routed properly, but not that default field...

    // First check if the default File formatters are sufficient.
//     $element[$delta] = file_field_formatter_view($obj_type, $object, $field, $instance, $langcode, $items, $display);

    // If this is not handled by the File module, then defer to Styles.
//     if (!$element[$delta]) {
      $element[$delta] = field_attach_view('media', $item, $display['type'], $langcode);

      // We'll defer to the Styles module to route formatting for the 'file'.
      $element[$delta]['file']['#theme'] = 'styles_field_formatter';
      $element[$delta]['file']['#element'] = $element[$delta]['file'];
//     }
  }

  return $element;
}

/**
 * Implement hook_field_widget_settings_form().
 */
function media_field_widget_settings_form($field, $instance) {

  $widget = $instance['widget'];
  $settings = $widget['settings'];
  $form = array();

  // Setup type selection form
  $types = media_type_get_types();
  $options = array();
  foreach ($types as $key => $definition) {
    $options[$key] = $definition->label;
  }

  $form['allowed_types'] = array (
    '#type' => 'checkboxes',
    '#title' => t('Allowed media types'),
    '#options' => $options,
    '#default_value' => $settings['allowed_types'],
    '#description' => t('Media types which are allowed for this field'),
    '#weight' => 1,
  );

  $streams = file_get_stream_wrappers();
  $options = array();
  unset($streams['temporary']);
  foreach ($streams as $scheme => $data) {
    $options[$scheme] = t('@scheme (@name)', array('@scheme' => $scheme . '://', '@name' => $data['name']));
  }
  $form['allowed_schemes'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Allowed URI schemes'),
    '#options' => $options,
    '#default_value' => $settings['allowed_schemes'],
    '#description' => t('URI schemes include public:// and private:// which are the Drupal files directories, and may also refer to remote sites.'),
    '#weight' => 2,
  );
  return $form;
}

function media_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $base) {
  $element = $base;
  $widget_settings = $instance['widget']['settings'];

  $defaults = array(
    'fid' => 0,
    'title' => '',
    'data' => NULL
  );

  $permission = user_access('use media browser');
  if (!$permission) {
    // @todo: show the default file upload field.
    // Maybe...
    return;
  }

  $current_value = array();
  // Obviously doesn't belong here, but in some lower level implementation.
  if (isset($items[$delta])) {
    $current_value = $items[$delta];
    $current_value['data'] = unserialize($current_value['data']);
  }

  $element += array(
    '#type' => 'media', // Would like to make this a fieldset, but throws some weird warning about element_children... not sure what it is about yet.
    '#collapsed' => TRUE,
    '#default_value' => $current_value,
    '#required' => $instance['required'],
    //'#media_file_extensions' => $instance['settings']['file_extensions'],
    // @TODO: Not implemented yet.
    '#media_options' => array(
      'global' => array(
        'types' => $instance['widget']['settings']['allowed_types'],
        // @todo: Not implemented
        'schemes' => $instance['widget']['settings']['allowed_schemes'],
      ),
    ),
    // Required to attach behaviors to the widget, @see theme_media_element();
    '#attributes' => array(
      'class' => array('media-widget'),
    ),
    '#process' => array('media_generic_widget_process'),
    // Add extra Field properties.
    '#field_name' => $field['field_name'],
    '#bundle' => $instance['bundle'],
    '#entity_type' => $instance['entity_type'],
    '#value_callback' => 'media_field_widget_value',
  );

  // This seems like a roayl hack.  What's a better way to do this?
  $settings = array('media' => array('fields' => array('edit-' . str_replace('_', '-', $instance['field_name']) => $element['#media_options'])));
  // hmm... Might need to think about this.
  // All settings would likely apply to all media in a multi-value, but what about passing the existing fid?
  drupal_add_js($settings, 'setting');

  return $element;
}

/**
 * An element #process callback for the media_managed_file type.
 *
 * Expands the media_managed_file type to include the uri field.
 */
function media_generic_widget_process($element, &$form_state, $form) {
  $fid = isset($element['#default_value']['fid']) ? $element['#default_value']['fid'] : 0;
  $file = media_load($fid);
  $path = drupal_get_path('module', 'media');

  if ($file) {
    $element['filename'] = array(
      '#type' => 'item',
      '#title' => $file->filename,
      '#markup' => '',
      '#prefix' => '<h3 class="media-filename">',
      '#suffix' => '</h3>',
    );
  }

  //@TODO: This should be loaded from the JS in case of a failed form submission.
  $element['preview'] = array(
    '#type' => 'item',
    '#markup' => empty($file) ? '<div class="empty-preview"><a class="launcher" href="#">' . t(media_variable_get('field_select_media_text')) . '</a></div>' : theme('media_admin_thumbnail', array('file' => $file, 'style_name' => 'thumbnail')),
    '#prefix' => '<div class="preview launcher">',
    '#suffix' => '</div>',
  );

  $element['more_fields_wrapper'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Meta-data'),
  );

  $element['more_fields_wrapper']['edit'] = array(
    '#type' => 'markup',
    '#markup' => l(t('Edit'), 'media/' . $fid . '/edit', array('query' => array('render' => 'media-popup'), 'attributes' => array('class'=> array('media-edit-link')))),
  );

  // Oh god, there must be a better way to add a wrapper.
  $parents = $element['#parents'];
  array_push($parents, 'more_fields');

  if ($file) {
    $media_wo_file = $file;
    $media_wo_file->file = array(); // heh, let's see how this works.
    field_attach_prepare_view('media', array($file->fid => $media_wo_file));
    entity_prepare_view('media', array($file->fid => $media_wo_file));
    $element['more_fields_wrapper']['more_fields'] = field_attach_view('media', $media_wo_file, 'media_preview');
  }

  //@HACK: @todo: this is so I can find it in media.js without putting every field in a settings variable.
  // If I make it hidden (which it should be) it will go to the top of the form... I know this sucks.
  // This is hidden in media.css
  $element['fid'] = array(
    '#type' => 'textfield',
    '#default_value' => $fid,
    '#attributes' => array('class' => array('fid')),
  );

  // Media browser attach code.
  $element['#attached']['js'][] = drupal_get_path('module', 'media') . '/javascript/media.fields.js';
  module_load_include('inc', 'media', 'media.browser');
  media_include_browser_js();

  return $element;
}

/**
 * Implements hook_field_validate().
 *
 * Possible error codes:
 * - 'media_fid_illegal_value': The value is not part of the list of allowed values.
 */
function media_field_validate($obj_type, $object, $field, $instance, $langcode, $items, &$errors) {

  $allowed_types = array_keys(array_filter($instance['widget']['settings']['allowed_types']));

    // @TODO: merge in stuff from media_uri_value
  foreach ($items as $delta => $item) {
    if ($item['fid'] == 0) {
      return TRUE;
      //@TODO: make support for submiting with just a URI here?
    }
    $result = db_select('file_managed', 'f')
      ->fields('f')
      ->condition('fid', $item['fid'])
      ->condition('type', $allowed_types, 'IN')
      ->execute()
      ->fetchField();
    if (!$result) {
      $errors[$field['field_name']][$langcode][$delta][] = array(
        'error' => 'media_fid_illegal_value',
        'message' => t('%name: illegal value.', array('%name' => t($instance['label']))),
      );
    }
  }
}

function media_field_widget_value($element, $input, $form_state) {
  $return = $input;
  if (!is_array($return)) {
    $return = array();
  }

  if (isset($return['data'])) {
    $return['data'] = serialize($return['data']);
  }

  $return += array(
    'fid' => 0,
    'title' => '',
    'data' => NULL,
  );

  return $return;
}
