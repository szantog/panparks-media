<?php
// $Id$

/**
 *  @file
 *  Define Media entity types.
 */

/**
 * Implement hook_entity_info().
 *
 * @todo: This has a pretty serious flaw.  In the case someone uploads
 * a file without a known "type" fields will puke.  We need to provide
 * some "default type in the install probably". @TODO: done using other now?
 */
function media_entity_info() {
  // @todo: change this function name now that it is no longer a hook.
  // @what? hook_entity_info is still a hook.
  $view_modes = media_field_view_modes('media');
  $return = array(
    'media' => array(
      'label' => t('Media'),
      'base table' => 'file',
      'fieldable' => TRUE,
      'view modes' => $view_modes,
      'object keys' => array(
        'id' => 'fid',
        'bundle' => 'type', // @TODO: Why 'type' here...
      ),
      'bundle keys' => array(
        'bundle' => 'machine_name', // @TODO: But 'machine_name' here?
      ),
      'bundles' => array(),
    ),
  );

  $media_types = media_type_get_types();

  foreach ($media_types as $type => $bundle_info) {
    $return['media']['bundles'][$type] = (array)$bundle_info;
    $return['media']['bundles'][$type]['admin'] = array(
      'label' => $bundle_info->label,
      'path' => 'admin/structure/media/manage/%media_type',
      'real path' => 'admin/structure/media/manage/' . $type,
      'bundle argument' => 4,
      // @TODO: Add an 'administer media types' perm?
      'access arguments' => array('administer site configuration'),
    );
  }

  // @TODO: Do we need to define search-related view modes for attached fields?
  return $return;
}

/**
 *  Update an existing media type or create a new one.
 *
 *  The default media types are currently 'Audio', 'Image', 'Video', and
 *  'Other', which are defined in media_install().
 *
 *  @param object &$type
 *    $type is an object with the following fields:
 *      ->machine_name => The name of the media asset type, such as 'video';
 *      ->label => The human readable name;
 *      ->base => boolean: If the media type cannot be removed.
 *      ->type_callback => Function call to filter an instance to its bundle.
 *      ->type_callback_args => An array of args to be passed to type_callback.
 *      ->view_mode_defaults => An array of view_mode_key => format.
 *  @return void;
 */
function media_type_save(&$type) {
  $types = variable_get('media_types', array());
  $type = media_type_set_defaults($type);
  $types[$type->machine_name] = $type;

  // Set up the defaults for this media type bundle.
  media_type_configure_fields($type);
  media_type_configure_formatters($type->machine_name, $type->view_mode_defaults);
  // We don't need to save this information
  unset($type->view_mode_defaults);
  variable_set('media_types', $types);
  // Clear the caches
  drupal_static_reset('media_type_get_types');
  drupal_static_reset('media_type_get_mime_map');
}

/**
 * Assigns formatters to view_modes for the file field of a media type.
 *
 * Each media type contains a field called "file" and certain stock view modes.
 * This function will assign a format to each view mode. For instance, images
 * would want to use a thumbnail format for files when in the "preview"
 * view_mode.  The called would pass in 'image' for $machine_name and
 * array('preview' => 'styles_file_square_thumbnail') to $view_modes_to_formatters.
 *
 * @param string $machine_name
 * @param array $view_modes_to_formatters
 * @throws Exception
 * @return void
 */
function media_type_configure_formatters($machine_name, $view_modes_to_formatters) {
  $instance = field_info_instance('media', 'file', $machine_name);
  if (!$instance) {
    throw new Exception('Unable to set formatter preferences for '. $machine_name);
  }
  foreach ($instance['display'] as $view_mode => $display) {
    if (isset($view_modes_to_formatters[$view_mode])) {
      $instance['display'][$view_mode]['type'] = $view_modes_to_formatters[$view_mode];
    }
  }
  field_update_instance($instance);
}

/**
 * Loads a media type based on its machine name.
 *
 * @param string $machine_name
 * @return StdClass
 */
