<?php
// $Id$

/**
 * @file
 * Functions related to the WYSIWYG editor.
 */

/**
 * Implementation of hook_wysiwyg_include_directory()
 */

function media_wysiwyg_include_directory($type) {
  switch($type) {
    case 'plugins':
      return $type;
        break;
    }
}

/**
 * Implement hook_filter_info
 * @return unknown_type
 */
function media_filter_info() {
  $filters['media_filter'] = array(
    'title' => t('Converts Media tags to Markup'),
    'description' => t('This filter will convert [[{type:media... ]] tags into markup'),
    'process callback' => 'media_filter',
    'tips callback' => 'media_filter_tips', // @TODO not implemented
    'cache' => FALSE,
  );
  return $filters;
}

//
/**
 * Implement hook_filter();
 * @TODO check for security probably pass text through filter_xss
 * @return unknown_type
 */
function media_filter($text, $filter) {
  $text = ' ' . $text . ' ';
  $text = preg_replace_callback("/\[\[.*?]]/s",'_media_markup', $text);
  return $text;
}

/**
 * Replace callback to convert tag into markup
 * @param string $match
 * Takes a match of tag code
 * @param boolean $wysiwyg
 *   Set to TRUE if called from within the WYSIWYG text area editor.
 * @return
 * Return the replaced markup
 */
function _media_markup($match, $wysiwyg = FALSE) {
  $match = str_replace("[[","",$match);
  $match = str_replace("]]","",$match);
  $tag = $match[0];

  try {
    if (!is_string($tag)) {
      throw new Exception('Unable to find matching tag');
    }
    $media = drupal_json_decode($tag);
    if (!isset($media['fid'])) {
      throw new Exception('No file Id');
    }
    if (!isset($media['view_mode'])) {
      // Should we log or throw an exception here instead?
      // Do we need to validate the view mode for fields API?
      $media['view_mode'] = media_variable_get('wysiwyg_default_view_mode');
    }
    $media_obj = media_load($media['fid']);
    if (!$media_obj) {
     throw new Exception('Could not load media object');
    }
    $settings = is_array($media['attributes']) ? $media['attributes'] : array();
    // @TODO: What case does this provide for?  Can we add this logic in JS when we embed it?
    // This doesn't look great to me.  Also won't work if the style has anything
    // between width and height (or if they are in reverse order).
    if (isset($settings['style'])) {
      if (preg_match('@width: (.+?)px; height: (.+?)px;@i', $settings['style'], $matches)) {
        $settings['width'] = $matches[1];
        $settings['height'] = $matches[2];
      }
    }
    if ($wysiwyg) {
      $settings['wysiwyg'] = $wysiwyg;
    }
  }
  catch (Exception $e) {
    watchdog('media', 'Unable to render media from %tag. Error: %error', array('%tag' => $tag, '%error' => $e->getMessage()));
    return FALSE;
  }

  return drupal_render(media_get_file_without_label($media_obj, $media['view_mode'], $settings));
}

/**
 * Process callback which will be called when form is completely built
 * This function will scan all textareas with text_format enabled and
 * add associative array of tag code => markup to Drupal.settings in JS
 * @param $form
 *
 * @see
 * _media_generate_tagMap
 */

function media_process_form(&$form, $form_state = array()) {
  foreach(element_children($form) as $item) {
    if(isset($form[$item]['#text_format'])) {
      // Make sure it gets created regardless of if we use it
      if (!isset($tagmap)) {
        $tagmap = array();
      }
      if(isset($form[$item]['#value'])) {
        $text = $form[$item]['#value'];
        $tagmap = _media_generate_tagMap($text);
      }
      // Continue if we found our text_format, so that it doesn't recurse.
      continue;
    }
    media_process_form($form[$item]);
  }
  if (isset($tagmap)) {
    drupal_add_js(array('tagmap' => array_unique($tagmap)), 'setting');
  }
  return $form;
}

/**
 * Generates an array of [inline tags] => <html> to be used in filter
 * replacement and to add the mapping to JS.
 * @param
 * The String containing text and html markup of textarea
 * @return
 * An associative array with tag code as key and html markup as the value.
 *
 * @see
 * media_process_form
 * _media_markup
 */
