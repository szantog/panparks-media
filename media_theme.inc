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
  $output = '<div class="media browser">'. $element .'</div>';
  
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
  return '<div class="media-file-list">'. theme('form_element', $element, $element['#children']) .'</div>';
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
  $output[] = '<div'. drupal_attributes($form['#attributes']) .'>';
  
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
 * Theme a thumbnail.
 */
function theme_media_admin_thumbnail($variables) {
  $output = '';
  $file = $variables['file'];
  
  // Display a thumbnail for images.
  if (strstr($file->filemime, 'image')) {
    $thumbnail = theme('image_style',
      array(
        'style_name' => 'thumbnail',
        'path' => $file->uri,
        'alt' => t('Thumbnail for !filename.', array('!filename' => $file->filename)),
      ));
    $output .= l($thumbnail,
      file_create_url($file->uri),
      array(
        'html' => TRUE,
        'attributes' => array('class' => 'media-thumbnail'),
      ));
  }
  // Display an icon for other file types.
  else {
    $output .= $file->filemine;
  }
  
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
