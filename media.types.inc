<?php
// $Id$

/**
 *  @file
 *  Define Media entity types, also provides support for the base types.
 *
 *  See media.install for a list of the base types.
 *  @TODO: Make media types have their own table.
 */

/**
 * Implement hook_entity_info().
 *
 */
function media_entity_info() {
  $view_modes = media_field_view_modes('media');
  $return = array(
    'media' => array(
      'label' => t('Media'),
  	  'entity class' => 'Media',
      'controller class' => 'MediaEntityController',
      'base table' => 'file',
      'fieldable' => TRUE,
      'view modes' => $view_modes,
      'object keys' => array(
        'id' => 'fid',
        'bundle' => 'type', // @TODO: Why 'type' here...
      ),
      'bundle keys' => array(
        'bundle' => 'name', // @TODO: But 'name' here?
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
 *      ->name => The name of the media asset type, such as 'video';
 *      ->label => The human readable name;
 *      ->base => boolean: If the media type cannot be removed.
 *      ->type_callback => Function call to filter an instance to its bundle.
 *      ->type_callback_args => An array of args to be passed to type_callback.
 *  @return void;
 */
function media_type_save(&$type) {
  if (empty($type->name)) {
    throw new Exception('Enable to add type, name not provided');
  }

  $type = media_type_set_defaults($type);
  if (!is_array($type->type_callback_args)) {
    throw new Exception('type_callback_args should be an array');
  }

  $type->type_callback_args = serialize($type->type_callback_args);

  $ret = db_merge('media_type')
    ->key(array('name' => $type->name))
    ->fields((array)$type)
  ->execute();

  media_type_configure_fields($type);

  // Clear the caches
  drupal_static_reset('media_type_get_types');
  drupal_static_reset('media_type_get_mime_map');
  return;
}



/**
 * Assigns formatters to view_modes for the file field of a media type.
 *
 * Each media type contains a field called "file" and certain stock view modes.
 * This function will assign a format to each view mode. For instance, images
 * would want to use a thumbnail format for files when in the "preview"
 * view_mode.  The called would pass in 'image' for $name and
 * array('preview' => 'styles_file_square_thumbnail') to $view_modes_to_formatters.
 *
 * @param string $name
 * @param array $view_modes_to_formatters
 * @throws Exception
 * @return void
 */
function media_type_configure_formatters($name, $view_modes_to_formatters) {
  $instance = field_info_instance('media', 'file', $name);
  if (!$instance) {
    throw new Exception('Unable to set formatter preferences for '. $name);
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
 * @param string $name
 * @return StdClass
 */
function media_type_load($name) {
  $types = media_type_get_types();
  if (isset($types[$name])) {
    return $types[$name];
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
  $instance = field_info_instance('media', 'file', $media_type->name);

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
      'bundle' => $media_type->name,
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
    $types = db_select('media_type', 'mt')
      ->orderBy('weight')
      ->fields('mt')
      ->execute()
      ->fetchAllAssoc('name'); // Will key by the name field.
    foreach ($types as &$type) {
      // I really hate this.
      $type->type_callback_args = unserialize($type->type_callback_args);
    }
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
 * Determines the type of media a passed in $file is.
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
  throw new Exception('Unable to determine type of media from ' . var_export($file, 1));
}

/**
 * Default callback used to determine if a piece of media is of a given type.
 *
 * @TODO: document 'any' and 'all' matching.
 *
 * @param $media
 *   The media file asset object.
 * @param $args
 *
 * @return unknown_type
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

/**
 * Implement hook_media_format_form_prepare_alter
 * @return unknown_type
 */
function media_media_format_form_prepare_alter(&$form, &$form_state, $media) {
  switch($media->type) {
    case 'image':
      // @TODO: Isn't there a $file->description?
      $description = $media->filename;
      $form['options']['alt'] = array(
        '#type' => 'textfield',
        '#title' => t('Description'),
        '#default_value' => $description,
      );
      break;
  }
}


class Media extends EntityDb {
  public function __construct($values = array()) {
    parent::__construct($values, 'media');
  }
}

class MediaEntityController extends EntityAPIController {
  
  public $customConditions = array();
  public $start;
  public $limit;
  
  /**
   * Wrapper around the DB_API to allow conditions which are not ==
   * 
   * @param $key
   * @param $value
   * @param $operator
   * @return unknown_type
   */
  public function addCondition($key, $value, $operator = NULL) {
    $this->customConditions = array($key, $value, $operator);
  }

  /**
   * Takes a DB query and... does what?
   * @param <type> $query
   */
  public function getMedia($query) {

  }
  /**
   * Overloaded (and this sucks).  $conditions can be an array or key=>value pairs or DBNTG conditions.
   * Sets status = TRUE by default.
   *  
   * @see sites/all/modules/media/includes/EntityAPIController#load($ids, $conditions)
   */
  public function load($ids = array(), $conditions = array(), $start = NULL, $limit = NULL) {
    // Conditions using the = (equals) operator.
    $normal_conditions = array();
    $normal_conditions['status'] = TRUE;

    $this->start = $start;
    $this->limit = $limit;

    foreach ($conditions as $k => $condition) {
      if (is_array($condition)) {
        $this->customConditions[] = array($condition[0], $condition[1], $condition[2]);
        // We can't use caching in this case
        $this->cache = FALSE;
      }
      else {
        $normal_conditions[$k] = $condition;
      }
    }
    
    // Unfortuantely, the load method won't query unless there is at least one condition
    // This is a bad pattern and undocumented side-effect.
    // @todo: refactor this.
    return parent::load($ids, $normal_conditions);
  }
  
  public function buildQuery() {
    parent::buildQuery();
    if ($this->limit) {
      $this->query->range($this->start, $this->limit);
    }
    foreach ($this->customConditions as $condition) {
      $this->query->condition($condition[0], $condition[1], $condition[2]);
    }
  }
}