function _media_generate_tagMap($text) {
  // Making $tagmap static as this function is called many times and
  // adds duplicate markup for each tag code in Drupal.settings JS,
  // so in media_process_form it adds something like tagCode:<markup>,
  // <markup> and when we replace in attach see two duplicate images
  // for one tagCode. Making static would make function remember value
  // between function calls. Since media_process_form is multiple times
  // with same form, this function is also called multiple times.
  static $tagmap = array();
  preg_match_all("/\[\[.*?]]/s", $text, $matches, PREG_SET_ORDER);
  foreach($matches as $match) {
    // We see if tagContent is already in $tagMap, if not we add it
    // to $tagmap otherwise return a empty error
    if(empty($tagmap[$match[0]])) {
      $tagmap[$match[0]] = _media_markup($match, TRUE);
    }
    else {
      return array();
    }
  }
  return $tagmap;
}

/**
 *
 * @param unknown_type $form
 * @param unknown_type $media
 * @return unknown_type
 */
function media_format_form($form, $form_state, $media) {
  // This will vary depending on the media type.

  $form = array();
  $form['#media'] = $media;
  $instance_info = field_info_instance('media', 'file', $media->type);
  $entity_info = entity_get_info('media');

  $options = array();
  foreach ($entity_info['view modes'] as $key => $title) {
    $format = $instance_info['display'][$key];
    //If the format is set to hidden, don't offer it
    if ($format['type'] == 'hidden') {
      continue;
    }

    //@TODO: Display more verbose information about which formatter and what it does.
    $options[$key] = $title['label'];
    $formats[$key] = drupal_render(media_get_file_without_label($media, $key, array('wysiwyg' => TRUE)));
  }

  if (!count($formats)) {
    throw new Exception('Unable to continue, no available formats for displaying media.');
    return;
  }

  $default_view_mode = media_variable_get('wysiwyg_default_view_mode');
  if (!isset($formats[$default_view_mode])) {
    $default_view_mode = key($formats);
  }

  // Add JS and settings array of formats.
  $settings = array();
  $settings['media'] = array('formatFormFormats' => $formats);
  drupal_add_js($settings, 'setting');

  drupal_add_library('media', 'media_base');
  drupal_add_library('system', 'form');

  $path = drupal_get_path('module', 'media');
  $form['#attached']['js'][] = $path . '/javascript/media-format-form.js';
  $form['#attached']['css'][] = $path . '/css/media-format-form.css';

  $form['heading'] = array(
    '#type' => 'markup',
    '#prefix' => '<h1 class="title">',
    '#suffix' => '</h1>',
    '#markup' => t('Embedding %filename', array('%filename' => $media->filename)),
  );

  $preview = field_view_field('media', $media, 'file', 'media_preview');
  $preview['#theme_wrappers'][] = 'media_thumbnail';

  $form['preview'] = array(
    '#type' => 'markup',
    '#title' => basename($media->uri),
    '#markup' => drupal_render($preview),
  );

  $form['format_group'] = array(
    '#type' => 'fieldset',
    '#title' => 'format',
    '#prefix' => t('<div class="clearfix" id="format-group-wrapper"><span id="format-description">Adding in <span id="selected-format">%currently_selected</span> format.  </span>', array('%currently_selected' => $options[$default_view_mode])),
    '#suffix' => t('</div>'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['format_group']['format'] = array(
    '#type' => 'radios',
    '#title' => t('Format'),
    '#options' => $options,
    '#default_value' => $default_view_mode,
  );

  // These will get passed on to WYSIWYG
  $form['options'] = array(
    '#type' => 'fieldset',
    '#title' => t('options'),
    '#description' => t('Alternate text a user will see if the image is not available'),
  );

  // Similar to a form_alter, but we want this to run first so that media.types.inc
  // can add the fields specific to a given type (like alt tags on media).
  // If implemented as an alter, this might not happen, making other alters not
  // be able to work on those fields.
  // @TODO: We need to pass in existing values for those attributes.
  drupal_alter('media_format_form_prepare', $form, $form_state, $media);

  if (!element_children($form['options'])) {
    $form['options']['#attributes'] = array('style' => 'display:none');
  }

  return $form;
}



/**
 * Wrapper around field_view_field, returns the file field w/o a label
 *
 * @param Object $media
 * @param string $view_mode
 * @param array $settings
 *  Any attribute overrides to pass to the style formatter.
 * @return array
 *  drupal_renderable array
 */
function media_get_file_without_label($media, $view_mode, $settings = array()) {
  $instance = field_info_instance('media', 'file', $media->type);
  $format = $instance['display'][$view_mode];
  $format['label'] = 'hidden';
  $format['settings'] = array_merge($format['settings'], $settings);
  $media->override = $settings;
  return field_view_field('media', $media, 'file', $format);
}
