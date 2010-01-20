<?php

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
 * @param
 * Takes a match of tag code
 * @return
 * Return the replaced markup
 */
function _media_markup($match) {
  $match = str_replace("[[","",$match);
  $match = str_replace("]]","",$match);
  $media = drupal_json_decode($match[0]);
  $view_mode = $media['view_mode'];
  // loads media object only when media fid found in json object, to prevent
  // errors. @TODO return something when fid is not found.
  if(isset($media['fid'])) {
    $media_obj = media_load($media['fid']);
    return drupal_render(media_get_file_without_label($media_obj, $view_mode));
  }
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
      $tagmap[$match[0]] = _media_markup($match);
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
  $form = array();
  $options = array();
  foreach (media_field_view_modes('media') as $key => $view_mode) {
    //@TODO: Get the file formatter, and display that here too.
    $options[$key] = $view_mode['label'];
    $formats[$key] = drupal_render(media_get_file_without_label($media, $key));
  }

  // This totally doesn't work because the dialog API doesn't support it.
  $settings = array();
  $settings['media'] = array('formatFormFormats' => $formats);
  drupal_add_js($settings, 'setting');

  drupal_add_library('media', 'media_base');

  $path = drupal_get_path('module', 'media');
  $form['#attached']['js'][] = $path . '/javascript/media-format-form.js';

  $form['preview'] = array(
    '#type' => 'markup',
    '#title' => basename($media->uri),
    '#markup' => drupal_render(field_view_field('media', $media, 'file', 'media_preview'))
  );

  $form['format'] = array(
    '#type' => 'radios',
    '#title' => t('Format'),
    '#options' => $options,
    '#default_value' => 'media_original',
  );

  return $form;
}

/**
 * Wrapper around field_view_field, returns the file field w/o a label
 *
 * @param Object $media
 * @param string $view_mode
 * @return array
 *  drupal_renderable array
 */
function media_get_file_without_label($media, $view_mode) {
  $instance = field_info_instance('media', 'file', $media->type);
  $format = $instance['display'][$view_mode];
  $format['label'] = 'hidden';
  return field_view_field('media', $media, 'file', $format);
}