function media_type_load($machine_name) {
  $types = media_type_get_types();
  if (isset($types[$machine_name])) {
    return $types[$machine_name];
  }
}

/**
 * Sets up the default fields which a media type needs.
 *
 * Currently, this is just a "file" field which contains the reference to the actual file.
 * @TODO Do we also want to make the file 'description/caption' a field?
 *
 * @param StdClass $media_type
 * @return void
 */
function media_type_configure_fields($media_type) {
  $field = field_info_field('file');
  $instance = field_info_instance('media', 'file', $media_type->machine_name);

  if (empty($field)) {
    $field = array(
      'field_name' => 'file',
      'type' => 'file',
    );
    $field = field_create_field($field);
  }

  // Perhaps the file field exists already, if so skip.
  if (empty($instance)) {
    $weight = -5;
    $instance = array(
      'field_name' => 'file',
      'object_type' => 'media',
      'bundle' => $media_type->machine_name,
      'label' => 'File',
      'widget_type' => 'file_file',
      'widget' => array(
        'weight' => $weight,
      ),
      'required' => TRUE,
      'locked' => TRUE,
      'display' => array(
        'media_preview' => array(
          'label' => 'hidden',
          'type' => 'file_default',
          'weight' => $weight,
        ),
        'media_original' => array(
          'label' => 'hidden',
          'type' => 'file_default',
          'weight' => $weight,
        ),
      ),
    );
    field_create_instance($instance);
  }
}

/**
 *  Loads all media types into an array keyed by machine name and sorted
 *  and weighted lexographically.
 *
 * @return array
 *  Media types keyed by machine name.
 */
function media_type_get_types() {
  $types =& drupal_static(__FUNCTION__);

  if (!$types) {
    $types = variable_get('media_types', array());
    uasort($types, 'drupal_sort_weight');
  }
  return $types;
}

/**
 *  Create the basic class and defaults for a media entity bundle type.
 */
function media_type_set_defaults($info) {
  $type = new StdClass();

  // This is used to filter a file to the proper bundle.
  $type->type_callback = 'media_is_type';
  $type->type_callback_args = array();
  $type->weight = 0;

  foreach ($info as $k => $v) {
    $type->{$k} = $v;
  }

  return $type;
}

/**
 * From the looks of other parts of the code.
 * This is not needed, but it was simpler to code up for now.
 *
 * @todo: integrate this properly with other APIs in media when fields is done
 * @param unknown_type $file
 * @return unknown_type
 */
function media_get_type($file) {
  $types = media_type_get_types();
  foreach ($types as $name => $type) {
    if (call_user_func_array($type->type_callback, array($file, $type->type_callback_args))) {
      return $name;
    }
  }
}

/**
 *  Default callback used to determine if a piece of media is of a given type.
 *
 * @TODO: document 'any' and 'all' matching.
 *
 *  @param $media
 *    The media file asset object.
 *  @param $args
 *
 *  @return unknown_type
 */
function media_is_type($media, $args) {
  $match_type = !empty($args['match_type']) ? 'any' : $args['match_type'];
  $match_all = $match_type == 'all';
  if (!empty($args['mimetypes'])) {
    foreach ($args['mimetypes'] as $expression) {
      if (preg_match($expression, $media->filemime)) {
        if (!$match_all) {
          return TRUE;
        }
      }
    }
    // Was not matched, so return
    if ($match_all) {
      return FALSE;
    }
  }

  if (!empty($args['extensions'])) {
    if (in_array(pathinfo($media->uri, PATHINFO_EXTENSION), $args['extensions'])) {
      if (!$match_all) {
        return TRUE;
      }
    }
    // Was not matched, so return
    if ($match_all) {
      return FALSE;
    }
  }

  if (!empty($args['streams'])) {
    // @TODO: Write this.
    // Note that this will ultimately need to be above mimetypes and extensions
    // so that we can allow for things like flickr photos vs. slideshows, etc.
  }
}
