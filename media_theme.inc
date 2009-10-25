<?php
// $Id$

/**
 * @file
 * Media Theming
 *
 * Theming functions for the Media module.
 */

/**
 * Display the media file browser.
 *
 * @param array $element
 *   The form element.
 * @return string
 */
function theme_media_file_browser($element) {
  // Add the CSS for our display.
  $output = '<div class="media browser">' . $element . '</div>';

  return $output;
}

/**
 * Display a media file list.
 *
 * @param array $element
 *   The form element.
 * @return string
 */
function theme_media_file_list($element) {
  // Add the CSS for our display.
  return '<div class="media-file-list">' . theme('form_element', $element, $element['#children']) . '</div>';
}

/**
 * Display a browser pane.
 *
 * @param array $form
 *   The form element.
 * @return string
 */
function theme_media_browser_pane($form) {
  $output   = array();

  // render the drawers
  $output[] = '<div' . drupal_attributes($form['#attributes']) . '>';

  // render the drawer list
  $output[] = '  <div class="browser drawers">';
  $output[] = drupal_render_form(null, $form['drawers']);
  $output[] = ' </div>';

  // render the drawer displays
  $output[] = drupal_render_form(null, $form);
  $output[] = '</div>';

  return implode("\n", $output);
}

/**
 *  Display a radio list of files as thumbnails.
 *
 *  @param $files
 *    An array of file objects to display.
 *  @return
 */
function theme_media_browser_thumbnails($files = array()) {
  $thumbnails = array();
  foreach ($files as $file) {
    $thumbnails[] = theme('media_admin_thumbnail', array('file' => $file));
  }
  return theme('item_list', $thumbnails);
}

/**
 * Theme a thumbnail.
 */
function theme_media_admin_thumbnail($variables) {
  $path = drupal_get_path('module', 'media');
  $file = $variables['file'];
  $file_url = file_create_url($file->uri);
  $output = '';

  // Display a thumbnail for images.
  if (strstr($file->filemime, 'image')) {
    $thumbnail = theme('image_style',
      array(
        'style_name' => 'square_thumbnail',
        'path' => $file->uri,
        'alt' => t('Thumbnail for !filename.', array('!filename' => $file->filename)),
      ));
    
  }
  // Display the 'unknown' icon for other file types.
  else {
    $thumbnail = theme('image',
      array(
        'path' => $path . '/images/file-unknown.png',
        'alt' => t('Thumbnail for !filename.', array('!filename' => $file->filename)),
        'attributes' => array('class' => 'file-unknown'),
      ));
  }
  $output .= l($thumbnail,
    $file_url,
    array(
      'html' => TRUE,
      'attributes' => array('class' => 'media-thumbnail'),
    ));

  return $output;
}


/**
 * Theme operations for a thumbnail.
 */
function theme_media_admin_thumbnail_operations($variables) {
  $destination = drupal_get_destination();
  $file = $variables['file'];
  $output = l(t('edit'), 'admin/content/media/' . $file->fid . '/edit', array('query' => $destination));
  return $output;
}


/* ******************************************** */
/* Content navigation controls                  */
/* ******************************************** */


/**
 * Theme function to display the results limiting- 10, 30, 50 results
 * per page. 
 *
 * @param $options
 *   array parameters for the URL
 * @param $limits
 *   array number of limit settings
 * @return unknown
 */
function theme_media_navigation_control_result_limit($options = array(), $limits = false) {
  if (! $limits) {
    $limits = array(10, 30, 50);
  }
  // @NOTE these do not need to be aware of the current
  //       page because clicking them will reset the 
  //       display to 1 -> $limit
  
  foreach ($limits as $limit) {
    // set the value of this limit parameter to this limit value
    $options['limit'] = $limit;
    $per_display[] = l($limit, $limit, array('query' => $options));
  }
  
  return theme('item_list', array('items'  => $per_display, 'attributes' => array('class' => 'result_limit')));
}


/**
 * Theme function to display pagination of results inside the content navigator
 * per page. 
 *
 * @param $options
 *   array parameters for the URL
 * @return unknown
 */
function theme_media_navigation_control_pager($options = array()) {
  if (! $limits) {
    $limits = array(10, 30, 50);
  }
  // @NOTE these do not need to be aware of the current
  //       page because clicking them will reset the 
  //       display to 1 -> $limit
  
  foreach ($limits as $limit) {
    // set the value of this limit parameter to this limit value
    $options['limit'] = $limit;
    $per_display[] = l($limit, $limit, array('query' => $options));
  }  
  return theme('item_list', array('items'  => $per_display, 'attributes' => array('class' => 'result_limit')));
}
