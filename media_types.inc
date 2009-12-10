<?php
/**
 * Implement hook_entity_info().
 *
 * @todo: This has a pretty serious flaw.  In the case someone uploads
 * a file without a known "type" fields will puke.  We need to provide
 * some "default type in the install probably".
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
 * Implement hook_field_info().
 *
 * Field settings:
 * - allowed_values: a list array of one or more vocabulary trees:
 *   - vid: a vocabulary ID.
 *   - parent: a term ID of a term whose children are allowed. This should be
 *     '0' if all terms in a vocabulary are allowed. The allowed values do not
 *     include the parent term.
 *
 */
function media_field_info() {
  return array(
    'media' => array(
      'label' => t('Multimedia asset'),
      'description' => t('This field stores a reference to a multimedia asset.'),
      'default_widget' => 'media_generic',
      'default_formatter' => 'media_generic',
    ),
  );
}

/**
 * Update an existing media type or create a new one.
 *
 * @param object $type
 * @return void;
 */
function media_type_save($type) {
  $types = variable_get('media_types', array());
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
  }
  return $types;
}

/**
 * Returns a an associative array mapping mimeType patterns to media types.
 *
 * Example:
 * return array(
 *   'image/gif' => 'image',
 *   'image/png' => 'image,
 * );
 *
 * @return array
 */
function media_type_get_mime_map() {
  $map =& drupal_static(__FUNCTION__);

  if (!$map) {
    $map = array();
    $types = media_type_get_types();
    foreach ($types as $name => $type) {
      $map += array_fill_keys($type->mimeTypes, $name);
    }
  }
  return $map;
}