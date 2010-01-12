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
 */
function _media_markup($match) {
  $match = str_replace("[[","",$match);
  $match = str_replace("]]","",$match);
  $media = drupal_json_decode($match[0]);
  $view_mode = $media['view_mode'];
  $media_obj = media_load($media['fid']);
  return  '<div class="media-embedded">'. drupal_render(field_view_field('media', $media_obj, 'file', $view_mode)). '</div>' ;
}



/**
 * Implementation of hook_node_load()
 * This would ensure that tagmap is added to Drupal.settings on node/edit,
 * there could be a better way to do this.
 * @todo: This will not work, needs to work with every textarea w/ wysiwyg.
 */

function media_node_load($nodes, $types) {
  foreach ($nodes as $node) {
    if (isset($node->body[LANGUAGE_NONE])) {
      $text = $node->body[LANGUAGE_NONE][0]['value'];
      $tagmap = _media_generate_tagMap($text);
    }
  }
  if (!empty($tagmap)) {
    drupal_add_js(array('tagmap' => $tagmap), 'setting');
  }
}

function media_wysiwyg_attach_alter(&$element) {
  //dpr($element);
  //die();
}

/**
 * Generates an array of [inline tags] => <html> to display them.
 * @param unknown_type $text
 * @return unknown_type
 */
function _media_generate_tagMap($text) {
  $tagmap = array();
  preg_match_all("/\[\[.*?]]/s", $text, $matches, PREG_SET_ORDER);
  foreach($matches as $match) {
    $tagmap[$match[0]] = _media_markup($match);
  }
  return $tagmap;
}
