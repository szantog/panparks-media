<?php


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
    return drupal_render(field_view_field('media', $media_obj, 'file', $view_mode));
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

function media_process_form(&$form) {
  foreach(element_children($form) as $item) {
    if(isset($form[$item]['#text_format'])) {
      if(isset($form[$item]['#default_value'])) {
        $text = $form[$item]['#default_value'];
        $tagmap = _media_generate_tagMap($text);
        if(!empty($tagmap)) {
          drupal_add_js(array('tagmap' => array_unique($tagmap)), 'setting');
         }
      }
      // Continue if we found our text_format, so that it doesn't recurse.
      continue;
    }
    media_process_form($form[$item]);
  }
  return $form;
}

function media_wysiwyg_attach_alter(&$element) {
  //dpr($element);
  //die();
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
