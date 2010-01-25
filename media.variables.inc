<?php
// $Id;

/**
 * @file Contains the variables used by media and their defaults.
 */

function media_variable_get($name, $default = NULL) {
  // Allow for an override of the default.
  // Useful when a variable is required (like $path), but namespacing still desired.
  if (!$default) {
    $default = media_variable_default($name);
  }
  // Namespace all variables
  $variable_name = 'media_' . $name;
  return variable_get($variable_name, $default);
}

function media_variable_set($name, $value) {
  $variable_name = 'media_' . $name;
  return variable_set($variable_name, $value);
}

function media_variable_default($name) {
  static $defaults;

  if (!$defaults) {
    $defaults = array();
    $defaults['wysiwyg_title'] = 'Media Browser';
    $defaults['wysiwyg_icon_title'] = 'Add Media';
    $defaults['wysiwyg_default_view_mode'] = 'media_large';
    // Name of the theme to use in media popup dialogs
    $defaults['dialog_get_theme_name'] = 'seven';
    $defaults['file_extensions'] = '';
    $defaults['max_filesize'] = '';
    $defaults['debug'] = FALSE;

    $defaults['file_list_size'] = 10;

    // The list of media types in the system... Should be a table probably
    $defaults['types'] = array();
    // Used in media.xml.inc
    $defaults['xml_cache_expire'] = 3600;
    // Was in cruft.inc $defaults['media_file_progress_image'] = '';
  }

  if (isset($defaults[$name])) {
    return $defaults[$name];
  }
}


?>
