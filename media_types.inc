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
  $return = array(
    'media' => array(
      'label' => t('Media'),
      'base table' => 'file',
      'fieldable' => TRUE,
      'object keys' => array(
        'id' => 'fid',
        'bundle' => 'type',
      ),
      'bundle keys' => array(
        'bundle' => 'machine_name',
      ),
      'bundles' => array(),
    ),
  );

  $media_types = media_type_get_types();

  foreach ($media_types as $type => $bundle_info) {
    $return['media']['bundles'][$type] = (array)$bundle_info;
    $return['media']['bundles'][$type]['admin'] = array(
      'path' => 'admin/structure/media/manage/%media_type',
      'real path' => 'admin/structure/media/manage/' . $type,
      'bundle argument' => 4,
      'access arguments' => array('administer site configuration'),
    );
  }
  return $return;
}

/**
 * Update an existing media type or create a new one.
 *
 *  @param object $type
 *    $type is an object with the following fields:
 *      ->machine_name => The name of the media asset type;
 *      ->label => The human readable name;
 *      ->base => @TODO: What's this?
 *      ->extensions => An array of allowed extensions; @TODO:replace w/ callback
 *      ->mimeTypes => (obsolete).
 *  @return void;
 */
function media_type_save($type) {
  $types = variable_get('media_types', array());
  $type = media_type_set_defaults($type);
  $types[$type->machine_name] = $type;

  variable_set('media_types', $types);
  // Clear the caches
  drupal_static_reset('media_type_get_types');
  drupal_static_reset('media_type_get_mime_map');
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
 * Loads all media types into an array keyed by machine name.
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
//     dpr($type);
    if (call_user_func_array($type->type_callback, array($file, $type->type_callback_args))) {
      return $name;
    }
  }
}

/**
 *  Default callback used to determine if a piece of media is of a given type.
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
  }
}
