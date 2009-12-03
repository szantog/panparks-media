<?php
// $Id$

/**
 * Implement hook_entity_info().
 */
function media_entity_info() {
  $return = array(
    'media' => array(
      'label' => t('Media'),
      //'controller class' => 'MediaController',
      'base table' => 'media_metadata',
      'fieldable' => TRUE,
      'object keys' => array(
        'id' => 'mid',
        'bundle' => 'media_type',
      ),
//      'bundle keys' => array(
//        'bundle' => 'machine_name',
//      ),
      'bundles' => array(),
    ),
  );

  $image_types = array (
    'image' => array(
      'label' => 'Image',
    ),
    'audio' => array(
      'label' => 'Audio',
    ),
    'video' => array(
      'label' => 'Video',
    ),
  );
  foreach ($image_types as $type => $bundle_info) {
    $return['media']['bundles'][$type] = $bundle_info;
    $return['media']['bundles'][$type]['admin'] = array(
      'path' => 'admin/media/type/%media_type',
      'real path' => 'admin/media/type/' . $type,
      'bundle argument' => 3,
      'access arguments' => array('administer taxonomy'),
    );
  }

  return $return;
}


/**
 * Implement hook_entity_info().
 */
function media_entity_info() {
  $return = array(
    'media' => array(
      'label' => t('Media'),
      'controller class' => 'MediaEntityController',
      'base table' => 'file',
      'fieldable' => TRUE,
      'object keys' => array(
        'id' => 'fid',
        'bundle' => 'type',
      ),
      'bundles' => array(),
    ),
  );

  $media_types = module_invoke_all('media_types');

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
  Implement hook_media_types
 */
function media_media_types() {
  $types = array();
  $types['image'] = new StdClass();
  $types['image']->label = "Image";
  $types['image']->base = TRUE;
  $types['image']->extensions = array('jpg', 'jpeg', 'gif', 'png', 'tiff');
  // @todo:not implement
  $types['image']->mimeTypes = array('/$image/');

  $types['audio'] = new StdClass();
  $types['audio']->label = "Audio";
  $types['image']->base = TRUE;
  $types['image']->extensions = array('mp3', 'ogg', 'wma');
  $types['image']->mimeTypes = array('/$audio/');

  $types['video'] = new StdClass();
  $types['video']->label = "Video";
  $types['image']->base = TRUE;
  $types['image']->extensions = array('mov', 'mp4', 'avi');
  $types['image']->mimeTypes = array('/$video/');

  return $types;
}


/**
 * Menu argument handler.  Loads a media type definition when %media_type is in the path.
 *
 * @param string $type
 * @return StdClass
 */
function media_type_load($type) {
  return $type;
  $media_types = module_invoke_all('media_types');
  return $media_types[$type];
}

function media_load($fid) {
  return entity_load('media', array($fid));
}

/**
 * Implement hook_permission().
 */
function media_permission() {
  return array(
    'administer media' => array(
      'title' => t('Administer media'),
      'description' => t('Add, edit or delete media.'),
    ),
  );
}