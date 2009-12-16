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
 * @TODO this is depreciated I think
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
 * @TODO this is depreciated I think
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
 * @TODO this is depreciated I think
 *
 * @param array $form
 *   The form element.
 * @return string
 */
function theme_media_browser_pane($form) {
  return;
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
 * Default theming function for creating the browser frame.
 * Assumes an array of file objects as $files and an
 * array of $parameters
 * @param $variables
 *   array of variables
 * @return unknown_type
 */
function theme_media_browser_content_frame($variables) {
  // Pull out all the variables into a usable form
  extract($variables);
  // Did we get any files back?
  if (! count($files)) {
    // @TODO display no files found
  }

  $html = array();

  // On the first invocation, load javascript and build the browser frame
   if ($invoke) {


  }
   // Render the results limiter
  $html[] = theme('media_browser_control_result_limit', array('parameters' => $parameters));
  // Render the actual content
  $html[] = drupal_render(drupal_get_form('media_file_listing_form', $files, $parameters));

  // Make sure to close the wrapping div
  if ($invoke) {
    $html[] = '</div>';
  }
  return implode("\n", $html);
}



/**
 * Display a item list of files as thumbnails. Implements
 * the admin thumbnail theme for now- serves as a wrapper
 *
 *  @param $files
 *    An array of file objects to display.
 *  @return
 */
function theme_media_browser_thumbnails($variables) {
  $files = $variables['files'];
  $thumbnails = array();
  foreach ($files as $file) {
    $thumbnails[] = theme('media_admin_thumbnail', array('file' => $file));
  }
  return theme('item_list', array('items' => $thumbnails, 'attributes' => array('class' => 'media_content_navigator results')));
}


/**
 * Theme a thumbnail.
 * @param $variables
 *   array items being passed in
 */
function theme_media_admin_thumbnail($variables) {
  $path = drupal_get_path('module', 'media');
  $file = $variables['file'];
  $file_url = file_create_url($file->uri);
  $output = '';
  if (module_exists('styles')) {
    $thumbnail = theme('styles',
      array(
        'field_type' => 'file',
        'style_name' => 'thumbnail',
        'uri' => $file->uri,
        'description' => t('Thumbnail for !filename.', array('!filename' => $file->filename)),
        'object' => $variables['file'],
    ));
  }
  else {
    // Display a thumbnail for images.
    if (strstr($file->filemime, 'image')) {
      $thumbnail = theme('image_style',
        array(
          'style_name' => 'thumbnail',
          'path' => $file->uri,
          'alt' => t('Thumbnail for !filename.', array('!filename' => $file->filename)),
        )
      );
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
 * @param $variables
 *   array parameters
 * @return unknown
 */
function theme_media_browser_control_result_limit($variables) {
  // Pull out all the variables into a usable form
  extract($variables);

  if (! isset($limits)) {
    $limits = array(10, 30, 50);
  }
  // @NOTE these do not need to be aware of the current
  //       page because clicking them will reset the
  //       display to 1 -> $limit
  $parameters['page'] = 0;
  // save the active limit
  $current_limit = $parameters['limit'];
  foreach ($limits as $limit) {
    if ($limit == $current_limit) {
      $class = 'active';
    }
    else {
      $class = '';
    }
    // set the value of this limit parameter to this limit value
    $parameters['limit'] = $limit;
    $per_display[] = l($limit, $limit, array('query' => $parameters, 'attributes' => array('class' => $class)));
  }

  return theme('item_list', array('items'  => $per_display, 'attributes' => array('class' => 'result_limit')));
}